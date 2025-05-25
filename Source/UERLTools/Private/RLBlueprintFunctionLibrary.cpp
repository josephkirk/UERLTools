// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLBlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"

URLEnvironmentComponent* URLBlueprintFunctionLibrary::CreateRLEnvironmentComponent(AActor* Owner, const FRLEnvironmentConfig& Config)
{
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("URLBlueprintFunctionLibrary::CreateRLEnvironmentComponent - Owner is null"));
		return nullptr;
	}

	URLEnvironmentComponent* Component = NewObject<URLEnvironmentComponent>(Owner);
	if (Component)
	{
		Component->EnvironmentConfig = Config;
		Owner->AddOwnedComponent(Component);
		Component->RegisterComponent();
		
		UE_LOG(LogTemp, Log, TEXT("URLBlueprintFunctionLibrary::CreateRLEnvironmentComponent - Component created successfully"));
	}

	return Component;
}

URLEnvironmentComponent* URLBlueprintFunctionLibrary::GetRLEnvironmentComponent(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<URLEnvironmentComponent>();
}

URLAgentManager* URLBlueprintFunctionLibrary::CreateRLAgentManager(UObject* Outer)
{
	if (!Outer)
	{
		// Use transient package as default outer
		Outer = GetTransientPackage();
	}

	URLAgentManager* AgentManager = NewObject<URLAgentManager>(Outer);
	if (AgentManager)
	{
		UE_LOG(LogTemp, Log, TEXT("URLBlueprintFunctionLibrary::CreateRLAgentManager - Agent manager created successfully"));
	}

	return AgentManager;
}

TArray<float> URLBlueprintFunctionLibrary::VectorToFloatArray(const FVector& Vector)
{
	TArray<float> Result;
	Result.Add(Vector.X);
	Result.Add(Vector.Y);
	Result.Add(Vector.Z);
	return Result;
}

FVector URLBlueprintFunctionLibrary::FloatArrayToVector(const TArray<float>& FloatArray)
{
	if (FloatArray.Num() >= 3)
	{
		return FVector(FloatArray[0], FloatArray[1], FloatArray[2]);
	}
	else if (FloatArray.Num() == 2)
	{
		return FVector(FloatArray[0], FloatArray[1], 0.0f);
	}
	else if (FloatArray.Num() == 1)
	{
		return FVector(FloatArray[0], 0.0f, 0.0f);
	}
	
	return FVector::ZeroVector;
}

TArray<float> URLBlueprintFunctionLibrary::RotatorToFloatArray(const FRotator& Rotator)
{
	TArray<float> Result;
	Result.Add(Rotator.Pitch);
	Result.Add(Rotator.Yaw);
	Result.Add(Rotator.Roll);
	return Result;
}

FRotator URLBlueprintFunctionLibrary::FloatArrayToRotator(const TArray<float>& FloatArray)
{
	if (FloatArray.Num() >= 3)
	{
		return FRotator(FloatArray[0], FloatArray[1], FloatArray[2]);
	}
	else if (FloatArray.Num() == 2)
	{
		return FRotator(FloatArray[0], FloatArray[1], 0.0f);
	}
	else if (FloatArray.Num() == 1)
	{
		return FRotator(FloatArray[0], 0.0f, 0.0f);
	}
	
	return FRotator::ZeroRotator;
}

TArray<float> URLBlueprintFunctionLibrary::NormalizeFloatArray(const TArray<float>& Input, float MinValue, float MaxValue)
{
	TArray<float> Result = Input;
	
	if (Input.Num() == 0)
	{
		return Result;
	}

	// Find min and max values in the input
	float InputMin = Input[0];
	float InputMax = Input[0];
	
	for (float Value : Input)
	{
		InputMin = FMath::Min(InputMin, Value);
		InputMax = FMath::Max(InputMax, Value);
	}

	// Avoid division by zero
	float Range = InputMax - InputMin;
	if (FMath::IsNearlyZero(Range))
	{
		// All values are the same, set to middle of target range
		float TargetMiddle = (MinValue + MaxValue) * 0.5f;
		for (float& Value : Result)
		{
			Value = TargetMiddle;
		}
		return Result;
	}

	// Normalize to [MinValue, MaxValue]
	float TargetRange = MaxValue - MinValue;
	for (float& Value : Result)
	{
		Value = MinValue + ((Value - InputMin) / Range) * TargetRange;
	}

	return Result;
}

TArray<float> URLBlueprintFunctionLibrary::DenormalizeFloatArray(const TArray<float>& Input, float MinValue, float MaxValue)
{
	TArray<float> Result = Input;
	
	// Assuming input is normalized to [MinValue, MaxValue], denormalize to [0, 1]
	float Range = MaxValue - MinValue;
	if (FMath::IsNearlyZero(Range))
	{
		// Invalid range, return zeros
		for (float& Value : Result)
		{
			Value = 0.0f;
		}
		return Result;
	}

	for (float& Value : Result)
	{
		Value = (Value - MinValue) / Range;
	}

	return Result;
}

TArray<float> URLBlueprintFunctionLibrary::ClampFloatArray(const TArray<float>& Input, float MinValue, float MaxValue)
{
	TArray<float> Result = Input;
	
	for (float& Value : Result)
	{
		Value = FMath::Clamp(Value, MinValue, MaxValue);
	}

	return Result;
}

bool URLBlueprintFunctionLibrary::ValidateObservation(const TArray<float>& Observation, int32 ExpectedDimension)
{
	if (Observation.Num() != ExpectedDimension)
	{
		UE_LOG(LogTemp, Warning, TEXT("URLBlueprintFunctionLibrary::ValidateObservation - Dimension mismatch. Expected: %d, Got: %d"), 
			ExpectedDimension, Observation.Num());
		return false;
	}

	// Check for invalid values (NaN, Infinity)
	for (int32 i = 0; i < Observation.Num(); ++i)
	{
		if (!FMath::IsFinite(Observation[i]))
		{
			UE_LOG(LogTemp, Warning, TEXT("URLBlueprintFunctionLibrary::ValidateObservation - Invalid value at index %d: %f"), 
				i, Observation[i]);
			return false;
		}
	}

	return true;
}

bool URLBlueprintFunctionLibrary::ValidateAction(const TArray<float>& Action, int32 ExpectedDimension)
{
	if (Action.Num() != ExpectedDimension)
	{
		UE_LOG(LogTemp, Warning, TEXT("URLBlueprintFunctionLibrary::ValidateAction - Dimension mismatch. Expected: %d, Got: %d"), 
			ExpectedDimension, Action.Num());
		return false;
	}

	// Check for invalid values (NaN, Infinity)
	for (int32 i = 0; i < Action.Num(); ++i)
	{
		if (!FMath::IsFinite(Action[i]))
		{
			UE_LOG(LogTemp, Warning, TEXT("URLBlueprintFunctionLibrary::ValidateAction - Invalid value at index %d: %f"), 
				i, Action[i]);
			return false;
		}
	}

	return true;
}

void URLBlueprintFunctionLibrary::LogFloatArray(const TArray<float>& Array, const FString& ArrayName)
{
	FString ArrayString = ArrayName + TEXT(": [");
	
	for (int32 i = 0; i < Array.Num(); ++i)
	{
		ArrayString += FString::Printf(TEXT("%.3f"), Array[i]);
		if (i < Array.Num() - 1)
		{
			ArrayString += TEXT(", ");
		}
	}
	
	ArrayString += TEXT("]");
	UE_LOG(LogTemp, Log, TEXT("%s"), *ArrayString);
}

void URLBlueprintFunctionLibrary::LogTrainingStatus(const FRLTrainingStatus& Status)
{
	UE_LOG(LogTemp, Log, TEXT("Training Status:"));
	UE_LOG(LogTemp, Log, TEXT("  Is Training: %s"), Status.bIsTraining ? TEXT("True") : TEXT("False"));
	UE_LOG(LogTemp, Log, TEXT("  Current Step: %d"), Status.CurrentStep);
	UE_LOG(LogTemp, Log, TEXT("  Current Episode: %d"), Status.CurrentEpisode);
	UE_LOG(LogTemp, Log, TEXT("  Average Reward: %.3f"), Status.AverageReward);
	UE_LOG(LogTemp, Log, TEXT("  Last Episode Reward: %.3f"), Status.LastEpisodeReward);
	UE_LOG(LogTemp, Log, TEXT("  Replay Buffer Size: %d"), Status.ReplayBufferSize);
}
