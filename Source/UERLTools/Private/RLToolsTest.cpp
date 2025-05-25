// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLToolsTest.h"
#include "Engine/Engine.h"

THIRD_PARTY_INCLUDES_START
#include "rl_tools/operations/cpu_mux.h"
#include "rl_tools/devices/cpu.h"
#include "rl_tools/nn/layers/dense/layer.h"
#include "rl_tools/nn_models/mlp/network.h"
THIRD_PARTY_INCLUDES_END

URLToolsTest::URLToolsTest()
{
	// Initialize the device
}

bool URLToolsTest::TestRLToolsIntegration()
{
	try
	{
		// Test basic rl_tools functionality
		using DEVICE = rlt::devices::DefaultCPU;
		using T = float;
		constexpr auto TI = typename DEVICE::index_t{};
		
		// Create a simple matrix to test basic operations
		constexpr TI ROWS = 3;
		constexpr TI COLS = 4;
		
		using MATRIX_SPEC = rlt::matrix::Specification<T, TI, ROWS, COLS>;
		using MATRIX = rlt::Matrix<MATRIX_SPEC>;
		
		MATRIX matrix;
		rlt::malloc(device, matrix);
		
		// Initialize matrix with some values
		rlt::randn(device, matrix, device.random);
		
		// Test some basic operations
		T sum = 0;
		for(TI row_i = 0; row_i < ROWS; row_i++)
		{
			for(TI col_i = 0; col_i < COLS; col_i++)
			{
				sum += rlt::get(matrix, row_i, col_i);
			}
		}
		
		rlt::free(device, matrix);
		
		// Log success message
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
				FString::Printf(TEXT("RLTools integration test successful! Matrix sum: %f"), sum));
		}
		
		UE_LOG(LogTemp, Log, TEXT("RLTools integration test successful! Matrix sum: %f"), sum);
		
		return true;
	}
	catch (...)
	{
		// Log error message
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
				TEXT("RLTools integration test failed!"));
		}
		
		UE_LOG(LogTemp, Error, TEXT("RLTools integration test failed!"));
		
		return false;
	}
}
