// Copyright 2025 NGUYEN PHI HUNG

#include "RLEnvironmentComponent.h"

// Module-wide log categories
#include "UERLLog.h"

URLEnvironmentComponent::URLEnvironmentComponent()
{
	PrimaryComponentTick.bCanEverTick = true; // Enable ticking by default
	
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
	CurrentStep = 0;
	bIsTerminated = false;
	bIsTruncated = false;
	LastReward = 0.0f;

	// Call Blueprint implementation if available, otherwise use default empty observation
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(URLEnvironmentComponent, BP_OnReset)))
	{
		LastObservation = BP_OnReset();
	}
	else
	{
		// Default to zero observation
		LastObservation.Init(0.0f, EnvironmentConfig.ObservationDim);
	}

	// Broadcast reset event
	OnEnvironmentReset.Broadcast(LastObservation);

	return LastObservation;
}

void URLEnvironmentComponent::Step(const TArray<float>& Action)
{
	if (bIsTerminated || bIsTruncated)
	{
		UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::Step called on a finished episode. Please call Reset() first."));
		OnEnvironmentStep.Broadcast(LastObservation, LastReward, bIsTerminated, bIsTruncated);
		return;
	}

	// Call Blueprint implementation if available
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(URLEnvironmentComponent, BP_OnStep)))
	{
		BP_OnStep(Action);
	}

	// Update step count
	CurrentStep++;

	// Get new observation
	LastObservation = GetObservation();

	// Calculate reward
	LastReward = CalculateReward();

	// Check termination conditions
	bIsTerminated = CheckTerminated();
	bool bMaxStepsReached = (EnvironmentConfig.MaxEpisodeLength > 0 && CurrentStep >= EnvironmentConfig.MaxEpisodeLength);
	bIsTruncated = CheckTruncated() || bMaxStepsReached;

	// If terminated, don't set truncated unless max steps reached
	if (bIsTerminated && !bMaxStepsReached)
	{
		bIsTruncated = false;
	}

	// Validate observation dimension
	if (LastObservation.Num() != EnvironmentConfig.ObservationDim)
	{
		UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::Step - Observation dimension mismatch. Expected %d, Got %d. Resizing and padding/truncating."), 
			EnvironmentConfig.ObservationDim, LastObservation.Num());
		LastObservation.SetNumZeroed(EnvironmentConfig.ObservationDim);
	}

	// Broadcast step event
	OnEnvironmentStep.Broadcast(LastObservation, LastReward, bIsTerminated, bIsTruncated);
}

TArray<float> URLEnvironmentComponent::GetObservation()
{
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(URLEnvironmentComponent, BP_GetObservation)))
	{
		return BP_GetObservation();
	}
	return LastObservation;
}

float URLEnvironmentComponent::CalculateReward()
{
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(URLEnvironmentComponent, BP_CalculateReward)))
	{
		return BP_CalculateReward();
	}
	return 0.0f;
}

bool URLEnvironmentComponent::CheckTerminated()
{
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(URLEnvironmentComponent, BP_CheckTerminated)))
	{
		return BP_CheckTerminated();
	}
	return false;
}

bool URLEnvironmentComponent::CheckTruncated()
{
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(URLEnvironmentComponent, BP_CheckTruncated)))
	{
		return BP_CheckTruncated();
	}
	return false;
}

