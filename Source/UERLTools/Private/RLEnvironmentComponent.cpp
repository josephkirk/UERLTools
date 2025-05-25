// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLEnvironmentComponent.h"

URLEnvironmentComponent::URLEnvironmentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize state
	CurrentStep = 0;
	bIsTerminated = false;
	bIsTruncated = false;
	LastReward = 0.0f;
}

void URLEnvironmentComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize the environment
	Reset();
}

void URLEnvironmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

TArray<float> URLEnvironmentComponent::Reset()
{
	// Reset environment state
	CurrentStep = 0;
	bIsTerminated = false;
	bIsTruncated = false;
	LastReward = 0.0f;

	// Get initial observation
	TArray<float> InitialObservation;
	
	// Try Blueprint implementation first
	if (BP_OnReset.IsBound())
	{
		InitialObservation = BP_OnReset();
	}
	else
	{
		// Default implementation - return zeros
		InitialObservation.Init(0.0f, EnvironmentConfig.ObservationDim);
		UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::Reset() - Using default implementation. Override BP_OnReset for custom behavior."));
	}

	// Validate observation dimension
	if (InitialObservation.Num() != EnvironmentConfig.ObservationDim)
	{
		UE_LOG(LogTemp, Error, TEXT("URLEnvironmentComponent::Reset() - Observation dimension mismatch. Expected: %d, Got: %d"), 
			EnvironmentConfig.ObservationDim, InitialObservation.Num());
		InitialObservation.SetNum(EnvironmentConfig.ObservationDim);
	}

	LastObservation = InitialObservation;

	// Broadcast reset event
	OnEnvironmentReset.Broadcast(InitialObservation);

	return InitialObservation;
}

void URLEnvironmentComponent::Step(const TArray<float>& Action)
{
	// Validate action dimension
	if (Action.Num() != EnvironmentConfig.ActionDim)
	{
		UE_LOG(LogTemp, Error, TEXT("URLEnvironmentComponent::Step() - Action dimension mismatch. Expected: %d, Got: %d"), 
			EnvironmentConfig.ActionDim, Action.Num());
		return;
	}

	// Don't step if episode is already finished
	if (IsEpisodeFinished())
	{
		UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::Step() - Trying to step finished episode. Call Reset() first."));
		return;
	}

	// Increment step counter
	CurrentStep++;

	// Execute action in environment
	if (BP_OnStep.IsBound())
	{
		BP_OnStep(Action);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::Step() - Using default implementation. Override BP_OnStep for custom behavior."));
	}

	// Get next observation
	TArray<float> NextObservation = GetObservation();

	// Calculate reward
	float Reward = CalculateReward();
	LastReward = Reward;

	// Check termination conditions
	bIsTerminated = CheckTerminated();
	bIsTruncated = CheckTruncated();

	// Cache observation
	LastObservation = NextObservation;

	// Broadcast step event
	OnEnvironmentStep.Broadcast(NextObservation, Reward, bIsTerminated, bIsTruncated);
}

TArray<float> URLEnvironmentComponent::GetObservation()
{
	TArray<float> Observation;

	// Try Blueprint implementation first
	if (BP_GetObservation.IsBound())
	{
		Observation = BP_GetObservation();
	}
	else
	{
		// Default implementation - return cached observation or zeros
		if (LastObservation.Num() == EnvironmentConfig.ObservationDim)
		{
			Observation = LastObservation;
		}
		else
		{
			Observation.Init(0.0f, EnvironmentConfig.ObservationDim);
		}
		UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::GetObservation() - Using default implementation. Override BP_GetObservation for custom behavior."));
	}

	// Validate observation dimension
	if (Observation.Num() != EnvironmentConfig.ObservationDim)
	{
		UE_LOG(LogTemp, Error, TEXT("URLEnvironmentComponent::GetObservation() - Observation dimension mismatch. Expected: %d, Got: %d"), 
			EnvironmentConfig.ObservationDim, Observation.Num());
		Observation.SetNum(EnvironmentConfig.ObservationDim);
	}

	return Observation;
}

float URLEnvironmentComponent::CalculateReward()
{
	float Reward = 0.0f;

	// Try Blueprint implementation first
	if (BP_CalculateReward.IsBound())
	{
		Reward = BP_CalculateReward();
	}
	else
	{
		// Default implementation - return 0
		UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::CalculateReward() - Using default implementation. Override BP_CalculateReward for custom behavior."));
	}

	return Reward;
}

bool URLEnvironmentComponent::CheckTerminated()
{
	bool bTerminated = false;

	// Try Blueprint implementation first
	if (BP_CheckTerminated.IsBound())
	{
		bTerminated = BP_CheckTerminated();
	}
	else
	{
		// Default implementation - never terminate
		UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::CheckTerminated() - Using default implementation. Override BP_CheckTerminated for custom behavior."));
	}

	return bTerminated;
}

bool URLEnvironmentComponent::CheckTruncated()
{
	bool bTruncated = false;

	// Try Blueprint implementation first
	if (BP_CheckTruncated.IsBound())
	{
		bTruncated = BP_CheckTruncated();
	}
	else
	{
		// Default implementation - check max episode length
		bTruncated = (CurrentStep >= EnvironmentConfig.MaxEpisodeLength);
	}

	return bTruncated;
}
