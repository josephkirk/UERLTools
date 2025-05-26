// Copyright Windsurf Engineering, Inc. All Rights Reserved.

#include "URLToolsBlueprintFunctionLibrary.h"
#include "RLToolsConversionUtils.h" // For actual conversion logic
#include "Logging/LogMacros.h"

// Fallback log category if UERLToolsLog is not globally defined
#ifndef LOG_UERLTOOLS
#define LOG_UERLTOOLS LogTemp
#endif

// --- Data Conversion Utilities ---
void UURLToolsBlueprintFunctionLibrary::Blueprint_ConvertObservationToRLMatrix(const TArray<float>& UEObservation, FString& OutRLMatrixRepresentation)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("Blueprint_ConvertObservationToRLMatrix: Placeholder implementation."));
    // Actual implementation would use RLToolsConversionUtils::UEArrayToRLMatrix
    // and then serialize the rl_tools::Matrix to a string or return an opaque handle.
    OutRLMatrixRepresentation = FString::Printf(TEXT("RLMatrix (Obs: %d elements)"), UEObservation.Num());
}

TArray<float> UURLToolsBlueprintFunctionLibrary::Blueprint_ConvertRLActionToUEFormat(const FString& RLActionRepresentation)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("Blueprint_ConvertRLActionToUEFormat: Placeholder implementation with input: %s"), *RLActionRepresentation);
    // Actual implementation would deserialize RLActionRepresentation to an rl_tools::Matrix
    // and then use RLToolsConversionUtils::RLMatrixToUEArray.
    return TArray<float>();
}

// --- Normalization/Denormalization Utilities ---
TArray<float> UURLToolsBlueprintFunctionLibrary::Blueprint_NormalizeData(const TArray<float>& Data, const FRLNormalizationParams& NormParams)
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Blueprint_NormalizeData called."));
    if (!NormParams.bIsEnabled)
    {
        return Data; // Return original data if normalization is disabled
    }

    // This is a simplified version. The actual robust implementation is in RLToolsConversionUtils.
    // This BP-facing function might call a simplified or direct version if appropriate,
    // or it might be better to have agents handle normalization internally via configs.
    TArray<float> NormalizedData = Data;
    for (int32 i = 0; i < NormalizedData.Num(); ++i)
    {
        float Mean = (NormParams.Mean.IsValidIndex(i) ? NormParams.Mean[i] : (NormParams.Mean.Num() > 0 ? NormParams.Mean[0] : 0.0f));
        float StdDev = (NormParams.StdDev.IsValidIndex(i) ? NormParams.StdDev[i] : (NormParams.StdDev.Num() > 0 ? NormParams.StdDev[0] : 1.0f));
        if (FMath::IsNearlyZero(StdDev))
        {
            UE_LOG(LOG_UERLTOOLS, Warning, TEXT("Blueprint_NormalizeData: Standard deviation is zero or near zero for index %d. Skipping normalization for this element."), i);
            continue;
        }
        NormalizedData[i] = (Data[i] - Mean) / StdDev;
    }
    return NormalizedData;
}

TArray<float> UURLToolsBlueprintFunctionLibrary::Blueprint_DenormalizeData(const TArray<float>& NormalizedData, const FRLNormalizationParams& NormParams)
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Blueprint_DenormalizeData called."));
    if (!NormParams.bIsEnabled)
    {
        return NormalizedData; // Return original data if normalization is disabled (effectively denormalized)
    }

    TArray<float> DenormalizedData = NormalizedData;
    for (int32 i = 0; i < DenormalizedData.Num(); ++i)
    {
        float Mean = (NormParams.Mean.IsValidIndex(i) ? NormParams.Mean[i] : (NormParams.Mean.Num() > 0 ? NormParams.Mean[0] : 0.0f));
        float StdDev = (NormParams.StdDev.IsValidIndex(i) ? NormParams.StdDev[i] : (NormParams.StdDev.Num() > 0 ? NormParams.StdDev[0] : 1.0f));
        DenormalizedData[i] = (NormalizedData[i] * StdDev) + Mean;
    }
    return DenormalizedData;
}

// --- Debugging and Visualization ---
FString UURLToolsBlueprintFunctionLibrary::Blueprint_GetRLMatrixAsString(const FString& RLMatrixRepresentation)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("Blueprint_GetRLMatrixAsString: Placeholder implementation for: %s"), *RLMatrixRepresentation);
    return FString::Printf(TEXT("String Rep of: %s [Further details TBD]"), *RLMatrixRepresentation);
}

bool UURLToolsBlueprintFunctionLibrary::Blueprint_CheckRLMatrixDimensions(const FString& RLMatrixRepresentation, int32& OutRows, int32& OutCols)
{
    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("Blueprint_CheckRLMatrixDimensions: Placeholder implementation for: %s"), *RLMatrixRepresentation);
    OutRows = -1;
    OutCols = -1;
    // Actual logic would parse RLMatrixRepresentation or use a handle to get dimensions.
    return false; // Placeholder, return true if dimensions are successfully retrieved.
}

// --- Configuration Utilities ---
FRLTrainingConfig UURLToolsBlueprintFunctionLibrary::Blueprint_MakeDefaultTrainingConfig()
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Blueprint_MakeDefaultTrainingConfig called."));
    return FRLTrainingConfig(); // Returns a default-constructed FRLTrainingConfig
}

bool UURLToolsBlueprintFunctionLibrary::Blueprint_ValidateTrainingConfig(const FRLTrainingConfig& Config, FString& OutErrorMessage)
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Blueprint_ValidateTrainingConfig called."));
    // Basic validation example (can be expanded significantly)
    if (Config.BatchSize <= 0)
    {
        OutErrorMessage = TEXT("BatchSize must be greater than 0.");
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("Validation Failed (FRLTrainingConfig): %s"), *OutErrorMessage);
        return false;
    }
    if (Config.LearningRate <= 0.0f)
    {
        OutErrorMessage = TEXT("LearningRate must be greater than 0.");
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("Validation Failed (FRLTrainingConfig): %s"), *OutErrorMessage);
        return false;
    }
    // Add more validation rules as needed

    OutErrorMessage = TEXT("Configuration is valid.");
    return true;
}

FRLEnvironmentConfig UURLToolsBlueprintFunctionLibrary::Blueprint_MakeDefaultEnvironmentConfig()
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Blueprint_MakeDefaultEnvironmentConfig called."));
    return FRLEnvironmentConfig(); // Returns a default-constructed FRLEnvironmentConfig
}

bool UURLToolsBlueprintFunctionLibrary::Blueprint_ValidateEnvironmentConfig(const FRLEnvironmentConfig& Config, FString& OutErrorMessage)
{
    UE_LOG(LOG_UERLTOOLS, Log, TEXT("Blueprint_ValidateEnvironmentConfig called."));
    // Basic validation example
    if (Config.MaxEpisodeSteps < 0 && Config.MaxEpisodeSteps != -1) // -1 might mean infinite
    {
        OutErrorMessage = TEXT("MaxEpisodeSteps must be non-negative or -1 (for infinite).");
        UE_LOG(LOG_UERLTOOLS, Warning, TEXT("Validation Failed (FRLEnvironmentConfig): %s"), *OutErrorMessage);
        return false;
    }
    // Add more validation rules as needed (e.g., observation/action space consistency)

    OutErrorMessage = TEXT("Configuration is valid.");
    return true;
}
