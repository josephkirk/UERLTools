// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLAgentManager.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include <exception> // Required for std::exception

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

	// Initialize training status
	TrainingStatus.bIsTraining = false;
	TrainingStatus.CurrentStep = 0;
	TrainingStatus.CurrentEpisode = 0;
	TrainingStatus.AverageReward = 0.0f;
	TrainingStatus.LastEpisodeReward = 0.0f;
	TrainingStatus.ReplayBufferSize = 0;
}

bool URLAgentManager::InitializeAgent(URLEnvironmentComponent* InEnvironmentComponent, const FRLTrainingConfig& InTrainingConfig)
{
    if (!InEnvironmentComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("URLAgentManager::InitializeAgent() - Environment component is null"));
        return false;
    }

    // Store references early for ValidateEnvironment if it needs them
    EnvironmentComponent = InEnvironmentComponent; 
    TrainingConfig = InTrainingConfig;

    if (!ValidateEnvironment()) // ValidateEnvironment might use EnvironmentComponent
    {
        UE_LOG(LogTemp, Error, TEXT("URLAgentManager::InitializeAgent() - Environment validation failed"));
        return false;
    }

    CleanupNetworks(); // Call this before new allocations

    try
    {
        TI ObservationDim = EnvironmentComponent->GetObservationDim();
        TI ActionDim = EnvironmentComponent->GetActionDim();

        UE_LOG(LogTemp, Log, TEXT("URLAgentManager::InitializeAgent() - Runtime Obs Dim: %d, Action Dim: %d"), ObservationDim, ActionDim);
        UE_LOG(LogTemp, Log, TEXT("URLAgentManager::InitializeAgent() - Spec    Obs Dim: %d, Action Dim: %d"), UERLAgentEnvironmentSpec::OBSERVATION_DIM, UERLAgentEnvironmentSpec::ACTION_DIM);

        if (ObservationDim != UERLAgentEnvironmentSpec::OBSERVATION_DIM || ActionDim != UERLAgentEnvironmentSpec::ACTION_DIM)
        {
            UE_LOG(LogTemp, Error, TEXT("URLAgentManager::InitializeAgent() - Environment dimensions (Obs: %d, Act: %d) do not match UERLAgentEnvironmentSpec dimensions (Obs: %d, Act: %d). Please update UERLAgentEnvironmentSpec in RLAgentManager.h or check environment component configuration."),
                ObservationDim, ActionDim, UERLAgentEnvironmentSpec::OBSERVATION_DIM, UERLAgentEnvironmentSpec::ACTION_DIM);
            return false;
        }

        EnvironmentAdapterInstance = new ENVIRONMENT_ADAPTER_TYPE(device, EnvironmentComponent, TrainingConfig.ObservationNormalizationParams, TrainingConfig.ActionNormalizationParams);
        UE_LOG(LogTemp, Log, TEXT("URLAgentManager::InitializeAgent() - UEEnvironmentAdapter instantiated successfully."));

        // ... (placeholder for actual network initialization) ...
        // For example, if ActorNetwork and CriticNetwork were to be initialized here:
        // ActorNetwork = new ActorNetworkType();
        // CriticNetwork = new CriticNetworkType();
        // rlt::malloc(device, *ActorNetwork);
        // rlt::malloc(device, *CriticNetwork);
        // rlt::init_weights(device, *ActorNetwork, device.random_float_cpu);
        // rlt::init_weights(device, *CriticNetwork, device.random_float_cpu);

        bIsInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("URLAgentManager::InitializeAgent() - Agent initialized successfully"));
        return true;
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("URLAgentManager::InitializeAgent() - Standard exception during initialization: %s"), ANSI_TO_TCHAR(e.what()));
        CleanupNetworks();
        bIsInitialized = false;
        return false;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("URLAgentManager::InitializeAgent() - Unknown exception during initialization"));
        CleanupNetworks();
        bIsInitialized = false;
        return false;
    }
}

bool URLAgentManager::StartTraining()
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Error, TEXT("URLAgentManager::StartTraining() - Agent not initialized"));
		return false;
	}

	if (TrainingStatus.bIsTraining)
	{
		UE_LOG(LogTemp, Warning, TEXT("URLAgentManager::StartTraining() - Training already in progress"));
		return true;
	}

	// Reset training state
	TrainingStatus.bIsTraining = true;
	TrainingStatus.CurrentStep = 0;
	TrainingStatus.CurrentEpisode = 0;
	bTrainingPaused = false;

	// Reset environment
	CurrentObservation = EnvironmentComponent->Reset();

	UE_LOG(LogTemp, Log, TEXT("URLAgentManager::StartTraining() - Training started"));
	return true;
}

void URLAgentManager::PauseTraining()
{
	if (TrainingStatus.bIsTraining)
	{
		bTrainingPaused = true;
		UE_LOG(LogTemp, Log, TEXT("URLAgentManager::PauseTraining() - Training paused"));
	}
}

void URLAgentManager::ResumeTraining()
{
	if (TrainingStatus.bIsTraining && bTrainingPaused)
	{
		bTrainingPaused = false;
		UE_LOG(LogTemp, Log, TEXT("URLAgentManager::ResumeTraining() - Training resumed"));
	}
}

void URLAgentManager::StopTraining()
{
	if (TrainingStatus.bIsTraining)
	{
		TrainingStatus.bIsTraining = false;
		bTrainingPaused = false;
		UE_LOG(LogTemp, Log, TEXT("URLAgentManager::StopTraining() - Training stopped"));
		
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
		UE_LOG(LogTemp, Error, TEXT("URLAgentManager::GetAction() - Agent not initialized"));
		return TArray<float>();
	}

	// Validate observation
	if (Observation.Num() != EnvironmentComponent->GetObservationDim())
	{
		UE_LOG(LogTemp, Error, TEXT("URLAgentManager::GetAction() - Invalid observation dimension"));
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
		UE_LOG(LogTemp, Error, TEXT("URLAgentManager::LoadPolicy() - Agent not initialized"));
		OnPolicyLoaded.Broadcast(false);
		return false;
	}

	// Check if file exists
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("URLAgentManager::LoadPolicy() - File does not exist: %s"), *FilePath);
		OnPolicyLoaded.Broadcast(false);
		return false;
	}

	// Placeholder for actual policy loading
	UE_LOG(LogTemp, Log, TEXT("URLAgentManager::LoadPolicy() - Policy loading not yet implemented"));
	OnPolicyLoaded.Broadcast(true);
	return true;
}

bool URLAgentManager::SavePolicy(const FString& FilePath)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Error, TEXT("URLAgentManager::SavePolicy() - Agent not initialized"));
		OnPolicySaved.Broadcast(false);
		return false;
	}

	// Placeholder for actual policy saving
	UE_LOG(LogTemp, Log, TEXT("URLAgentManager::SavePolicy() - Policy saving not yet implemented"));
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
		UE_LOG(LogTemp, Log, TEXT("Training Step: %d, Episode: %d, Avg Reward: %.2f"), 
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

void URLAgentManager::CleanupNetworks()
{
    UE_LOG(LogTemp, Log, TEXT("URLAgentManager::CleanupNetworks() - Cleaning up networks and adapter..."));

    if (EnvironmentAdapterInstance)
    {
        delete EnvironmentAdapterInstance;
        EnvironmentAdapterInstance = nullptr;
        UE_LOG(LogTemp, Log, TEXT("URLAgentManager::CleanupNetworks() - UEEnvironmentAdapter cleaned up."));
    }

    // ... (cleanup for ActorNetwork, CriticNetwork, etc.) ...
    // Example:
    // if (ActorNetwork) {
    //     rlt::free(device, *ActorNetwork);
    //     delete ActorNetwork;
    //     ActorNetwork = nullptr;
    // }
    // if (CriticNetwork) {
    //     rlt::free(device, *CriticNetwork);
    //     delete CriticNetwork;
    //     CriticNetwork = nullptr;
    // }
    UE_LOG(LogTemp, Log, TEXT("URLAgentManager::CleanupNetworks() - Placeholder for Actor/Critic network cleanup."));
}

URLAgentManager::~URLAgentManager()
{
    CleanupNetworks();
    UE_LOG(LogTemp, Log, TEXT("URLAgentManager destroyed."));
}

bool URLAgentManager::PerformTrainingStep()
{
    // Get action from current policy
    CurrentAction = GetAction(CurrentObservation);

    // Step environment
    EnvironmentComponent->Step(CurrentAction);

    // ... (existing code)
	// Get next observation and reward
	TArray<float> NextObservation = EnvironmentComponent->GetObservation();
	float Reward = EnvironmentComponent->CalculateReward();

	// Update episode tracking
	EpisodeReward += Reward;
	EpisodeStepCount++;
	TrainingStatus.CurrentStep++;

	// Check if episode is finished
	if (EnvironmentComponent->IsEpisodeFinished())
	{
		// Episode finished
		TrainingStatus.LastEpisodeReward = EpisodeReward;
		EpisodeRewards.Add(EpisodeReward);
		TrainingStatus.CurrentEpisode++;

		// Reset for next episode
		EpisodeReward = 0.0f;
		EpisodeStepCount = 0;
		CurrentObservation = EnvironmentComponent->Reset();
	}
	else
	{
		// Continue episode
		CurrentObservation = NextObservation;
	}

	// Update training status
	UpdateTrainingStatus();
	LogTrainingProgress();

	// Broadcast training step event
	OnTrainingStep.Broadcast(TrainingStatus.CurrentStep, TrainingStatus.AverageReward);

	return true;
}

void URLAgentManager::CollectExperience()
{
	// Placeholder for experience collection
	// This would store transitions in replay buffer
}

void URLAgentManager::UpdateNetworks()
{
	// Placeholder for network updates
	// This would perform the actual RL algorithm updates
}
