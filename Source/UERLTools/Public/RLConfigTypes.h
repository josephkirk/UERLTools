// Copyright 2025 NGUYEN PHI HUNG

#pragma once

#include "CoreMinimal.h"
#include "RLConfigTypes.generated.h"

/**
 * Parameters for normalizing or denormalizing observation/action data.
 */
USTRUCT(BlueprintType)
struct UERLTOOLS_API FRLNormalizationParams
{
    GENERATED_BODY()

    /** Whether normalization/denormalization should be applied. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Normalization")
    bool bIsEnabled = false;

    /** 
     * Mean values for each dimension of the data. 
     * If empty or fewer elements than data dimension, normalization might be skipped or use default (0).
     * For single value normalization across all dimensions, provide a single element array.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Normalization")
    TArray<float> Mean;

    /** 
     * Standard deviation values for each dimension of the data. 
     * If empty or fewer elements than data dimension, normalization might be skipped or use default (1).
     * For single value normalization across all dimensions, provide a single element array.
     * Values should be positive; small values might lead to instability.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Normalization")
    TArray<float> StdDev;

    FRLNormalizationParams() = default;
};
