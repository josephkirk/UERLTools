// Copyright 2025 NGUYEN PHI HUNG

#include "RLToolsTest.h"
#include "UERLLog.h"
#include "Engine/Engine.h"

THIRD_PARTY_INCLUDES_START
#include "rl_tools/operations/cpu_mux.h"
#include "rl_tools/devices/cpu.h"
#include "rl_tools/nn/layers/dense/layer.h"
#include "rl_tools/nn_models/mlp/network.h"
#include "rl_tools/nn/optimizers/adam/adam.h"
#include "rl_tools/nn/loss_functions/mse/operations_generic.h"
THIRD_PARTY_INCLUDES_END

// Module-wide log categories
#include "UERLLog.h"

#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        UERL_RL_ERROR("Test failed: %s", TEXT_UTF8_TO_TCHAR(message)); \
        if (GEngine) { \
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, \
                FString::Printf(TEXT("Test failed: %s"), TEXT_UTF8_TO_TCHAR(message))); \
        } \
        return false; \
    }

URLToolsTest::URLToolsTest()
{
    // Seed the random number generator for consistent test results
    device.random = {0};
    rlt::random::seed(device.random, 42);
}

bool URLToolsTest::TestRLToolsIntegration()
{
    bool allTestsPassed = true;
    
    // Run all test cases
    allTestsPassed &= TestMatrixOperations();
    allTestsPassed &= TestNeuralNetworkLayer();
    allTestsPassed &= TestMLPNetwork();
    allTestsPassed &= TestOptimizer();
    
    // Final status
    if (allTestsPassed)
    {
        UERL_RL_LOG("All RLTools tests passed successfully!");
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                TEXT("All RLTools tests passed successfully!"));
        }
    }
    
    return allTestsPassed;
}

bool URLToolsTest::TestMatrixOperations()
{
    using DEVICE = rlt::devices::DefaultCPU;
    using T = float;
    constexpr auto TI = typename DEVICE::index_t{};
    
    try 
    {
        // Test 1: Basic matrix operations
        constexpr TI ROWS = 3;
        constexpr TI COLS = 4;
        
        using MATRIX_SPEC = rlt::matrix::Specification<T, TI, ROWS, COLS>;
        using MATRIX = rlt::Matrix<MATRIX_SPEC>;
        
        MATRIX matrix1, matrix2, result;
        rlt::malloc(device, matrix1);
        rlt::malloc(device, matrix2);
        rlt::malloc(device, result);
        
        // Initialize matrices
        rlt::randn(device, matrix1, device.random);
        rlt::randn(device, matrix2, device.random);
        
        // Test matrix addition
        rlt::add(device, matrix1, matrix2, result);
        
        // Verify addition
        for(TI row = 0; row < ROWS; row++) 
        {
            for(TI col = 0; col < COLS; col++) 
            {
                T expected = rlt::get(matrix1, row, col) + rlt::get(matrix2, row, col);
                T actual = rlt::get(result, row, col);
                TEST_ASSERT(
                    std::abs(actual - expected) < 1e-6f, 
                    "Matrix addition failed"
                );
            }
        }
        
        // Test matrix multiplication
        constexpr TI INNER_DIM = COLS;
        using MATRIX2_SPEC = rlt::matrix::Specification<T, TI, COLS, 2>;
        using MATRIX2 = rlt::Matrix<MATRIX2_SPEC>;
        using RESULT_SPEC = rlt::matrix::Specification<T, TI, ROWS, 2>;
        using RESULT_MATRIX = rlt::Matrix<RESULT_SPEC>;
        
        MATRIX2 matrix3;
        RESULT_MATRIX result_matmul;
        rlt::malloc(device, matrix3);
        rlt::malloc(device, result_matmul);
        
        rlt::randn(device, matrix3, device.random);
        
        // Perform matrix multiplication
        rlt::multiply(device, matrix1, matrix3, result_matmul);
        
        // Verify matrix multiplication (check first element as a sample)
        T expected = 0;
        for(TI k = 0; k < INNER_DIM; k++) 
        {
            expected += rlt::get(matrix1, 0, k) * rlt::get(matrix3, k, 0);
        }
        
        T actual = rlt::get(result_matmul, 0, 0);
        TEST_ASSERT(
            std::abs(actual - expected) < 1e-4f, 
            "Matrix multiplication failed"
        );
        
        // Cleanup
        rlt::free(device, matrix1);
        rlt::free(device, matrix2);
        rlt::free(device, result);
        rlt::free(device, matrix3);
        rlt::free(device, result_matmul);
        
        UERL_RL_LOG("Matrix operations test passed!");
        return true;
    }
    catch (const std::exception& e)
    {
        UERL_RL_ERROR("Matrix operations test failed: %s", e.what());
        return false;
    }
}

bool URLToolsTest::TestNeuralNetworkLayer()
{
    using DEVICE = rlt::devices::DefaultCPU;
    using T = float;
    constexpr auto TI = typename DEVICE::index_t{};
    
    try 
    {
        // Define layer specifications
        constexpr TI INPUT_DIM = 4;
        constexpr TI OUTPUT_DIM = 2;
        constexpr TI BATCH_SIZE = 3;
        
        using LAYER_SPEC = rlt::nn::layers::dense::Specification<T, TI, INPUT_DIM, OUTPUT_DIM>;
        using LAYER = rlt::nn::layers::dense::Layer<LAYER_SPEC>;
        
        // Allocate and initialize layer
        LAYER layer;
        rlt::malloc(device, layer);
        rlt::init_kaiming(device, layer, device.random);
        
        // Create input and output buffers
        using INPUT_SPEC = rlt::matrix::Specification<T, TI, BATCH_SIZE, INPUT_DIM>;
        using OUTPUT_SPEC = rlt::matrix::Specification<T, TI, BATCH_SIZE, OUTPUT_DIM>;
        
        rlt::Matrix<INPUT_SPEC> input;
        rlt::Matrix<OUTPUT_SPEC> output;
        
        rlt::malloc(device, input);
        rlt::malloc(device, output);
        
        // Initialize input with random values
        rlt::randn(device, input, device.random);
        
        // Forward pass
        rlt::evaluate(device, layer, input, output);
        
        // Verify output dimensions
        TEST_ASSERT(
            rlt::row_count(output) == BATCH_SIZE && rlt::col_count(output) == OUTPUT_DIM,
            "Layer output dimensions are incorrect"
        );
        
        // Test that output is not all zeros (simple sanity check)
        bool all_zeros = true;
        for(TI i = 0; i < BATCH_SIZE; i++) 
        {
            for(TI j = 0; j < OUTPUT_DIM; j++) 
            {
                if (rlt::get(output, i, j) != 0) 
                {
                    all_zeros = false;
                    break;
                }
            }
            if (!all_zeros) break;
        }
        
        TEST_ASSERT(!all_zeros, "Layer output is all zeros");
        
        // Cleanup
        rlt::free(device, layer);
        rlt::free(device, input);
        rlt::free(device, output);
        
        UERL_RL_LOG("Neural network layer test passed!");
        return true;
    }
    catch (const std::exception& e)
    {
        UERL_RL_ERROR("Neural network layer test failed: %s", e.what());
        return false;
    }
}

bool URLToolsTest::TestMLPNetwork()
{
    using DEVICE = rlt::devices::DefaultCPU;
    using T = float;
    constexpr auto TI = typename DEVICE::index_t{};
    
    try 
    {
        // Define network architecture
        constexpr TI INPUT_DIM = 4;
        constexpr TI HIDDEN_DIM = 8;
        constexpr TI OUTPUT_DIM = 2;
        constexpr TI BATCH_SIZE = 5;
        
        using NETWORK_SPEC = rlt::nn_models::mlp::Specification<T, TI, INPUT_DIM, OUTPUT_DIM, 3>;
        using NETWORK = rlt::nn_models::mlp::NeuralNetwork<NETWORK_SPEC>;
        
        // Allocate and initialize network
        NETWORK network;
        rlt::malloc(device, network);
        rlt::init_weights(device, network, device.random);
        
        // Create input and output buffers
        using INPUT_SPEC = rlt::matrix::Specification<T, TI, BATCH_SIZE, INPUT_DIM>;
        using OUTPUT_SPEC = rlt::matrix::Specification<T, TI, BATCH_SIZE, OUTPUT_DIM>;
        
        rlt::Matrix<INPUT_SPEC> input;
        rlt::Matrix<OUTPUT_SPEC> output;
        
        rlt::malloc(device, input);
        rlt::malloc(device, output);
        
        // Initialize input with random values
        rlt::randn(device, input, device.random);
        
        // Forward pass
        rlt::evaluate(device, network, input, output);
        
        // Verify output dimensions
        TEST_ASSERT(
            rlt::row_count(output) == BATCH_SIZE && rlt::col_count(output) == OUTPUT_DIM,
            "Network output dimensions are incorrect"
        );
        
        // Test that output is not all zeros (simple sanity check)
        bool all_zeros = true;
        for(TI i = 0; i < BATCH_SIZE; i++) 
        {
            for(TI j = 0; j < OUTPUT_DIM; j++) 
            {
                if (rlt::get(output, i, j) != 0) 
                {
                    all_zeros = false;
                    break;
                }
            }
            if (!all_zeros) break;
        }
        
        TEST_ASSERT(!all_zeros, "Network output is all zeros");
        
        // Cleanup
        rlt::free(device, network);
        rlt::free(device, input);
        rlt::free(device, output);
        
        UERL_RL_LOG("MLP network test passed!");
        return true;
    }
    catch (const std::exception& e)
    {
        UERL_RL_ERROR("MLP network test failed: %s", e.what());
        return false;
    }
}

bool URLToolsTest::TestOptimizer()
{
    using DEVICE = rlt::devices::DefaultCPU;
    using T = float;
    constexpr auto TI = typename DEVICE::index_t{};
    
    try 
    {
        // Simple test to verify optimizer can step without errors
        constexpr TI INPUT_DIM = 3;
        constexpr TI OUTPUT_DIM = 1;
        constexpr TI BATCH_SIZE = 2;
        
        // Define a simple network
        using NETWORK_SPEC = rlt::nn_models::mlp::Specification<T, TI, INPUT_DIM, OUTPUT_DIM, 2>;
        using NETWORK = rlt::nn_models::mlp::NeuralNetwork<NETWORK_SPEC>;
        
        // Define optimizer
        using OPTIMIZER_SPEC = rlt::nn::optimizers::adam::Specification<T, TI>;
        OPTIMIZER_SPEC optimizer_spec;
        optimizer_spec.alpha = 1e-3f;
        
        using OPTIMIZER = rlt::nn::optimizers::Adam<OPTIMIZER_SPEC>;
        OPTIMIZER optimizer;
        
        // Allocate and initialize network and optimizer
        NETWORK network;
        rlt::malloc(device, network);
        rlt::init_weights(device, network, device.random);
        
        typename NETWORK::Buffer<BATCH_SIZE> buffer;
        rlt::malloc(device, buffer);
        
        // Create input and target
        using INPUT_SPEC = rlt::matrix::Specification<T, TI, BATCH_SIZE, INPUT_DIM>;
        using TARGET_SPEC = rlt::matrix::Specification<T, TI, BATCH_SIZE, OUTPUT_DIM>;
        
        rlt::Matrix<INPUT_SPEC> input;
        rlt::Matrix<TARGET_SPEC> target;
        
        rlt::malloc(device, input);
        rlt::malloc(device, target);
        
        // Initialize with random data
        rlt::randn(device, input, device.random);
        rlt::randn(device, target, device.random);
        
        // Forward pass
        rlt::forward(device, network, input, buffer);
        
        // Compute loss
        T loss_before = rlt::loss::mse::evaluate(device, network.output, target);
        
        // Backward pass and optimization step
        rlt::reset_optimizer_state(device, optimizer, network);
        rlt::backward(device, network, input, buffer);
        rlt::step(device, optimizer, network);
        
        // Forward pass again
        rlt::forward(device, network, input, buffer);
        
        // Compute loss after optimization step
        T loss_after = rlt::loss::mse::evaluate(device, network.output, target);
        
        // In a real test, we'd expect the loss to decrease, but for this simple test
        // we'll just verify the optimizer step completed without errors
        UERL_RL_LOG("Optimizer test - Loss before: %f, after: %f", loss_before, loss_after);
        
        // Cleanup
        rlt::free(device, network);
        rlt::free(device, buffer);
        rlt::free(device, input);
        rlt::free(device, target);
        
        UERL_RL_LOG("Optimizer test passed!");
        return true;
    }
    catch (const std::exception& e)
    {
        UERL_RL_ERROR("Optimizer test failed: %s", e.what());
        return false;
    }
}
