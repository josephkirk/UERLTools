// Copyright 2025 NGUYEN PHI HUNG

#include "RLAgentManager.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include <exception> // Required for std::exception

// Module-wide log categories
#include "UERLLog.h"

URLAgentManager::URLAgentManager()
{
	// Initialize state
	bIsInitialized = false;
	bTrainingPaused = false;
	EpisodeStepCount = 0;
	EpisodeReward = 0.0f;
	EnvironmentComponent = nullptr;
	ActorNetwork = nullptr;
	CriticNetwork = nullptr;
	EnvironmentAdapterInstance = nullptr;
	RltContext = nullptr;

	// Initialize training status
	TrainingStatus.bIsTraining = false;
	TrainingStatus.CurrentStep = 0;
	TrainingStatus.CurrentEpisode = 0;
	TrainingStatus.AverageReward = 0.0f;
	TrainingStatus.LastEpisodeReward = 0.0f;
	TrainingStatus.ReplayBufferSize = 0;
}

bool URLAgentManager::InitializeAgent(URLEnvironmentComponent* InEnvironmentComponent, const FLocalRLTrainingConfig& InTrainingConfig)
{
    if (!InEnvironmentComponent)
    {
        UERL_ERROR( TEXT("URLAgentManager::InitializeAgent() - Environment component is null"));
        return false;
    }

    // Store references early for ValidateEnvironment if it needs them
    EnvironmentComponent = InEnvironmentComponent; 
    TrainingConfig = InTrainingConfig;

    if (!ValidateEnvironment()) // ValidateEnvironment might use EnvironmentComponent
    {
        UERL_ERROR( TEXT("URLAgentManager::InitializeAgent() - Environment validation failed"));
        return false;
    }

    CleanupNetworks(); // Call this before new allocations

    try
    {
        TI ObservationDim = EnvironmentComponent->GetObservationDim();
        TI ActionDim = EnvironmentComponent->GetActionDim();

        UERL_LOG( TEXT("URLAgentManager::InitializeAgent() - Runtime Obs Dim: %d, Action Dim: %d"), ObservationDim, ActionDim);
        UERL_LOG( TEXT("URLAgentManager::InitializeAgent() - Spec    Obs Dim: %d, Action Dim: %d"), UERLAgentEnvironmentSpec::OBSERVATION_DIM, UERLAgentEnvironmentSpec::ACTION_DIM);

        if (ObservationDim != UERLAgentEnvironmentSpec::OBSERVATION_DIM || ActionDim != UERLAgentEnvironmentSpec::ACTION_DIM)
        {
            UERL_ERROR( TEXT("URLAgentManager::InitializeAgent() - Environment dimensions (Obs: %d, Act: %d) do not match UERLAgentEnvironmentSpec dimensions (Obs: %d, Act: %d). Please update UERLAgentEnvironmentSpec in RLAgentManager.h or check environment component configuration."),
                ObservationDim, ActionDim, UERLAgentEnvironmentSpec::OBSERVATION_DIM, UERLAgentEnvironmentSpec::ACTION_DIM);
            return false;
        }

        EnvironmentAdapterInstance = new ENVIRONMENT_ADAPTER_TYPE(device, EnvironmentComponent, TrainingConfig.ObservationNormalizationParams, TrainingConfig.ActionNormalizationParams);
        UERL_LOG( TEXT("URLAgentManager::InitializeAgent() - UEEnvironmentAdapter instantiated successfully."));

        // ... (placeholder for actual network initialization) ...
        // For example, if ActorNetwork and CriticNetwork were to be initialized here:
        // ActorNetwork = new ActorNetworkType();
        // CriticNetwork = new CriticNetworkType();
        // rlt::malloc(device, *ActorNetwork);
        // rlt::malloc(device, *CriticNetwork);
        // rlt::init_weights(device, *ActorNetwork, device.random_float_cpu);
        // rlt::init_weights(device, *CriticNetwork, device.random_float_cpu);

        bIsInitialized = true;
        UERL_LOG( TEXT("URLAgentManager::InitializeAgent() - Agent initialized successfully"));
        return true;
    }
    catch (const std::exception& e)
    {
        UERL_ERROR( TEXT("URLAgentManager::InitializeAgent() - Standard exception during initialization: %s"), ANSI_TO_TCHAR(e.what()));
        CleanupNetworks();
        bIsInitialized = false;
        return false;
    }
    catch (...)
    {
        UERL_ERROR( TEXT("URLAgentManager::InitializeAgent() - Unknown exception during initialization"));
        CleanupNetworks();
        bIsInitialized = false;
        return false;
    }
}

bool URLAgentManager::StartTraining()
{
	if (!bIsInitialized)
	{
		UERL_ERROR( TEXT("URLAgentManager::StartTraining() - Agent not initialized"));
		return false;
	}

	if (TrainingStatus.bIsTraining)
	{
		UERL_WARNING( TEXT("URLAgentManager::StartTraining() - Training already in progress"));
		return true;
	}

	// Reset training state
	TrainingStatus.bIsTraining = true;
	TrainingStatus.CurrentStep = 0;
	TrainingStatus.CurrentEpisode = 0;
	bTrainingPaused = false;

	// Reset environment
	CurrentObservation = EnvironmentComponent->Reset();

	UERL_LOG( TEXT("URLAgentManager::StartTraining() - Training started"));
	return true;
}

void URLAgentManager::PauseTraining()
{
	if (TrainingStatus.bIsTraining)
	{
		bTrainingPaused = true;
		UERL_LOG( TEXT("URLAgentManager::PauseTraining() - Training paused"));
	}
}

void URLAgentManager::ResumeTraining()
{
	if (TrainingStatus.bIsTraining && bTrainingPaused)
	{
		bTrainingPaused = false;
		UERL_LOG( TEXT("URLAgentManager::ResumeTraining() - Training resumed"));
	}
}

void URLAgentManager::StopTraining()
{
	if (TrainingStatus.bIsTraining)
	{
		TrainingStatus.bIsTraining = false;
		bTrainingPaused = false;
		UERL_LOG( TEXT("URLAgentManager::StopTraining() - Training stopped"));
		
		OnTrainingFinished.Broadcast(true);
	}
}

bool URLAgentManager::StepTraining(int32 NumSteps)
{
	if (!bIsInitialized || !TrainingStatus.bIsTraining || bTrainingPaused)
	{
		return false;
	}

	for (int32 i = 0; i < NumSteps; ++i)
	{
		if (!PerformTrainingStep())
		{
			return false;
		}

		// Check if we've reached max training steps
		if (TrainingStatus.CurrentStep >= TrainingConfig.MaxTrainingSteps)
		{
			StopTraining();
			break;
		}
	}

	return true;
}

TArray<float> URLAgentManager::GetAction(const TArray<float>& Observation)
{
	if (!bIsInitialized)
	{
		UERL_ERROR( TEXT("URLAgentManager::GetAction() - Agent not initialized"));
		return TArray<float>();
	}

	// Validate observation
	if (Observation.Num() != EnvironmentComponent->GetObservationDim())
	{
		UERL_ERROR( TEXT("URLAgentManager::GetAction() - Invalid observation dimension"));
		return TArray<float>();
	}

	// For now, return random actions as placeholder
	// In a full implementation, this would use the trained policy
	TArray<float> Action;
	Action.SetNum(EnvironmentComponent->GetActionDim());
	
	for (int32 i = 0; i < Action.Num(); ++i)
	{
		Action[i] = FMath::RandRange(-1.0f, 1.0f);
	}

	return Action;
}

bool URLAgentManager::LoadPolicy(const FString& FilePath)
{
	if (!bIsInitialized)
	{
		UERL_ERROR( TEXT("URLAgentManager::LoadPolicy() - Agent not initialized"));
		OnPolicyLoaded.Broadcast(false);
		return false;
	}

	// Check if file exists
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		UERL_ERROR( TEXT("URLAgentManager::LoadPolicy() - File does not exist: %s"), *FilePath);
		OnPolicyLoaded.Broadcast(false);
		return false;
	}

	// Placeholder for actual policy loading
	UERL_LOG( TEXT("URLAgentManager::LoadPolicy() - Policy loading not yet implemented"));
	OnPolicyLoaded.Broadcast(true);
	return true;
}

bool URLAgentManager::SavePolicy(const FString& FilePath)
{
	if (!bIsInitialized)
	{
		UERL_ERROR( TEXT("URLAgentManager::SavePolicy() - Agent not initialized"));
		OnPolicySaved.Broadcast(false);
		return false;
	}

	// Placeholder for actual policy saving
	UERL_LOG( TEXT("URLAgentManager::SavePolicy() - Policy saving not yet implemented"));
	OnPolicySaved.Broadcast(true);
	return true;
}

void URLAgentManager::UpdateTrainingStatus()
{
	// Update average reward
	if (EpisodeRewards.Num() > 0)
	{
		float Sum = 0.0f;
		for (float Reward : EpisodeRewards)
		{
			Sum += Reward;
		}
		TrainingStatus.AverageReward = Sum / EpisodeRewards.Num();
	}

	// Keep only last 100 episodes for average calculation
	if (EpisodeRewards.Num() > 100)
	{
		EpisodeRewards.RemoveAt(0);
	}
}

void URLAgentManager::LogTrainingProgress()
{
	if (TrainingStatus.CurrentStep % 1000 == 0)
	{
		UERL_LOG( TEXT("Training Step: %d, Episode: %d, Avg Reward: %.2f"), 
			TrainingStatus.CurrentStep, TrainingStatus.CurrentEpisode, TrainingStatus.AverageReward);
	}
}

bool URLAgentManager::ValidateEnvironment() const
{
	if (!EnvironmentComponent)
	{
		return false;
	}

	if (EnvironmentComponent->GetObservationDim() <= 0 || EnvironmentComponent->GetActionDim() <= 0)
	{
		return false;
	}

}
bool URLAgentManager::InitializeAgentLogic(URLEnvironmentComponent* InEnvironmentComponent, const FLocalRLTrainingConfig& InTrainingConfig, rlt::devices::DefaultCPU::CONTEXT_TYPE* InRltContext, FName InAgentName)
{
    if (!InEnvironmentComponent)
    {
        UERL_ERROR(TEXT("URLAgentManager::InitializeAgentLogic() - Environment component is null"));
        return false;
    }

    if (!InRltContext)
    {
        UERL_ERROR(TEXT("URLAgentManager::InitializeAgentLogic() - RL tools context is null"));
        return false;
    }

    // Store references
    EnvironmentComponent = InEnvironmentComponent;
    TrainingConfig = InTrainingConfig;
    RltContext = InRltContext;
    AgentName = InAgentName;

    // Log initialization
    UERL_LOG(TEXT("URLAgentManager::InitializeAgentLogic() - Initializing agent '%s' with RL tools context"), *AgentName.ToString());

    // Validate environment dimensions match our expectations
    if (!ValidateEnvironment())
    {
        UERL_ERROR(TEXT("URLAgentManager::InitializeAgentLogic() - Environment validation failed"));
        return false;
    }

    // Clean up any existing resources
    CleanupNetworks();

    try
    {
        // Create environment adapter
        EnvironmentAdapterInstance = new ENVIRONMENT_ADAPTER_TYPE(device, EnvironmentComponent, 
            TrainingConfig.ObservationNormalizationParams, 
            TrainingConfig.ActionNormalizationParams);

        if (!EnvironmentAdapterInstance)
        {
            UERL_ERROR(TEXT("URLAgentManager::InitializeAgentLogic() - Failed to create environment adapter"));
            return false;
        }

        // Initialize networks and other RL components here
        // This is a placeholder for actual network initialization
        // ActorNetwork = new ActorNetworkType();
        // rlt::malloc(device, *ActorNetwork);
        // rlt::init_weights(device, *ActorNetwork, device.random_float_cpu);

        bIsInitialized = true;
        UERL_LOG(TEXT("URLAgentManager::InitializeAgentLogic() - Agent '%s' initialized successfully"), *AgentName.ToString());
        return true;
    }
    catch (const std::exception& e)
    {
        UERL_ERROR(TEXT("URLAgentManager::InitializeAgentLogic() - Standard exception during initialization: %s"), ANSI_TO_TCHAR(e.what()));
        CleanupNetworks();
        bIsInitialized = false;
        return false;
    }
    catch (...)
    {
        UERL_ERROR(TEXT("URLAgentManager::InitializeAgentLogic() - Unknown exception during initialization"));
        CleanupNetworks();
        bIsInitialized = false;
        return false;
    }
}

void URLAgentManager::ShutdownAgent()
{
    if (!bIsInitialized)
    {
        return; // Already shut down
    }

    UERL_LOG(TEXT("URLAgentManager::ShutdownAgent() - Shutting down agent '%s'"), *AgentName.ToString());

    // Stop any ongoing training
    if (TrainingStatus.bIsTraining)
    {
        StopTraining();
    }

    // Clean up all resources
    CleanupNetworks();

    // Reset state
    bIsInitialized = false;
    bTrainingPaused = false;
    EpisodeStepCount = 0;
    EpisodeReward = 0.0f;
    EnvironmentComponent = nullptr;
    RltContext = nullptr;
    AgentName = NAME_None;
    TrainingStatus = FRLTrainingStatus();
}

void URLAgentManager::~URLAgentManager()
{
    ShutdownAgent();
    UERL_LOG(TEXT("URLAgentManager destroyed."));
}

void URLAgentManager::CleanupNetworks()
{
    UERL_LOG(TEXT("URLAgentManager::CleanupNetworks() - Cleaning up networks and adapter..."));

    // Free network resources if they exist
    if (ActorNetwork)
    {
        try
        {
            rlt::free(device, *ActorNetwork);
        }
        catch (...)
        {
            UERL_WARNING(TEXT("URLAgentManager::CleanupNetworks() - Exception while freeing actor network"));
        }
        delete ActorNetwork;
        ActorNetwork = nullptr;
    }

    if (CriticNetwork)
    {
        try
        {
            rlt::free(device, *CriticNetwork);
        }
        catch (...)
        {
            UERL_WARNING(TEXT("URLAgentManager::CleanupNetworks() - Exception while freeing critic network"));
        }
        delete CriticNetwork;
        CriticNetwork = nullptr;
    }

    // Free environment adapter
    if (EnvironmentAdapterInstance)
    {
        delete EnvironmentAdapterInstance;
        EnvironmentAdapterInstance = nullptr;
    }

    bIsInitialized = false;
}

// Implementation of PerformTrainingStep
bool URLAgentManager::PerformTrainingStep()
{
    if (!bIsInitialized || !EnvironmentComponent)
    {
        UERL_ERROR(TEXT("URLAgentManager::PerformTrainingStep() - Agent not properly initialized"));
        return false;
    }

    try
    {
        // Get action from current policy
        TArray<float> Action = GetAction(CurrentObservation);

        // Step environment
        EnvironmentComponent->Step(Action);

        // Get next observation and reward
        TArray<float> NextObservation = EnvironmentComponent->GetObservation();
        float Reward = EnvironmentComponent->CalculateReward();

        // Store experience in replay buffer
        // TODO: Implement experience storage when replay buffer is set up

        // Update training status
        TrainingStatus.CurrentStep++;
        EpisodeStepCount++;
        EpisodeReward += Reward;

        // Check if episode is done
        bool bIsDone = EnvironmentComponent->IsDone();
        if (bIsDone || EpisodeStepCount >= TrainingConfig.MaxEpisodeSteps)
        {
            // Log episode completion
            TrainingStatus.LastEpisodeReward = EpisodeReward;
            TrainingStatus.CurrentEpisode++;
            
            // Reset episode-specific counters
            EpisodeStepCount = 0;
            EpisodeReward = 0.0f;

            // Reset environment for next episode
            CurrentObservation = EnvironmentComponent->Reset();
        }
        else
        {
            // Update current observation for next step
            CurrentObservation = NextObservation;
        }

        // Update training status
        UpdateTrainingStatus();
        LogTrainingProgress();

        return true;
    }
    catch (const std::exception& e)
    {
        UERL_ERROR(TEXT("URLAgentManager::PerformTrainingStep() - Exception: %s"), ANSI_TO_TCHAR(e.what()));
        return false;
    }
    catch (...)
    {
        UERL_ERROR(TEXT("URLAgentManager::PerformTrainingStep() - Unknown exception"));
        return false;
    }
}

void URLAgentManager::CollectExperience()
{
    // This method is a placeholder for collecting experience in the replay buffer
    // Implementation will be added when the replay buffer is properly set up
    
    // For now, we'll just log that this method was called
    UERL_VERBOSE(TEXT("URLAgentManager::CollectExperience() - Collecting experience"));
}

void URLAgentManager::UpdateNetworks()
{
    // This method is a placeholder for updating the neural networks
    // Implementation will be added when the training loop is properly set up
    
    // For now, we'll just log that this method was called
    UERL_VERBOSE(TEXT("URLAgentManager::UpdateNetworks() - Updating networks"));
}
