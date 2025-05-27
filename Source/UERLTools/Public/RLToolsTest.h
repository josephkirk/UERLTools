// Copyright 2025 NGUYEN PHI HUNG

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

THIRD_PARTY_INCLUDES_START
#include "rl_tools/operations/cpu_mux.h"
#include "rl_tools/devices/cpu.h"
#include "rl_tools/nn/layers/dense/layer.h"
#include "rl_tools/nn_models/mlp/network.h"
#include "rl_tools/nn/optimizers/adam/adam.h"
THIRD_PARTY_INCLUDES_END

#include "RLToolsTest.generated.h"

/**
 * Test class to verify rl_tools integration
 */
UCLASS(BlueprintType, Blueprintable)
class UERLTOOLS_API URLToolsTest : public UObject
{
    GENERATED_BODY()

public:
    URLToolsTest();

    // Test function to verify rl_tools is working
    UFUNCTION(BlueprintCallable, Category = "RLTools Test")
    bool TestRLToolsIntegration();

private:
    // rl_tools device instance
    rl_tools::devices::DefaultCPU device;
    
    // Individual test cases
    bool TestMatrixOperations();
    bool TestNeuralNetworkLayer();
    bool TestMLPNetwork();
    bool TestOptimizer();
};
