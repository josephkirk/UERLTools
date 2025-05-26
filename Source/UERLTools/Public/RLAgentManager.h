// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "RLEnvironmentComponent.h"
#include "RLConfigTypes.h" // Added for FRLNormalizationParams
#include "UEEnvironmentAdapter.h" // Added for UEEnvironmentAdapter

THIRD_PARTY_INCLUDES_START
#include "rl_tools/operations/cpu_mux.h"
#include "rl_tools/devices/cpu.h"
#include "rl_tools/nn/layers/dense/layer.h"
#include "rl_tools/nn_models/mlp/network.h"
#include "rl_tools/rl/algorithms/td3/td3.h"
#include "rl_tools/rl/components/off_policy_runner/off_policy_runner.h"
#include "rl_tools/rl/components/replay_buffer/replay_buffer.h"
THIRD_PARTY_INCLUDES_END

#include "RLAgentManager.generated.h"

// Forward declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrainingStep, int32, Step, float, AverageReward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrainingFinished, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPolicyLoaded, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPolicySaved, bool, bSuccess);

/**
 * Training configuration structure
 */
USTRUCT(BlueprintType)
struct UERLTOOLS_API FRLTrainingConfig
{
	GENERATED_BODY()

	// Number of training steps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
	int32 MaxTrainingSteps = 100000;

	// Learning rate for actor network
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
	float ActorLearningRate = 0.0003f;

	// Learning rate for critic network
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
	float CriticLearningRate = 0.0003f;

	// Discount factor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
	float Gamma = 0.99f;

	// Batch size for training
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
	int32 BatchSize = 256;

	// Replay buffer capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
	int32 ReplayBufferCapacity = 1000000;

	// Steps between training updates
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
	int32 TrainingInterval = 1;

	// Steps to collect before starting training
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
	int32 WarmupSteps = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Normalization")
	FRLNormalizationParams ObservationNormalizationParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Normalization")
	FRLNormalizationParams ActionNormalizationParams;

	FRLTrainingConfig()
	{
		MaxTrainingSteps = 100000;
		ActorLearningRate = 0.0003f;
		CriticLearningRate = 0.0003f;
		Gamma = 0.99f;
		BatchSize = 256;
		ReplayBufferCapacity = 1000000;
		TrainingInterval = 1;
		WarmupSteps = 10000;
	}
};

/**
 * Training status information
 */
USTRUCT(BlueprintType)
struct UERLTOOLS_API FRLTrainingStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Training Status")
	bool bIsTraining = false;

	UPROPERTY(BlueprintReadOnly, Category = "Training Status")
	int32 CurrentStep = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Training Status")
	int32 CurrentEpisode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Training Status")
	float AverageReward = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Training Status")
	float LastEpisodeReward = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Training Status")
	int32 ReplayBufferSize = 0;
};

/**
 * RL Agent Manager that handles training and inference using rl_tools
 */
UCLASS(BlueprintType, Blueprintable)
class UERLTOOLS_API URLAgentManager : public UObject
{
	GENERATED_BODY()

public:
	URLAgentManager();

	// Training configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
	FRLTrainingConfig TrainingConfig;

	// Current training status
	UPROPERTY(BlueprintReadOnly, Category = "Training")
	FRLTrainingStatus TrainingStatus;

	// Blueprint Events
	UPROPERTY(BlueprintAssignable, Category = "Training Events")
	FOnTrainingStep OnTrainingStep;

	UPROPERTY(BlueprintAssignable, Category = "Training Events")
	FOnTrainingFinished OnTrainingFinished;

	UPROPERTY(BlueprintAssignable, Category = "Policy Events")
	FOnPolicyLoaded OnPolicyLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Policy Events")
	FOnPolicySaved OnPolicySaved;

	// Initialization
	UFUNCTION(BlueprintCallable, Category = "Agent")
	bool InitializeAgent(URLEnvironmentComponent* InEnvironmentComponent, const FRLTrainingConfig& InTrainingConfig);

	// Training functions
	UFUNCTION(BlueprintCallable, Category = "Training")
	bool StartTraining();

	UFUNCTION(BlueprintCallable, Category = "Training")
	void PauseTraining();

	UFUNCTION(BlueprintCallable, Category = "Training")
	void ResumeTraining();

	UFUNCTION(BlueprintCallable, Category = "Training")
	void StopTraining();

	UFUNCTION(BlueprintCallable, Category = "Training")
	bool StepTraining(int32 NumSteps = 1);

	// Inference functions
	UFUNCTION(BlueprintCallable, Category = "Inference")
	TArray<float> GetAction(const TArray<float>& Observation);

	// Policy management
	UFUNCTION(BlueprintCallable, Category = "Policy")
	bool LoadPolicy(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Policy")
	bool SavePolicy(const FString& FilePath);

	// Status and utility functions
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Training")
	FRLTrainingStatus GetTrainingStatus() const { return TrainingStatus; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Training")
	bool IsTraining() const { return TrainingStatus.bIsTraining; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Agent")
	bool IsInitialized() const { return bIsInitialized; }

protected:
	// rl_tools types and constants
	using DEVICE = rlt::devices::DefaultCPU;
	using T = float;
	using TI = typename DEVICE::index_t;

	// Define Environment Specification for rl_tools types
	struct UERLAgentEnvironmentSpec {
		// Inherit T and TI from the outer scope (URLAgentManager protected section)

		// PLACEHOLDER DIMENSIONS: These MUST be validated against the URLEnvironmentComponent instance at runtime.
		// Adjust these to your most common environment configuration.
		static constexpr TI OBSERVATION_DIM = 4; // Example placeholder
		static constexpr TI ACTION_DIM = 2;    // Example placeholder

		struct OBSERVATION_SPEC {
			using T = UERLAgentEnvironmentSpec::T;
			using TI = UERLAgentEnvironmentSpec::TI;
			static constexpr TI DIM = UERLAgentEnvironmentSpec::OBSERVATION_DIM;
			using SPEC = rlt::matrix::Specification<T, TI, 1, DIM>;
			template <typename CONFIG> using Matrix = rlt::MatrixStatic<CONFIG>; // Assuming fixed size observations
		};
		struct ACTION_SPEC {
			using T = UERLAgentEnvironmentSpec::T;
			using TI = UERLAgentEnvironmentSpec::TI;
			static constexpr TI DIM = UERLAgentEnvironmentSpec::ACTION_DIM;
			using SPEC = rlt::matrix::Specification<T, TI, 1, DIM>;
			template <typename CONFIG> using Matrix = rlt::MatrixStatic<CONFIG>; // Assuming fixed size actions
		};
		// Note: The UEEnvironmentAdapter itself defines its State struct internally.
		// This UERLAgentEnvironmentSpec is primarily for configuring other rl_tools components
		// that depend on environment properties like dimensions.
	};

	// rl_tools types and constants (continuation)
	// T and TI are already defined above, no need to repeat here.
	
	// Network architecture constants
	static constexpr TI HIDDEN_DIM = 64;
	static constexpr TI NUM_LAYERS = 2;
	static constexpr auto ACTIVATION_FUNCTION = rlt::nn::activation_functions::ActivationFunction::RELU;

	// Actor network type
	// These specs depend on the environment's observation and action dimensions for input/output layers.
	// However, rl_tools MLP structures are often defined by hidden layers, input/output are inferred or set at instantiation.
	// For now, let's assume these are generic MLP structures.
	using ACTOR_STRUCTURE_SPEC = rlt::nn_models::mlp::StructureSpecification<T, TI, UERLAgentEnvironmentSpec::OBSERVATION_DIM, UERLAgentEnvironmentSpec::ACTION_DIM, HIDDEN_DIM, NUM_LAYERS, ACTIVATION_FUNCTION, rlt::nn::activation_functions::TANH>; // Actor output usually tanh
	using ACTOR_SPEC = rlt::nn_models::mlp::AdamSpecification<ACTOR_STRUCTURE_SPEC>;
	using ACTOR_TYPE = rlt::nn_models::mlp::NeuralNetworkAdam<ACTOR_SPEC>;

	// Critic network type (takes observation and action as input)
	// The input dimension for the critic is ObservationDim + ActionDim.
	// We'll need a custom StructureSpecification or handle this during instantiation if rl_tools doesn't directly support it in the generic MLP spec.
	// For simplicity, we'll assume a critic structure that can be configured or adapted.
	// This might require a more specific critic network structure definition later.
	using CRITIC_STRUCTURE_SPEC = rlt::nn_models::mlp::StructureSpecification<T, TI, UERLAgentEnvironmentSpec::OBSERVATION_DIM + UERLAgentEnvironmentSpec::ACTION_DIM, 1, HIDDEN_DIM, NUM_LAYERS, ACTIVATION_FUNCTION, rlt::nn::activation_functions::IDENTITY>; // Critic output is Q-value
	using CRITIC_SPEC = rlt::nn_models::mlp::AdamSpecification<CRITIC_STRUCTURE_SPEC>;
	using CRITIC_TYPE = rlt::nn_models::mlp::NeuralNetworkAdam<CRITIC_SPEC>;

	// TD3 Parameters
	using TD3_PARAMETERS = rlt::rl::algorithms::td3::Parameters<T, TI>;

	// Actor-Critic type
	using ACTOR_CRITIC_SPEC = rlt::rl::algorithms::td3::Specification<T, TI, UERLAgentEnvironmentSpec, ACTOR_TYPE, CRITIC_TYPE, TD3_PARAMETERS>;
	using ACTOR_CRITIC_TYPE = rlt::rl::algorithms::td3::ActorCritic<ACTOR_CRITIC_SPEC>;

	// Replay Buffer
	static constexpr TI REPLAY_BUFFER_CAPACITY = 100000; // Example capacity
	using REPLAY_BUFFER_SPEC = rlt::rl::components::replay_buffer::Specification<T, TI, UERLAgentEnvironmentSpec::OBSERVATION_DIM, UERLAgentEnvironmentSpec::ACTION_DIM, REPLAY_BUFFER_CAPACITY>;
	using REPLAY_BUFFER_TYPE = rlt::rl::components::ReplayBuffer<REPLAY_BUFFER_SPEC>;

	// Off-Policy Runner
	// The OffPolicyRunner needs the actual UEEnvironmentAdapter type, not just its spec.
	// Let's define the adapter type first.
	using ENVIRONMENT_ADAPTER_TYPE = UEEnvironmentAdapter<DEVICE, UERLAgentEnvironmentSpec>;
	using OFF_POLICY_RUNNER_SPEC = rlt::rl::components::off_policy_runner::Specification<T, TI, ENVIRONMENT_ADAPTER_TYPE, REPLAY_BUFFER_TYPE::CAPACITY, typename ACTOR_CRITIC_TYPE::ParametersType::OFF_POLICY_RUNNER_PARAMETERS_TYPE>;
    using OFF_POLICY_RUNNER_TYPE = rlt::rl::components::OffPolicyRunner<OFF_POLICY_RUNNER_SPEC>;

private:
    // rl_tools instances
    DEVICE device; // The rl_tools device
    ENVIRONMENT_ADAPTER_TYPE* EnvironmentAdapterInstance = nullptr;

	UPROPERTY()
	URLEnvironmentComponent* EnvironmentComponent;

	// Initialization flag
	bool bIsInitialized;

	// Training state
	bool bTrainingPaused;
	int32 EpisodeStepCount;
	float EpisodeReward;
	TArray<float> EpisodeRewards;

	// Current observation and action
	TArray<float> CurrentObservation;
	TArray<float> CurrentAction;

	// Actor and Critic networks (will be allocated dynamically)
	ACTOR_TYPE* ActorNetwork;
	CRITIC_TYPE* CriticNetwork;

	// Helper functions
	void UpdateTrainingStatus();
	void LogTrainingProgress();
	bool ValidateEnvironment() const;
	void CleanupNetworks();

	// Training step implementation
	bool PerformTrainingStep();
	void CollectExperience();
	void UpdateNetworks();
};
