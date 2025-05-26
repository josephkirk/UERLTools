// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLEnvironmentComponent.h"

URLEnvironmentComponent::URLEnvironmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Base class doesn't tick by default. Derived classes can enable if needed.
	CurrentEpisodeStep = 0;
	bIsTerminatedState = false;
	bIsTruncatedState = false;
	CachedReward = 0.0f;
	// EnvironmentConfig will be default initialized by its own constructor
}

void URLEnvironmentComponent::BeginPlay()
{
	Super::BeginPlay();
	// It's generally better to require an explicit ResetEnvironment call to start an episode,
	// rather than auto-resetting on BeginPlay, to give more control to the user/system.
	// If an auto-reset is desired, the user can call ResetEnvironment() in their BeginPlay override
	// or via an initial setup Blueprint.
}

void URLEnvironmentComponent::ResetEnvironment()
{
	CurrentEpisodeStep = 0;
	bIsTerminatedState = false;
	bIsTruncatedState = false;
	CachedReward = 0.0f; // Reward is typically for (s,a,s'), so initial reward is often 0.

	// Call Blueprint/derived C++ logic to get initial observation
	CachedObservation = BP_HandleReset(); 
    
    // Validate observation dimension
    if (CachedObservation.Num() != EnvironmentConfig.ObservationDim)
    {
        UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::ResetEnvironment - Observation dimension mismatch. Expected %d, Got %d from BP_HandleReset. Resizing and padding/truncating."), EnvironmentConfig.ObservationDim, CachedObservation.Num());
        CachedObservation.SetNumZeroed(EnvironmentConfig.ObservationDim); // Ensures correct size
    }

	OnEnvironmentResetComplete.Broadcast(CachedObservation);
}

void URLEnvironmentComponent::StepAction(const TArray<float>& Action)
{
    if (IsDone())
    {
        UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::StepAction called on a finished episode (Terminated: %s, Truncated: %s). Please ResetEnvironment first."), bIsTerminatedState ? TEXT("True") : TEXT("False"), bIsTruncatedState ? TEXT("True") : TEXT("False"));
        // Broadcast current (terminal) state again or do nothing to prevent further state changes.
        OnEnvironmentStepComplete.Broadcast(CachedObservation, CachedReward, bIsTerminatedState, bIsTruncatedState);
        return;
    }

    // Validate action dimension for continuous actions
    if (EnvironmentConfig.bContinuousActions && Action.Num() != EnvironmentConfig.ActionDim)
    {
        UE_LOG(LogTemp, Error, TEXT("URLEnvironmentComponent::StepAction - Action dimension mismatch for continuous actions. Expected %d, Got %d. Action will be ignored or may cause errors in BP_HandleStep."), EnvironmentConfig.ActionDim, Action.Num());
        // Potentially return or use a default/empty action to prevent BP errors with wrong action size.
        // For now, proceeding, but this is a risky state.
    }
    
    // Call Blueprint/derived C++ logic to apply action and update environment
	BP_HandleStep(Action);

	CurrentEpisodeStep++;

	// Update internal state by calling Blueprint/derived C++ getters
	CachedObservation = BP_CalculateObservation();
	CachedReward = BP_CalculateReward();
	bIsTerminatedState = BP_CheckTerminated();
	
	// Check for truncation due to max steps *before* custom truncation logic
    // to ensure max steps is always honored if MaxEpisodeLength > 0.
	bool bMaxStepsReached = (EnvironmentConfig.MaxEpisodeLength > 0 && CurrentEpisodeStep >= EnvironmentConfig.MaxEpisodeLength);
	bIsTruncatedState = BP_CheckTruncated() || bMaxStepsReached; 

    // Standard practice: if an environment is terminated, it cannot also be truncated by other means 
    // (unless termination and max_steps truncation happen on the exact same step).
    // If it's a true terminal state, truncation flags (other than max_steps) are often cleared.
    if (bIsTerminatedState && !bMaxStepsReached) {
        bIsTruncatedState = false; // True termination overrides other truncation reasons.
    }
    
    // Validate observation dimension after step
    if (CachedObservation.Num() != EnvironmentConfig.ObservationDim)
    {
        UE_LOG(LogTemp, Warning, TEXT("URLEnvironmentComponent::StepAction - Observation dimension mismatch after step. Expected %d, Got %d from BP_CalculateObservation. Resizing and padding/truncating."), EnvironmentConfig.ObservationDim, CachedObservation.Num());
        CachedObservation.SetNumZeroed(EnvironmentConfig.ObservationDim); // Ensures correct size
    }

	OnEnvironmentStepComplete.Broadcast(CachedObservation, CachedReward, bIsTerminatedState, bIsTruncatedState);
}

TArray<float> URLEnvironmentComponent::GetCurrentObservation() const
{
	return CachedObservation;
}

float URLEnvironmentComponent::GetCurrentReward() const
{
	return CachedReward;
}

bool URLEnvironmentComponent::IsDone() const
{
	return bIsTerminatedState || bIsTruncatedState;
}

int32 URLEnvironmentComponent::GetMaxEpisodeSteps() const
{
	return EnvironmentConfig.MaxEpisodeLength;
}

bool URLEnvironmentComponent::HasMaxEpisodeSteps() const
{
	// MaxEpisodeLength > 0 indicates it's a meaningful limit.
	// 0 or negative might mean no limit or rely on other termination.
	return EnvironmentConfig.MaxEpisodeLength > 0;
}


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
