// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RLEnvironmentComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RLSimpleTargetEnvironment.generated.h"

/**
 * Simple target-reaching environment for demonstration
 * Agent (pawn) learns to reach a target location
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class UERLTOOLS_API URLSimpleTargetEnvironment : public URLEnvironmentComponent
{
	GENERATED_BODY()

public:
	URLSimpleTargetEnvironment();

protected:
	virtual void BeginPlay() override;

public:
	// Environment parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Environment")
	float ArenaSize = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Environment")
	float TargetRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Environment")
	float MaxSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Environment")
	float RewardScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Environment")
	bool bRandomizeTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Environment")
	bool bRandomizeStartPosition = true;

	// Current state
	UPROPERTY(BlueprintReadOnly, Category = "Target Environment State")
	FVector AgentPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Target Environment State")
	FVector AgentVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Target Environment State")
	FVector TargetPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Target Environment State")
	float DistanceToTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Target Environment State")
	float LastDistanceToTarget;

	// Override base environment functions
	virtual TArray<float> Reset() override;
	virtual void Step(const TArray<float>& Action) override;
	virtual TArray<float> GetObservation() override;
	virtual float CalculateReward() override;
	virtual bool CheckTerminated() override;
	virtual bool CheckTruncated() override;

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "Target Environment")
	void SetTargetPosition(const FVector& NewTargetPosition);

	UFUNCTION(BlueprintCallable, Category = "Target Environment")
	void RandomizeTargetPosition();

	UFUNCTION(BlueprintCallable, Category = "Target Environment")
	void RandomizeAgentPosition();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Target Environment")
	bool IsAgentAtTarget() const;

protected:
	// Helper functions
	FVector GetRandomPositionInArena() const;
	void UpdateAgentState();
	void ClampAgentToArena();

private:
	// Previous position for velocity calculation
	FVector PreviousAgentPosition;
};
