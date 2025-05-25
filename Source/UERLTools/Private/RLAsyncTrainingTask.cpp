// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLAsyncTrainingTask.h"
#include "Engine/World.h"
#include "TimerManager.h"

// FRLAsyncTrainingTask Implementation

FRLAsyncTrainingTask::FRLAsyncTrainingTask(URLAgentManager* InAgentManager, int32 InMaxSteps, int32 InProgressUpdateInterval)
	: AgentManager(InAgentManager)
	, MaxSteps(InMaxSteps)
	, ProgressUpdateInterval(InProgressUpdateInterval)
	, bShouldStop(false)
	, bIsComplete(false)
	, bWasSuccessful(false)
	, CurrentStep(0)
	, AverageReward(0.0f)
{
}

void FRLAsyncTrainingTask::DoWork()
{
	if (!AgentManager)
	{
		bIsComplete = true;
		bWasSuccessful = false;
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("FRLAsyncTrainingTask::DoWork - Starting async training"));

	try
	{
		while (ShouldContinue() && CurrentStep < MaxSteps)
		{
			// Perform training step
			bool bStepSuccess = AgentManager->StepTraining(1);
			if (!bStepSuccess)
			{
				UE_LOG(LogTemp, Error, TEXT("FRLAsyncTrainingTask::DoWork - Training step failed"));
				break;
			}

			// Update progress
			FRLTrainingStatus Status = AgentManager->GetTrainingStatus();
			CurrentStep = Status.CurrentStep;
			AverageReward = Status.AverageReward;

			// Small delay to prevent overwhelming the system
			FPlatformProcess::Sleep(0.001f); // 1ms
		}

		bWasSuccessful = !bShouldStop && (CurrentStep >= MaxSteps || !AgentManager->IsTraining());
	}
	catch (...)
	{
		UE_LOG(LogTemp, Error, TEXT("FRLAsyncTrainingTask::DoWork - Exception during training"));
		bWasSuccessful = false;
	}

	bIsComplete = true;
	UE_LOG(LogTemp, Log, TEXT("FRLAsyncTrainingTask::DoWork - Training completed. Success: %s, Steps: %d"), 
		bWasSuccessful ? TEXT("True") : TEXT("False"), CurrentStep);
}

// URLAsyncTrainingTask Implementation

URLAsyncTrainingTask::URLAsyncTrainingTask()
	: LastReportedStep(0)
	, LastReportedReward(0.0f)
{
}

bool URLAsyncTrainingTask::StartAsyncTraining(URLAgentManager* AgentManager, int32 MaxSteps, int32 ProgressUpdateInterval)
{
	if (!AgentManager)
	{
		UE_LOG(LogTemp, Error, TEXT("URLAsyncTrainingTask::StartAsyncTraining - AgentManager is null"));
		return false;
	}

	if (!AgentManager->IsInitialized())
	{
		UE_LOG(LogTemp, Error, TEXT("URLAsyncTrainingTask::StartAsyncTraining - AgentManager is not initialized"));
		return false;
	}

	// Stop any existing task
	StopAsyncTraining();

	// Start training on the agent manager
	if (!AgentManager->StartTraining())
	{
		UE_LOG(LogTemp, Error, TEXT("URLAsyncTrainingTask::StartAsyncTraining - Failed to start training on AgentManager"));
		return false;
	}

	// Create and start async task
	AsyncTask = MakeShared<FAsyncTask<FRLAsyncTrainingTask>>(AgentManager, MaxSteps, ProgressUpdateInterval);
	AsyncTask->StartBackgroundTask();

	// Start progress timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ProgressTimerHandle,
			this,
			&URLAsyncTrainingTask::CheckProgress,
			0.1f, // Check every 100ms
			true
		);
	}

	UE_LOG(LogTemp, Log, TEXT("URLAsyncTrainingTask::StartAsyncTraining - Async training started"));
	return true;
}

void URLAsyncTrainingTask::StopAsyncTraining()
{
	// Stop the async task
	if (AsyncTask.IsValid())
	{
		AsyncTask->GetTask().Stop();
		AsyncTask->EnsureCompletion();
		AsyncTask.Reset();
	}

	// Clear progress timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProgressTimerHandle);
	}

	CleanupTask();
	UE_LOG(LogTemp, Log, TEXT("URLAsyncTrainingTask::StopAsyncTraining - Async training stopped"));
}

bool URLAsyncTrainingTask::IsTrainingActive() const
{
	return AsyncTask.IsValid() && !AsyncTask->GetTask().IsComplete();
}

void URLAsyncTrainingTask::GetTrainingProgress(int32& CurrentStep, float& AverageReward, bool& bIsComplete) const
{
	if (AsyncTask.IsValid())
	{
		CurrentStep = AsyncTask->GetTask().GetCurrentStep();
		AverageReward = AsyncTask->GetTask().GetAverageReward();
		bIsComplete = AsyncTask->GetTask().IsComplete();
	}
	else
	{
		CurrentStep = 0;
		AverageReward = 0.0f;
		bIsComplete = true;
	}
}

void URLAsyncTrainingTask::CheckProgress()
{
	if (!AsyncTask.IsValid())
	{
		return;
	}

	const FRLAsyncTrainingTask& Task = AsyncTask->GetTask();
	
	// Check if task is complete
	if (Task.IsComplete())
	{
		// Fire completion event
		OnComplete.Broadcast(Task.WasSuccessful());
		
		// Cleanup
		CleanupTask();
		return;
	}

	// Check for progress updates
	int32 CurrentStep = Task.GetCurrentStep();
	float AverageReward = Task.GetAverageReward();

	// Fire progress event if values changed significantly
	if (CurrentStep != LastReportedStep || FMath::Abs(AverageReward - LastReportedReward) > 0.001f)
	{
		OnProgress.Broadcast(CurrentStep, AverageReward);
		LastReportedStep = CurrentStep;
		LastReportedReward = AverageReward;
	}
}

void URLAsyncTrainingTask::CleanupTask()
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProgressTimerHandle);
	}

	// Reset state
	LastReportedStep = 0;
	LastReportedReward = 0.0f;
}
