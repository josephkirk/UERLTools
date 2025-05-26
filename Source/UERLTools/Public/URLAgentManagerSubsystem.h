#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "URLAgentManagerSubsystem.generated.h"

// Forward declarations
class URLEnvironmentComponent; // Assuming this is defined in RLEnvironmentComponent.h
struct FRLTrainingConfig;     // Assuming this USTRUCT is defined, e.g., in RLTypes.h
// #include "rl_tools/operations_cpu.h" // Example for rl_tools types

// DECLARE_LOG_CATEGORY_EXTERN(LogUERLTools, Log, All); // Defined in UERLToolsLog.h or similar

// Delegate declarations for Blueprint events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAgentTrainingStepCompleted, FName, AgentName, int32, Step, float, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentTrainingFinished, FName, AgentName, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentPolicySaved, FName, AgentName, const FString&, FilePath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentPolicyLoaded, FName, AgentName, const FString&, FilePath);


/**
 * USTRUCT to hold context for each RL agent.
 * This will eventually contain rl_tools specific data structures like policy, optimizer, etc.
 */
USTRUCT(BlueprintType)
struct FRLAgentContext
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RL Agent Context")
    FName AgentName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RL Agent Context")
    TWeakObjectPtr<URLEnvironmentComponent> EnvironmentComponent;

    // TODO: Add rl_tools specific data members
    // e.g., typename DEVICE::CONTEXT* device_context;
    // e.g., typename POLICY::template Instance<...> policy;
    // e.g., typename OPTIMIZER::template Instance<...> optimizer;
    // e.g., typename ACTOR_CRITIC_TYPE::template Instance<...> actor_critic;

    // Placeholder for training status
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RL Agent Context")
    bool bIsTraining = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RL Agent Context")
    int32 CurrentTrainingStep = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RL Agent Context")
    float LastReward = 0.0f;

    FRLAgentContext() : AgentName(NAME_None), EnvironmentComponent(nullptr), bIsTraining(false), CurrentTrainingStep(0), LastReward(0.0f) {}
};


/**
 * Manages the lifecycle and operations of RL agents within the game instance.
 */
UCLASS(BlueprintType, Blueprintable)
class UERLTOOLS_API URLAgentManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    //~ Begin USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem interface

    // Agent Configuration & Management
    UFUNCTION(BlueprintCallable, Category = "RLTools|Agent Management")
    bool ConfigureAgent(FName AgentName, URLEnvironmentComponent* EnvironmentComponent, const FRLTrainingConfig& TrainingConfig);

    UFUNCTION(BlueprintCallable, Category = "RLTools|Agent Management")
    bool RemoveAgent(FName AgentName);

    // Policy Management
    UFUNCTION(BlueprintCallable, Category = "RLTools|Policy Management")
    bool LoadPolicy(FName AgentName, const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "RLTools|Policy Management")
    bool SavePolicy(FName AgentName, const FString& FilePath);

    // Training Control
    UFUNCTION(BlueprintCallable, Category = "RLTools|Training")
    bool StartTraining(FName AgentName);

    UFUNCTION(BlueprintCallable, Category = "RLTools|Training")
    bool PauseTraining(FName AgentName);

    UFUNCTION(BlueprintCallable, Category = "RLTools|Training")
    bool StopTraining(FName AgentName);

    // Inference
    UFUNCTION(BlueprintCallable, Category = "RLTools|Inference")
    TArray<float> GetAction(FName AgentName, const TArray<float>& Observation);

    // Status & Logging
    UFUNCTION(BlueprintCallable, Category = "RLTools|Status")
    bool GetAgentTrainingStatus(FName AgentName, bool&bIsCurrentlyTraining, int32& OutCurrentStep, float& OutLastReward);

    // UFUNCTION(BlueprintCallable, Category = "RLTools|Status")
    // FRLAgentEpisodeStats GetAgentEpisodeStats(FName AgentName); // Placeholder for more detailed stats struct

public:
    // Blueprint Assignable Delegates for asynchronous operations
    UPROPERTY(BlueprintAssignable, Category = "RLTools|Events")
    FOnAgentTrainingStepCompleted OnAgentTrainingStepCompleted;

    UPROPERTY(BlueprintAssignable, Category = "RLTools|Events")
    FOnAgentTrainingFinished OnAgentTrainingFinished;

    UPROPERTY(BlueprintAssignable, Category = "RLTools|Events")
    FOnAgentPolicySaved OnAgentPolicySaved;

    UPROPERTY(BlueprintAssignable, Category = "RLTools|Events")
    FOnAgentPolicyLoaded OnAgentPolicyLoaded;

private:
    UPROPERTY() // Keep TMap private, expose via functions
    TMap<FName, FRLAgentContext> ManagedAgents;

    // TODO: Add rl_tools global device context if needed
    // typename rl_tools::devices::DefaultCPU DEVICE;
    // typename DEVICE::CONTEXT* global_device_context = nullptr;
};
