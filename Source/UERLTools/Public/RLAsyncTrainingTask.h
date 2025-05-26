// Copyright 2025 NGUYEN PHI HUNG

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Async/AsyncWork.h"
#include "RLAgentManager.h"
#include "RLAsyncTrainingTask.generated.h"

// Forward declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAsyncTrainingProgress, int32, Step, float, AverageReward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncTrainingComplete, bool, bSuccess);

/**
 * Async task for running RL training in background
 */
class FRLAsyncTrainingTask : public FNonAbandonableTask
{
	friend class FAsyncTask<FRLAsyncTrainingTask>;

public:
	FRLAsyncTrainingTask(URLAgentManager* InAgentManager, int32 InMaxSteps, int32 InProgressUpdateInterval = 1000);

	// Required by FNonAbandonableTask
	void DoWork();
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FRLAsyncTrainingTask, STATGROUP_ThreadPoolAsyncTasks); }

	// Check if task should continue
	bool ShouldContinue() const { return !bShouldStop && AgentManager && AgentManager->IsTraining(); }

	// Stop the task
	void Stop() { bShouldStop = true; }

	// Get progress
	int32 GetCurrentStep() const { return CurrentStep; }
	float GetAverageReward() const { return AverageReward; }
	bool IsComplete() const { return bIsComplete; }
	bool WasSuccessful() const { return bWasSuccessful; }

private:
	URLAgentManager* AgentManager;
	int32 MaxSteps;
	int32 ProgressUpdateInterval;
	
	// Task state
	volatile bool bShouldStop;
	volatile bool bIsComplete;
	volatile bool bWasSuccessful;
	volatile int32 CurrentStep;
	volatile float AverageReward;
};

/**
 * Blueprint node for async training
 */
UCLASS()
class UERLTOOLS_API URLAsyncTrainingTask : public UObject
{
	GENERATED_BODY()

public:
	URLAsyncTrainingTask();

	// Blueprint events
	UPROPERTY(BlueprintAssignable, Category = "Async Training")
	FOnAsyncTrainingProgress OnProgress;

	UPROPERTY(BlueprintAssignable, Category = "Async Training")
	FOnAsyncTrainingComplete OnComplete;

	// Start async training
	UFUNCTION(BlueprintCallable, Category = "Async Training", meta = (DisplayName = "Start Async Training"))
	bool StartAsyncTraining(URLAgentManager* AgentManager, int32 MaxSteps = 10000, int32 ProgressUpdateInterval = 1000);

	// Stop async training
	UFUNCTION(BlueprintCallable, Category = "Async Training", meta = (DisplayName = "Stop Async Training"))
	void StopAsyncTraining();

	// Check status
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Async Training", meta = (DisplayName = "Is Training Active"))
	bool IsTrainingActive() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Async Training", meta = (DisplayName = "Get Training Progress"))
	void GetTrainingProgress(int32& CurrentStep, float& AverageReward, bool& bIsComplete) const;

protected:
	// Tick function to check progress and fire events
	UFUNCTION()
	void CheckProgress();

private:
	// Async task
	TSharedPtr<FAsyncTask<FRLAsyncTrainingTask>> AsyncTask;
	
	// Timer for progress updates
	FTimerHandle ProgressTimerHandle;
	
	// Last reported values to avoid duplicate events
	int32 LastReportedStep;
	float LastReportedReward;
	
	// Cleanup
	void CleanupTask();
};
