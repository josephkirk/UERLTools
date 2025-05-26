#include "URLAgentManagerSubsystem.h"
#include "RLEnvironmentComponent.h" // Assuming RLEnvironmentComponent.h exists
#include "RLTypes.h"                // Assuming FRLTrainingConfig is in RLTypes.h
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
    ManagedAgents.GetKeys(AgentNames);
    for (FName AgentName : AgentNames)
    {
        if (ManagedAgents.Contains(AgentName))
        {
            CleanupAgentResources(ManagedAgents[AgentName]);
        }
    }
    ManagedAgents.Empty();

    // Deallocate rl_tools global device context
    if (rlt_context)
    {
        rlt::free(rlt_device, rlt_context);
        rlt_context = nullptr;
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("rl_tools context freed."));
    }

    Super::Deinitialize();
}

bool URLAgentManagerSubsystem::ConfigureAgent(FName AgentName, URLEnvironmentComponent* EnvironmentComponent, const FRLTrainingConfig& TrainingConfig)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("ConfigureAgent for agent '%s' called but not fully implemented."), *AgentName.ToString());
    if (AgentName == NAME_None)
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("ConfigureAgent: AgentName cannot be None."));
        return false;
    }
    if (!IsValid(EnvironmentComponent))
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("ConfigureAgent for agent '%s': Invalid EnvironmentComponent provided."), *AgentName.ToString());
        return false;
    }

    if (ManagedAgents.Contains(AgentName))
    {
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("ConfigureAgent: Agent '%s' already exists. Reconfiguring (not fully supported yet)."), *AgentName.ToString());
        // Potentially clean up old agent before reconfiguring
        RemoveAgent(AgentName);
    }

    FRLAgentContext NewAgentContext;
    NewAgentContext.AgentName = AgentName;
    NewAgentContext.EnvironmentComponent = EnvironmentComponent;
    NewAgentContext.CurrentTrainingConfig = TrainingConfig;
    // --- Initialize rl_tools components ---
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Initializing rl_tools components for agent '%s'..."), *AgentName.ToString());

    // 1. Determine Observation and Action Dimensions from EnvironmentComponent
    // TODO: int32 observation_dim = EnvironmentComponent->GetObservationDimension();
    // TODO: int32 action_dim = EnvironmentComponent->GetActionDimension();
    // TODO: Add validation for these dimensions.

    // 2. Initialize Policy (e.g., Neural Network)
    // TODO: Define ACTUAL_POLICY_TYPE based on TrainingConfig (e.g., rlt::nn_models::mlp::NeuralNetwork<POLICY_SPEC>).
    // TODO: Extract policy-specific hyperparameters from NewAgentContext.CurrentTrainingConfig.
    // NewAgentContext.rlt_policy_buffer = rlt::malloc(rlt_device, rlt_context, sizeof(ACTUAL_POLICY_TYPE));
    // if (!NewAgentContext.rlt_policy_buffer) {
    //     UE_LOG(LOG_UERLTOOLS, Error, TEXT("ConfigureAgent: Failed to allocate memory for policy buffer for agent '%s'."), *AgentName.ToString());
    //     return false;
    // }
    // rlt::init(rlt_device, rlt_context, static_cast<ACTUAL_POLICY_TYPE*>(NewAgentContext.rlt_policy_buffer), observation_dim, action_dim /*, other_policy_params */);
    // TODO: Check policy initialization success if possible.

    // 3. Initialize Optimizer
    // TODO: Define ACTUAL_OPTIMIZER_TYPE based on TrainingConfig (e.g., rlt::nn::optimizers::Adam<OPTIMIZER_SPEC>).
    // TODO: Extract optimizer-specific hyperparameters (e.g., learning rate from NewAgentContext.CurrentTrainingConfig.LearningRate).
    // NewAgentContext.rlt_optimizer_buffer = rlt::malloc(rlt_device, rlt_context, sizeof(ACTUAL_OPTIMIZER_TYPE));
    // if (!NewAgentContext.rlt_optimizer_buffer) {
    //     UE_LOG(LOG_UERLTOOLS, Error, TEXT("ConfigureAgent: Failed to allocate memory for optimizer buffer for agent '%s'."), *AgentName.ToString());
    //     // CRITICAL: Must free any previously allocated buffers (e.g., policy_buffer) before returning false.
    //     // Consider a helper function for partial cleanup on failure.
    //     if(NewAgentContext.rlt_policy_buffer) rlt::free(rlt_device, rlt_context, NewAgentContext.rlt_policy_buffer);
    //     return false;
    // }
    // rlt::init(rlt_device, rlt_context, static_cast<ACTUAL_OPTIMIZER_TYPE*>(NewAgentContext.rlt_optimizer_buffer), NewAgentContext.CurrentTrainingConfig.LearningRate /*, other_optimizer_params */);
    // TODO: Check optimizer initialization success.

    // 4. Initialize Actor-Critic Structure (if applicable, e.g., for PPO, SAC)
    //    This might combine the policy and value function networks and use the optimizer.
    // TODO: Define ACTUAL_ACTOR_CRITIC_TYPE (e.g., rlt::rl::algorithms::ppo::ActorCritic<ACTOR_CRITIC_SPEC>).
    // TODO: Extract AC-specific hyperparameters from NewAgentContext.CurrentTrainingConfig.
    // NewAgentContext.rlt_actor_critic_buffer = rlt::malloc(rlt_device, rlt_context, sizeof(ACTUAL_ACTOR_CRITIC_TYPE));
    // if (!NewAgentContext.rlt_actor_critic_buffer) { /* ... handle allocation failure & cleanup ... */ return false; }
    // rlt::init(rlt_device, rlt_context, static_cast<ACTUAL_ACTOR_CRITIC_TYPE*>(NewAgentContext.rlt_actor_critic_buffer),
    //           static_cast<ACTUAL_POLICY_TYPE*>(NewAgentContext.rlt_policy_buffer), // or actor network part
    //           /* value_network_buffer (if separate), */
    //           static_cast<ACTUAL_OPTIMIZER_TYPE*>(NewAgentContext.rlt_optimizer_buffer),
    //           /* other_actor_critic_params_from_config */);
    // TODO: Check AC initialization success.

    // 5. Initialize Replay Buffer (for Off-Policy Algorithms like SAC, DDPG, DQN)
    // TODO: Check if algorithm is off-policy based on TrainingConfig.
    // TODO: Define ACTUAL_REPLAY_BUFFER_TYPE (e.g., rlt::rl::components::ReplayBuffer<REPLAY_BUFFER_SPEC>).
    // TODO: Extract replay buffer capacity and other params from NewAgentContext.CurrentTrainingConfig.
    // NewAgentContext.rlt_replay_buffer = rlt::malloc(rlt_device, rlt_context, sizeof(ACTUAL_REPLAY_BUFFER_TYPE));
    // if (!NewAgentContext.rlt_replay_buffer) { /* ... handle allocation failure & cleanup ... */ return false; }
    // rlt::init(rlt_device, rlt_context, static_cast<ACTUAL_REPLAY_BUFFER_TYPE*>(NewAgentContext.rlt_replay_buffer), observation_dim, action_dim /*, capacity, other_rb_params */);
    // TODO: Check replay buffer initialization success.

    ManagedAgents.Add(AgentName, NewAgentContext);
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s' configured."), *AgentName.ToString());
    return true;
}

bool URLAgentManagerSubsystem::RemoveAgent(FName AgentName)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("RemoveAgent for agent '%s' called."), *AgentName.ToString());
    if (ManagedAgents.Contains(AgentName))
    {
        FRLAgentContext& AgentContext = ManagedAgents[AgentName];
        if (AgentContext.bIsTraining)
        {
            StopTraining(AgentName); // Ensure training is stopped
        }
        CleanupAgentResources(AgentContext); // Free rl_tools resources

        ManagedAgents.Remove(AgentName);
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s' removed."), *AgentName.ToString());
        return true;
    }
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("RemoveAgent: Agent '%s' not found."), *AgentName.ToString());
    return false;
}

bool URLAgentManagerSubsystem::LoadPolicy(FName AgentName, const FString& FilePath)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("LoadPolicy for agent '%s' from path '%s' called but not implemented."), *AgentName.ToString(), *FilePath);
    if (!ManagedAgents.Contains(AgentName))
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("LoadPolicy: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    // TODO: Implement policy loading from file using rl_tools serialization
    OnAgentPolicyLoaded.Broadcast(AgentName, FilePath); // Broadcast success/failure based on actual outcome
    return false; // Placeholder
}

bool URLAgentManagerSubsystem::SavePolicy(FName AgentName, const FString& FilePath)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("SavePolicy for agent '%s' to path '%s' called but not implemented."), *AgentName.ToString(), *FilePath);
    if (!ManagedAgents.Contains(AgentName))
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("SavePolicy: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    // TODO: Implement policy saving to file using rl_tools serialization
    OnAgentPolicySaved.Broadcast(AgentName, FilePath); // Broadcast success/failure
    return false; // Placeholder
}

bool URLAgentManagerSubsystem::StartTraining(FName AgentName)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("StartTraining for agent '%s' called but not implemented."), *AgentName.ToString());
    if (!ManagedAgents.Contains(AgentName))
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("StartTraining: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    FRLAgentContext& AgentContext = ManagedAgents[AgentName];
    if (AgentContext.bIsTraining)
    {
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("StartTraining: Agent '%s' is already training."), *AgentName.ToString());
        return false;
    }
    AgentContext.bIsTraining = true;
    // TODO: Implement asynchronous training loop (FAsyncTask or FTSTicker)
    // This loop will call environment step, observe, rl_tools update, etc.
    // It should also call OnAgentTrainingStepCompleted.Broadcast(...)
    return true; // Placeholder
}

bool URLAgentManagerSubsystem::PauseTraining(FName AgentName)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("PauseTraining for agent '%s' called but not implemented."), *AgentName.ToString());
    if (!ManagedAgents.Contains(AgentName))
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("PauseTraining: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    FRLAgentContext& AgentContext = ManagedAgents[AgentName];
    if (!AgentContext.bIsTraining)
    {
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("PauseTraining: Agent '%s' is not currently training."), *AgentName.ToString());
        return false;
    }
    // TODO: Implement logic to pause the async training task
    AgentContext.bIsTraining = false; // Simplistic pause, actual async task needs pausing
    return false; // Placeholder
}

bool URLAgentManagerSubsystem::StopTraining(FName AgentName)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("StopTraining for agent '%s' called but not implemented."), *AgentName.ToString());
    if (!ManagedAgents.Contains(AgentName))
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("StopTraining: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    FRLAgentContext& AgentContext = ManagedAgents[AgentName];
    if (!AgentContext.bIsTraining)
    {
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("StopTraining: Agent '%s' was not training."), *AgentName.ToString());
        // Still proceed to ensure cleanup if state is inconsistent
    }
    AgentContext.bIsTraining = false;
    // TODO: Implement logic to gracefully stop the async training task
    // Ensure any resources held by the training task are released.
    OnAgentTrainingFinished.Broadcast(AgentName, true); // Broadcast success/failure
    return true; // Placeholder
}

TArray<float> URLAgentManagerSubsystem::GetAction(FName AgentName, const TArray<float>& Observation)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("GetAction for agent '%s' called but not fully implemented. Returning empty action."), *AgentName.ToString());
    if (!ManagedAgents.Contains(AgentName))
    {
        UE_LOG(LOG_UERLTOOLS, Error, TEXT("GetAction: Agent '%s' not found."), *AgentName.ToString());
        return TArray<float>();
    }
    // FRLAgentContext& AgentContext = ManagedAgents[AgentName];
    // TODO:
    // 1. Convert TArray<float> Observation to rl_tools::Matrix
    // 2. Perform inference using AgentContext.policy and the rl_tools observation matrix
    // 3. Convert rl_tools action matrix back to TArray<float>
    // 4. Return the action
    return TArray<float>(); // Placeholder
}

void URLAgentManagerSubsystem::CleanupAgentResources(FRLAgentContext& AgentContext)
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Cleaning up resources for agent '%s'..."), *AgentContext.AgentName.ToString());
    // Free rl_tools allocated memory for each component
    // This requires knowing the actual types and how they were initialized.
    // For now, we assume they are simple buffers allocated with rlt::malloc.
    // If rl_tools components have their own rlt::free_buffers functions, those should be called first.

    if (AgentContext.rlt_actor_critic_buffer)
    {
        // Example: if actor_critic has its own free_buffers function:
        // rlt::free_buffers(rlt_device, rlt_context, static_cast<ACTUAL_ACTOR_CRITIC_TYPE*>(AgentContext.rlt_actor_critic_buffer));
        rlt::free(rlt_device, rlt_context, AgentContext.rlt_actor_critic_buffer);
        AgentContext.rlt_actor_critic_buffer = nullptr;
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s': rlt_actor_critic_buffer freed."), *AgentContext.AgentName.ToString());
    }
    if (AgentContext.rlt_replay_buffer)
    {
        rlt::free(rlt_device, rlt_context, AgentContext.rlt_replay_buffer);
        AgentContext.rlt_replay_buffer = nullptr;
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s': rlt_replay_buffer freed."), *AgentContext.AgentName.ToString());
    }
    if (AgentContext.rlt_optimizer_buffer)
    {
        rlt::free(rlt_device, rlt_context, AgentContext.rlt_optimizer_buffer);
        AgentContext.rlt_optimizer_buffer = nullptr;
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s': rlt_optimizer_buffer freed."), *AgentContext.AgentName.ToString());
    }
    if (AgentContext.rlt_policy_buffer)
    {
        rlt::free(rlt_device, rlt_context, AgentContext.rlt_policy_buffer);
        AgentContext.rlt_policy_buffer = nullptr;
        UE_LOG(LOG_UERLTOOLS, Log, TEXT("Agent '%s': rlt_policy_buffer freed."), *AgentContext.AgentName.ToString());
    }
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Finished cleaning up resources for agent '%s'."), *AgentContext.AgentName.ToString());
}

bool URLAgentManagerSubsystem::GetAgentTrainingStatus(FName AgentName, bool& bIsCurrentlyTraining, int32& OutCurrentStep, float& OutLastReward)
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("GetAgentTrainingStatus for agent '%s' called."), *AgentName.ToString());
    bIsCurrentlyTraining = false;
    OutCurrentStep = 0;
    OutLastReward = 0.0f;

    if (!ManagedAgents.Contains(AgentName))
    {
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("GetAgentTrainingStatus: Agent '%s' not found."), *AgentName.ToString());
        return false;
    }
    const FRLAgentContext& AgentContext = ManagedAgents[AgentName];
    bIsCurrentlyTraining = AgentContext.bIsTraining;
    OutCurrentStep = AgentContext.CurrentTrainingStep;
    OutLastReward = AgentContext.LastReward;
    return true;
}

// Implement other UFUNCTIONs as needed...
