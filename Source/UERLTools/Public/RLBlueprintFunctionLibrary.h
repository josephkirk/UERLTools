// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RLEnvironmentComponent.h"
#include "RLAgentManager.h"
#include "RLBlueprintFunctionLibrary.generated.h"

/**
 * Blueprint function library for RLTools utility functions
 */
UCLASS()
class UERLTOOLS_API URLBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Environment utility functions
	UFUNCTION(BlueprintCallable, Category = "RLTools|Environment", meta = (DisplayName = "Create RL Environment Component"))
	static URLEnvironmentComponent* CreateRLEnvironmentComponent(AActor* Owner, const FRLEnvironmentConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "RLTools|Environment", meta = (DisplayName = "Get Environment Component"))
	static URLEnvironmentComponent* GetRLEnvironmentComponent(AActor* Actor);

	// Agent utility functions
	UFUNCTION(BlueprintCallable, Category = "RLTools|Agent", meta = (DisplayName = "Create RL Agent Manager"))
	static URLAgentManager* CreateRLAgentManager(UObject* Outer = nullptr);

	// Data conversion utilities
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RLTools|Utility", meta = (DisplayName = "Vector to Float Array"))
	static TArray<float> VectorToFloatArray(const FVector& Vector);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RLTools|Utility", meta = (DisplayName = "Float Array to Vector"))
	static FVector FloatArrayToVector(const TArray<float>& FloatArray);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RLTools|Utility", meta = (DisplayName = "Rotator to Float Array"))
	static TArray<float> RotatorToFloatArray(const FRotator& Rotator);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RLTools|Utility", meta = (DisplayName = "Float Array to Rotator"))
	static FRotator FloatArrayToRotator(const TArray<float>& FloatArray);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RLTools|Utility", meta = (DisplayName = "Normalize Float Array"))
	static TArray<float> NormalizeFloatArray(const TArray<float>& Input, float MinValue = -1.0f, float MaxValue = 1.0f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RLTools|Utility", meta = (DisplayName = "Denormalize Float Array"))
	static TArray<float> DenormalizeFloatArray(const TArray<float>& Input, float MinValue = -1.0f, float MaxValue = 1.0f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RLTools|Utility", meta = (DisplayName = "Clamp Float Array"))
	static TArray<float> ClampFloatArray(const TArray<float>& Input, float MinValue = -1.0f, float MaxValue = 1.0f);

	// Validation utilities
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RLTools|Validation", meta = (DisplayName = "Validate Observation"))
	static bool ValidateObservation(const TArray<float>& Observation, int32 ExpectedDimension);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RLTools|Validation", meta = (DisplayName = "Validate Action"))
	static bool ValidateAction(const TArray<float>& Action, int32 ExpectedDimension);

	// Logging utilities
	UFUNCTION(BlueprintCallable, Category = "RLTools|Debug", meta = (DisplayName = "Log Float Array"))
	static void LogFloatArray(const TArray<float>& Array, const FString& ArrayName = TEXT("Array"));

	UFUNCTION(BlueprintCallable, Category = "RLTools|Debug", meta = (DisplayName = "Log Training Status"))
	static void LogTrainingStatus(const FRLTrainingStatus& Status);
};
