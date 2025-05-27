#include "URLAgentManagerSubsystem.h"
#include "RLEnvironmentComponent.h"
#include "RLAgentManager.h"
#include "RLTypes.h"
#include "Logging/LogMacros.h"

// Fallback log category
#ifndef LOG_UERLTOOLS
#define LOG_UERLTOOLS LogTemp
#else
#define LOG_UERLTOOLS LogUERLTools
#endif

void URLAgentManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("URLAgentManagerSubsystem Initializing..."));

    // Initialize rl_tools global device context
    rlt_context = (rlt::devices::DefaultCPU::CONTEXT_TYPE*)rlt::malloc(rlt_device, sizeof(rlt::devices::DefaultCPU::CONTEXT_TYPE));
    if (!rlt_context) {
        UE_LOG(LOG_UERLTOOLS, Fatal, TEXT("Failed to allocate rl_tools context!"));
        return; // Early exit if context allocation fails
    }
    rlt::init(rlt_device, rlt_context);
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("URLAgentManagerSubsystem Initialized with rl_tools context."));
}

void URLAgentManagerSubsystem::Deinitialize()
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("URLAgentManagerSubsystem Deinitializing..."));

    // Ensure all agents and their rl_tools resources are cleaned up
    TArray<FName> AgentNames;
    ActiveAgents.GetKeys(AgentNames);
    for (FName AgentName : AgentNames)
    {
        URLAgentManager* Agent = ActiveAgents.FindRef(AgentName);
        if (Agent)
        {
            Agent->ShutdownAgent();
        }
    }
    ActiveAgents.Empty();

    // Deallocate rl_tools global device context
    if (rlt_context)
    {
        rlt::free(rlt_device, rlt_context);
        rlt_context = nullptr;
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("rl_tools context freed."));
    }

    Super::Deinitialize();
}

bool URLAgentManagerSubsystem::CreateAgent(FName AgentName, URLEnvironmentComponent* EnvironmentComponent, const FRLTrainingConfig& TrainingConfig)
{
    if (AgentName == NAME_None)
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("CreateAgent: AgentName cannot be None."));
        return false;
    }
    if (!IsValid(EnvironmentComponent))
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("CreateAgent for agent '%s': Invalid EnvironmentComponent provided."), *AgentName.ToString());
        return false;
    }
    if (ActiveAgents.Contains(AgentName))
    {
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("CreateAgent: Agent '%s' already exists. Remove it first or use a different name."), *AgentName.ToString());
        return false;
    }

    URLAgentManager* NewAgent = NewObject<URLAgentManager>(this); // 'this' is the subsystem, acting as outer
    if (!NewAgent)
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("CreateAgent: Failed to create URLAgentManager instance for agent '%s'."), *AgentName.ToString());
        return false;
    }

    // The subsystem's rlt_context is passed to the agent. FLocalRLTrainingConfig is used temporarily.
    // TODO: Update FLocalRLTrainingConfig to FRLTrainingConfig once tech debt #2 is addressed.
    if (NewAgent->InitializeAgentLogic(EnvironmentComponent, static_cast<FLocalRLTrainingConfig>(TrainingConfig), rlt_context, AgentName))
    {
        ActiveAgents.Add(AgentName, NewAgent);
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s' created and initialized successfully."), *AgentName.ToString());
        return true;
    }
    else
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("CreateAgent: Failed to initialize agent logic for '%s'."), *AgentName.ToString());
        // NewAgent will be garbage collected if not added to ActiveAgents and no other strong refs
        return false;
    }
}

// OLD ConfigureAgent function (now deprecated)
bool URLAgentManagerSubsystem::ConfigureAgent(FName AgentName, URLEnvironmentComponent* EnvironmentComponent, const FRLTrainingConfig& TrainingConfig)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("ConfigureAgent is deprecated for agent '%s'. Use CreateAgent instead."), *AgentName.ToString());
    // REMOVE ALL PREVIOUS RL_TOOLS INITIALIZATION LOGIC FROM HERE
    // (the rlt::malloc and rlt::init calls for policy, optimizer, actor_critic, replay_buffer)
    // That logic now belongs in URLAgentManager::InitializeAgentLogic
    return CreateAgent(AgentName, EnvironmentComponent, TrainingConfig);
}
}

bool URLAgentManagerSubsystem::RemoveAgent(FName AgentName)
{
    URLAgentManager* AgentToRemove = ActiveAgents.FindRef(AgentName);
    if (AgentToRemove)
    {
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("Removing agent '%s'..."), *AgentName.ToString());
        if (AgentToRemove->IsTraining()) 
        {
            AgentToRemove->StopTraining(); 
        }
        AgentToRemove->ShutdownAgent();

        ActiveAgents.Remove(AgentName);
        // AgentToRemove (UObject) will be garbage collected
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s' removed."), *AgentName.ToString());
        return true;
    }
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("RemoveAgent: Agent '%s' not found."), *AgentName.ToString());
    return false;
}

bool URLAgentManagerSubsystem::LoadPolicy(FName AgentName, const FString& FilePath)
{
    URLAgentManager* Agent = ActiveAgents.FindRef(AgentName);
    if (Agent)
    {
        bool bSuccess = Agent->LoadPolicy(FilePath);
        OnAgentPolicyLoaded.Broadcast(AgentName, FilePath); // Consider broadcasting based on bSuccess
        return bSuccess;
    }
    UE_LOG(LOG_UERLTOOLS, Error, TEXT("LoadPolicy: Agent '%s' not found."), *AgentName.ToString());
    return false;
}

bool URLAgentManagerSubsystem::SavePolicy(FName AgentName, const FString& FilePath)
{
    URLAgentManager* Agent = ActiveAgents.FindRef(AgentName);
    if (Agent)
    {
        bool bSuccess = Agent->SavePolicy(FilePath);
        OnAgentPolicySaved.Broadcast(AgentName, FilePath);
        return bSuccess;
    }
    UE_LOG(LOG_UERLTOOLS, Error, TEXT("SavePolicy: Agent '%s' not found."), *AgentName.ToString());
    return false;
}

bool URLAgentManagerSubsystem::StartTraining(FName AgentName)
{
    URLAgentManager* Agent = ActiveAgents.FindRef(AgentName);
    if (!Agent)
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("StartTraining: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    
    if (Agent->IsTraining())
    {
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("StartTraining: Agent '%s' is already training."), *AgentName.ToString());
        return false;
    }
    AgentContext.bIsTraining = true;
    // TODO: Implement asynchronous training loop (FAsyncTask or FTSTicker)
    // This loop will call environment step, observe, rl_tools update, etc.
    return true; // Placeholder
}

bool URLAgentManagerSubsystem::PauseTraining(FName AgentName)
{
    URLAgentManager* Agent = ActiveAgents.FindRef(AgentName);
    if (!Agent)
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("PauseTraining: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    
    if (!Agent->IsTraining())
    {
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("PauseTraining: Agent '%s' is not currently training."), *AgentName.ToString());
        return false;
    }
    
    Agent->PauseTraining();
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s' training paused."), *AgentName.ToString());
    return true; // Simplistic pause, actual async task needs pausing
}

bool URLAgentManagerSubsystem::StopTraining(FName AgentName)
{
    URLAgentManager* Agent = ActiveAgents.FindRef(AgentName);
    if (!Agent)
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("StopTraining: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    
    if (!Agent->IsTraining())
    {
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("StopTraining: Agent '%s' was not training."), *AgentName.ToString());
        // Still proceed to ensure cleanup if state is inconsistent
    }
    Agent->StopTraining();
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s' training stopped."), *AgentName.ToString());
    return true; // Placeholder
}

TArray<float> URLAgentManagerSubsystem::GetAction(FName AgentName, const TArray<float>& Observation)
{
    URLAgentManager* Agent = ActiveAgents.FindRef(AgentName);
    if (!Agent)
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("GetAction: Agent '%s' not found."), *AgentName.ToString());
        return TArray<float>();
    }
    
    if (!Agent->IsInitialized())
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("GetAction: Agent '%s' is not initialized."), *AgentName.ToString());
        return TArray<float>();
    }
    
    return Agent->GetAction(Observation);
}

void URLAgentManagerSubsystem::CleanupAgentResources(FRLAgentContext& AgentContext)
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Cleaning up resources for agent '%s'..."), *AgentContext.AgentName.ToString());
    // CleanupAgentResources is no longer needed as URLAgentManager handles its own cleanup
    // Resources are managed by URLAgentManager's ShutdownAgent method
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Finished cleaning up resources for agent '%s'."), *AgentContext.AgentName.ToString());
}

bool URLAgentManagerSubsystem::GetAgentTrainingStatus(FName AgentName, bool& bIsCurrentlyTraining, int32& OutCurrentStep, float& OutLastReward)
{
    bIsCurrentlyTraining = false;
    OutCurrentStep = 0;
    OutLastReward = 0.0f;
    
    URLAgentManager* Agent = ActiveAgents.FindRef(AgentName);
    if (!Agent)
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("GetAgentTrainingStatus: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    
    const FRLTrainingStatus Status = Agent->GetTrainingStatus();
    bIsCurrentlyTraining = Status.bIsTraining;
    OutCurrentStep = Status.CurrentStep;
    OutLastReward = Status.LastEpisodeReward;
    
    return true;
}

// Implement other UFUNCTIONs as needed...
