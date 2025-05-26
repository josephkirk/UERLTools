// Copyright 2025 NGUYEN PHI HUNG

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RLTypes.h" // For FRLTrainingConfig, FRLEnvironmentConfig, FRLNormalizationParams
#include "URLToolsBlueprintFunctionLibrary.generated.h"

// Forward declarations for rl_tools types if directly exposed, or use opaque handles.
// For now, we'll assume conversions happen to UE types before BP exposure for matrix data.

UCLASS()
class UERLTOOLS_API UURLToolsBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // --- Data Conversion Utilities (Wrapping RLToolsConversionUtils) ---

    /**
     * Converts a UE TArray<float> observation to an internal representation suitable for rl_tools.
     * This is a placeholder and would typically involve more complex handling or direct use of subsystem.
     */
    UFUNCTION(BlueprintCallable, Category = "RLTools|Data Utils", meta = (Keywords = "convert observation tensor matrix rltools"))
    static void Blueprint_ConvertObservationToRLMatrix(const TArray<float>& UEObservation, FString& OutRLMatrixRepresentation /* Placeholder for actual rl_tools Matrix or handle */);

    /**
     * Converts an internal rl_tools action representation to a UE TArray<float>.
     * This is a placeholder.
     */
    UFUNCTION(BlueprintCallable, Category = "RLTools|Data Utils", meta = (Keywords = "convert action tensor matrix rltools"))
    static TArray<float> Blueprint_ConvertRLActionToUEFormat(const FString& RLActionRepresentation /* Placeholder */);

    // --- Normalization/Denormalization Utilities ---

    /**
     * Normalizes a TArray<float> using the provided normalization parameters.
     */
    UFUNCTION(BlueprintCallable, Category = "RLTools|Data Utils", meta = (Keywords = "normalize observation action data stats mean stddev"))
    static TArray<float> Blueprint_NormalizeData(const TArray<float>& Data, const FRLNormalizationParams& NormParams);

    /**
     * Denormalizes a TArray<float> using the provided normalization parameters.
     */
    UFUNCTION(BlueprintCallable, Category = "RLTools|Data Utils", meta = (Keywords = "denormalize observation action data stats mean stddev"))
    static TArray<float> Blueprint_DenormalizeData(const TArray<float>& NormalizedData, const FRLNormalizationParams& NormParams);

    // --- Debugging and Visualization ---

    /**
     * Returns a string representation of an rl_tools Matrix (placeholder).
     * In a real scenario, this might take an opaque handle to a matrix or be part of a debug draw system.
     */
    UFUNCTION(BlueprintCallable, Category = "RLTools|Debug", meta = (Keywords = "print debug display tensor matrix rltools"))
    static FString Blueprint_GetRLMatrixAsString(const FString& RLMatrixRepresentation /* Placeholder */);

    /**
     * Checks the dimensions of an rl_tools Matrix (placeholder).
     */
    UFUNCTION(BlueprintCallable, Category = "RLTools|Debug", meta = (Keywords = "dimension shape size tensor matrix rltools"))
    static bool Blueprint_CheckRLMatrixDimensions(const FString& RLMatrixRepresentation /* Placeholder */, int32& OutRows, int32& OutCols);

    // --- Configuration Utilities ---

    /**
     * Creates a default FRLTrainingConfig.
     */
    UFUNCTION(BlueprintPure, Category = "RLTools|Config", meta = (Keywords = "make default training config rltools"))
    static FRLTrainingConfig Blueprint_MakeDefaultTrainingConfig();

    /**
     * Validates an FRLTrainingConfig, returning true if valid and logging errors if not.
     */
    UFUNCTION(BlueprintCallable, Category = "RLTools|Config", meta = (Keywords = "validate check training config rltools"))
    static bool Blueprint_ValidateTrainingConfig(const FRLTrainingConfig& Config, FString& OutErrorMessage);

    /**
     * Creates a default FRLEnvironmentConfig.
     */
    UFUNCTION(BlueprintPure, Category = "RLTools|Config", meta = (Keywords = "make default environment config rltools"))
    static FRLEnvironmentConfig Blueprint_MakeDefaultEnvironmentConfig();

    /**
     * Validates an FRLEnvironmentConfig, returning true if valid and logging errors if not.
     */
    UFUNCTION(BlueprintCallable, Category = "RLTools|Config", meta = (Keywords = "validate check environment config rltools"))
    static bool Blueprint_ValidateEnvironmentConfig(const FRLEnvironmentConfig& Config, FString& OutErrorMessage);
};
