// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"

THIRD_PARTY_INCLUDES_START
#include "rl_tools/operations/cpu_mux.h"
#include "rl_tools/devices/cpu.h"
THIRD_PARTY_INCLUDES_END

#include "RLEnvironmentComponent.generated.h"

// Forward declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnvironmentReset, const TArray<float>&, InitialObservation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnEnvironmentStep, const TArray<float>&, NextObservation, float, Reward, bool, bTerminated, bool, bTruncated);

/**
 * Configuration structure for RL environment
 */
USTRUCT(BlueprintType)
struct UERLTOOLS_API FRLEnvironmentConfig
{
	GENERATED_BODY()

	// Observation space dimension
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Config")
	int32 ObservationDim = 4;

	// Action space dimension
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Config")
	int32 ActionDim = 2;

	// Maximum episode length
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Config")
	int32 MaxEpisodeLength = 1000;

	// Whether actions are continuous or discrete
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Config")
	bool bContinuousActions = true;

	FRLEnvironmentConfig()
	{
		ObservationDim = 4;
		ActionDim = 2;
		MaxEpisodeLength = 1000;
		bContinuousActions = true;
	}
};

/**
 * Base environment component that acts as a bridge between UE and rl_tools
 * This component should be inherited and customized for specific RL tasks
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class UERLTOOLS_API URLEnvironmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URLEnvironmentComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Environment configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FRLEnvironmentConfig EnvironmentConfig;

	// Current episode step count
	UPROPERTY(BlueprintReadOnly, Category = "Environment State")
	int32 CurrentStep;

	// Whether the environment is currently terminated
	UPROPERTY(BlueprintReadOnly, Category = "Environment State")
	bool bIsTerminated;

	// Whether the environment is currently truncated (max steps reached)
	UPROPERTY(BlueprintReadOnly, Category = "Environment State")
	bool bIsTruncated;

	// Blueprint Events
	UPROPERTY(BlueprintAssignable, Category = "Environment Events")
	FOnEnvironmentReset OnEnvironmentReset;

	UPROPERTY(BlueprintAssignable, Category = "Environment Events")
	FOnEnvironmentStep OnEnvironmentStep;

	// Core environment functions
	UFUNCTION(BlueprintCallable, Category = "Environment")
	virtual TArray<float> Reset();

	UFUNCTION(BlueprintCallable, Category = "Environment")
	virtual void Step(const TArray<float>& Action);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	virtual TArray<float> GetObservation();

	UFUNCTION(BlueprintCallable, Category = "Environment")
	virtual float CalculateReward();

	UFUNCTION(BlueprintCallable, Category = "Environment")
	virtual bool CheckTerminated();

	UFUNCTION(BlueprintCallable, Category = "Environment")
	virtual bool CheckTruncated();

	// Utility functions
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
	int32 GetObservationDim() const { return EnvironmentConfig.ObservationDim; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
	int32 GetActionDim() const { return EnvironmentConfig.ActionDim; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
	bool IsContinuousActions() const { return EnvironmentConfig.bContinuousActions; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
	bool IsEpisodeFinished() const { return bIsTerminated || bIsTruncated; }

protected:
	// Override these functions in Blueprint or derived classes for custom behavior
	UFUNCTION(BlueprintImplementableEvent, Category = "Environment", meta = (DisplayName = "On Reset Implementation"))
	TArray<float> BP_OnReset();

	UFUNCTION(BlueprintImplementableEvent, Category = "Environment", meta = (DisplayName = "On Step Implementation"))
	void BP_OnStep(const TArray<float>& Action);

	UFUNCTION(BlueprintImplementableEvent, Category = "Environment", meta = (DisplayName = "Get Observation Implementation"))
	TArray<float> BP_GetObservation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Environment", meta = (DisplayName = "Calculate Reward Implementation"))
	float BP_CalculateReward();

	UFUNCTION(BlueprintImplementableEvent, Category = "Environment", meta = (DisplayName = "Check Terminated Implementation"))
	bool BP_CheckTerminated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Environment", meta = (DisplayName = "Check Truncated Implementation"))
	bool BP_CheckTruncated();

private:
	// rl_tools device
	rlt::devices::DefaultCPU Device;

	// Last observation cache
	TArray<float> LastObservation;

	// Last reward cache
	float LastReward;
};
