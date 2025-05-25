// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

THIRD_PARTY_INCLUDES_START
#include "rl_tools/operations/cpu_mux.h"
#include "rl_tools/devices/cpu.h"
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
	rlt::devices::DefaultCPU device;
};
