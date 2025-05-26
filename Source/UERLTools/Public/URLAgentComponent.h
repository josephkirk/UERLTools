// Copyright 2025 NGUYEN PHI HUNG

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RLTypes.h" // For FRLAgentConfig if defined, or other shared types
#include "URLAgentComponent.generated.h"

class URLEnvironmentComponent;
class URLAgentManagerSubsystem; // Forward declaration

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UERLTOOLS_API UURLAgentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UURLAgentComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Configuration for the agent, to be linked with URLAgentManagerSubsystem
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL Agent")
    FName AgentId; // Unique identifier for this agent instance

    // Optional: Direct reference to the environment this agent interacts with.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL Agent")
    TObjectPtr<URLEnvironmentComponent> AssociatedEnvironment;

    // TODO: UPROPERTY for FRLAgentConfig AgentConfig; (once FRLAgentConfig is defined in RLTypes.h as per task 2.3.2)

    UFUNCTION(BlueprintCallable, Category = "RL Agent")
    void InitializeAgent(); // Registers with the manager, sets up initial state

    /** 
     * Call this to request an action from the agent's policy.
     * The action will be delivered asynchronously via the ReceiveAction method or a delegate.
     */
    UFUNCTION(BlueprintCallable, Category = "RL Agent")
    void RequestAction();

    // Called by the manager when a new policy is available or loaded
    UFUNCTION()
    void OnPolicyUpdated(); // Placeholder

    // Called by the manager when an action is computed for this agent
    // TODO: Consider making this a BlueprintAssignable delegate for flexibility
    UFUNCTION()
    void ReceiveAction(const TArray<float>& Action); // Placeholder

private:
    UPROPERTY()
    TObjectPtr<URLAgentManagerSubsystem> AgentManager;

    // Helper to get the Agent Manager Subsystem
    URLAgentManagerSubsystem* GetAgentManager() const;
};
