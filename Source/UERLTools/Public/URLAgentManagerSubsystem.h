#pragma once

// Forward declarations
class URLEnvironmentComponent; // Assuming this is defined in RLEnvironmentComponent.h
struct FRLTrainingConfig;     // Assuming this USTRUCT is defined, e.g., in RLTypes.h
class URLAgentManager;        // Forward declaration for URLAgentManager

THIRD_PARTY_INCLUDES_START
#include <rl_tools/operations/cpu.h> // For rl_tools::devices::DefaultCPU, rl_tools::malloc, rl_tools::free, rl_tools::init
THIRD_PARTY_INCLUDES_END

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "URLAgentManagerSubsystem.generated.h"



// DECLARE_LOG_CATEGORY_EXTERN(LogUERLTools, Log, All); // Defined in UERLToolsLog.h or similar

// Delegate declarations for Blueprint events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAgentTrainingStepCompleted, FName, AgentName, int32, Step, float, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentTrainingFinished, FName, AgentName, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentPolicySaved, FName, AgentName, const FString&, FilePath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentPolicyLoaded, FName, AgentName, const FString&, FilePath);





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
    bool CreateAgent(FName AgentName, URLEnvironmentComponent* EnvironmentComponent, const FRLTrainingConfig& TrainingConfig);

    UFUNCTION(BlueprintCallable, Category = "RLTools|Agent Management", meta = (DeprecatedFunction, DeprecationMessage="Use CreateAgent instead."))
    bool ConfigureAgent(FName AgentName, URLEnvironmentComponent* EnvironmentComponent, const FRLTrainingConfig& TrainingConfig); // Kept for backward compatibility during refactor, to be removed

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
    UPROPERTY() // Keep TMap private, expose via functions. URLAgentManager instances are UObjects and will be managed by GC if UPROPERTY.
    TMap<FName, URLAgentManager*> ActiveAgents;

    // TODO: Add rl_tools global device context if needed
    // rl_tools global device and context
    rl_tools::devices::DefaultCPU rlt_device; // rl_tools device instance
    rl_tools::devices::DefaultCPU::CONTEXT_TYPE* rlt_context = nullptr; // Pointer to the global rl_tools context for CPU operations
};
