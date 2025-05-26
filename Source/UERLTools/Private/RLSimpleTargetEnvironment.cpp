// Copyright 2025 NGUYEN PHI HUNG

#include "RLSimpleTargetEnvironment.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

// Module-wide log categories
#include "UERLLog.h"

URLSimpleTargetEnvironment::URLSimpleTargetEnvironment()
{
	// Set default environment configuration for target reaching task
	EnvironmentConfig.ObservationDim = 8; // Agent pos (2), Agent vel (2), Target pos (2), Distance (1), Normalized distance (1)
	EnvironmentConfig.ActionDim = 2; // Movement in X and Y
	EnvironmentConfig.MaxEpisodeLength = 1000;
	EnvironmentConfig.bContinuousActions = true;

	// Initialize state
	AgentPosition = FVector::ZeroVector;
	AgentVelocity = FVector::ZeroVector;
	TargetPosition = FVector(500.0f, 0.0f, 0.0f);
	DistanceToTarget = 0.0f;
	LastDistanceToTarget = 0.0f;
	PreviousAgentPosition = FVector::ZeroVector;
}

void URLSimpleTargetEnvironment::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize positions
	if (bRandomizeTarget)
	{
		RandomizeTargetPosition();
	}
	
	if (bRandomizeStartPosition)
	{
		RandomizeAgentPosition();
	}
	else
	{
		AgentPosition = FVector::ZeroVector;
	}
	
	UpdateAgentState();
}

TArray<float> URLSimpleTargetEnvironment::Reset()
{
	// Call parent reset
	Super::Reset();

	// Randomize positions if enabled
	if (bRandomizeTarget)
	{
		RandomizeTargetPosition();
	}
	
	if (bRandomizeStartPosition)
	{
		RandomizeAgentPosition();
	}
	else
	{
		AgentPosition = FVector::ZeroVector;
	}

	// Reset velocity
	AgentVelocity = FVector::ZeroVector;
	PreviousAgentPosition = AgentPosition;

	// Update state
	UpdateAgentState();

	// Move the actual pawn if it exists
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->SetActorLocation(AgentPosition);
	}

	return GetObservation();
}

void URLSimpleTargetEnvironment::Step(const TArray<float>& Action)
{
	// Validate action
	if (Action.Num() != EnvironmentConfig.ActionDim)
	{
		UERL_ERROR( TEXT("URLSimpleTargetEnvironment::Step - Invalid action dimension"));
		return;
	}

	// Store previous position
	PreviousAgentPosition = AgentPosition;
	LastDistanceToTarget = DistanceToTarget;

	// Apply action (movement)
	FVector MovementAction(Action[0], Action[1], 0.0f);
	MovementAction = MovementAction.GetClampedToMaxSize(1.0f); // Clamp to unit circle
	
	// Calculate new position
	float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f; // Default to ~60 FPS
	FVector Movement = MovementAction * MaxSpeed * DeltaTime;
	AgentPosition += Movement;

	// Clamp agent to arena
	ClampAgentToArena();

	// Update agent state
	UpdateAgentState();

	// Move the actual pawn if it exists
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->SetActorLocation(AgentPosition);
	}

	// Call parent step
	Super::Step(Action);
}

TArray<float> URLSimpleTargetEnvironment::GetObservation()
{
	TArray<float> Observation;
	Observation.SetNum(EnvironmentConfig.ObservationDim);

	// Agent position (normalized to [-1, 1])
	Observation[0] = AgentPosition.X / ArenaSize;
	Observation[1] = AgentPosition.Y / ArenaSize;

	// Agent velocity (normalized)
	Observation[2] = FMath::Clamp(AgentVelocity.X / MaxSpeed, -1.0f, 1.0f);
	Observation[3] = FMath::Clamp(AgentVelocity.Y / MaxSpeed, -1.0f, 1.0f);

	// Target position (normalized to [-1, 1])
	Observation[4] = TargetPosition.X / ArenaSize;
	Observation[5] = TargetPosition.Y / ArenaSize;

	// Distance to target (raw)
	Observation[6] = DistanceToTarget;

	// Normalized distance to target [0, 1]
	float MaxPossibleDistance = ArenaSize * FMath::Sqrt(2.0f); // Diagonal of arena
	Observation[7] = FMath::Clamp(DistanceToTarget / MaxPossibleDistance, 0.0f, 1.0f);

	return Observation;
}

float URLSimpleTargetEnvironment::CalculateReward()
{
	float Reward = 0.0f;

	// Distance-based reward (closer is better)
	float DistanceReward = (LastDistanceToTarget - DistanceToTarget) * RewardScale;
	Reward += DistanceReward;

	// Bonus for reaching target
	if (IsAgentAtTarget())
	{
		Reward += 100.0f * RewardScale;
	}

	// Small penalty for each step to encourage efficiency
	Reward -= 0.1f * RewardScale;

	return Reward;
}

bool URLSimpleTargetEnvironment::CheckTerminated()
{
	// Episode terminates when agent reaches target
	return IsAgentAtTarget();
}

bool URLSimpleTargetEnvironment::CheckTruncated()
{
	// Use parent implementation (max episode length)
	return Super::CheckTruncated();
}

void URLSimpleTargetEnvironment::SetTargetPosition(const FVector& NewTargetPosition)
{
	TargetPosition = NewTargetPosition;
	
	// Clamp to arena
	TargetPosition.X = FMath::Clamp(TargetPosition.X, -ArenaSize, ArenaSize);
	TargetPosition.Y = FMath::Clamp(TargetPosition.Y, -ArenaSize, ArenaSize);
	TargetPosition.Z = 0.0f;

	UpdateAgentState();
}

void URLSimpleTargetEnvironment::RandomizeTargetPosition()
{
	TargetPosition = GetRandomPositionInArena();
	UpdateAgentState();
}

void URLSimpleTargetEnvironment::RandomizeAgentPosition()
{
	AgentPosition = GetRandomPositionInArena();
	
	// Ensure agent doesn't start too close to target
	while (FVector::Dist(AgentPosition, TargetPosition) < TargetRadius * 2.0f)
	{
		AgentPosition = GetRandomPositionInArena();
	}
	
	UpdateAgentState();
}

bool URLSimpleTargetEnvironment::IsAgentAtTarget() const
{
	return DistanceToTarget <= TargetRadius;
}

FVector URLSimpleTargetEnvironment::GetRandomPositionInArena() const
{
	float X = FMath::RandRange(-ArenaSize, ArenaSize);
	float Y = FMath::RandRange(-ArenaSize, ArenaSize);
	return FVector(X, Y, 0.0f);
}

void URLSimpleTargetEnvironment::UpdateAgentState()
{
	// Calculate velocity
	float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
	if (DeltaTime > 0.0f)
	{
		AgentVelocity = (AgentPosition - PreviousAgentPosition) / DeltaTime;
	}

	// Calculate distance to target
	DistanceToTarget = FVector::Dist(AgentPosition, TargetPosition);
}

void URLSimpleTargetEnvironment::ClampAgentToArena()
{
	AgentPosition.X = FMath::Clamp(AgentPosition.X, -ArenaSize, ArenaSize);
	AgentPosition.Y = FMath::Clamp(AgentPosition.Y, -ArenaSize, ArenaSize);
	AgentPosition.Z = 0.0f; // Keep on ground plane
}
