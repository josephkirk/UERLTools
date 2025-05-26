// Copyright Windsurf Engineering, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RLConfigTypes.h" // For FRLNormalizationParams
#include "RLTypes.generated.h"

/**
 * Configuration for the RL environment.
 */
USTRUCT(BlueprintType)
struct UERLTOOLS_API FRLEnvironmentConfig
{
    GENERATED_BODY()

    /** Maximum number of steps per episode. -1 for infinite. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    int32 MaxEpisodeSteps = 1000;

    // Add other environment-specific configurations here, e.g.:
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    // float TimeScale = 1.0f;

    FRLEnvironmentConfig() = default;
};

/**
 * Configuration for the RL training process.
 */
USTRUCT(BlueprintType)
struct UERLTOOLS_API FRLTrainingConfig
{
    GENERATED_BODY()

    /** Learning rate for the optimizer. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    float LearningRate = 0.0003f;

    /** Discount factor for future rewards (gamma). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    float DiscountFactor = 0.99f;

    /** Batch size for training updates. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    int32 BatchSize = 64;

    /** Total number of timesteps to train for. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    int32 TotalTimesteps = 1000000;

    /** Parameters for observation normalization. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Normalization")
    FRLNormalizationParams ObservationNormalizationParams;

    /** Parameters for action normalization (if applicable, e.g., for continuous actions). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Normalization")
    FRLNormalizationParams ActionNormalizationParams;

    // Add other algorithm-specific hyperparameters here, e.g.:
    // For PPO:
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|PPO")
    // float ClipRatio = 0.2f;
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|PPO")
    // int32 PPOEpochs = 10;

    FRLEnvironmentConfig EnvironmentConfig; // Associated environment configuration

    FRLTrainingConfig() = default;
};
