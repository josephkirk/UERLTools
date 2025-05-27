// Copyright 2025 NGUYEN PHI HUNG

#include "URLAgentComponent.h"
#include "RLEnvironmentComponent.h"
#include "Subsystems/URLAgentManagerSubsystem.h"
#include "UERLLog.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Module-wide log categories
#include "UERLLog.h"

UURLAgentComponent::UURLAgentComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // No tick by default
    AgentId = NAME_None;
    AssociatedEnvironment = nullptr;
    AgentManager = nullptr;
}

void UURLAgentComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeAgent();
}

void UURLAgentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AgentManager && AgentId != NAME_None)
    {
        // TODO: Consider if RemoveAgent should be called here or managed explicitly by the user.
        // For now, we'll let the user manage removal to prevent accidental unregistration during PIE.
        // AgentManager->RemoveAgent(AgentId);
        UERL_URL_LOG("Agent component (%s) EndPlay. Consider manual RemoveAgent if needed.", *AgentId.ToString());
    }
    Super::EndPlay(EndPlayReason);
}

URLAgentManagerSubsystem* UURLAgentComponent::GetAgentManager() const
{
    if (AgentManager)
    {
        return AgentManager;
    }

    if (GetWorld())
    {
        return UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<URLAgentManagerSubsystem>();
    }
    return nullptr;
}

void UURLAgentComponent::InitializeAgent()
{
    AgentManager = GetAgentManager();
    if (!AgentManager)
    {
        UERL_URL_ERROR("Could not get URLAgentManagerSubsystem. Agent cannot be initialized.");
        return;
    }

    if (AgentId == NAME_None)
    {
        // Attempt to generate a unique ID based on the owner actor's name and this component's name
        AActor* OwnerActor = GetOwner();
        if (OwnerActor)
        {
            AgentId = FName(*(OwnerActor->GetName() + TEXT("_") + GetName()));
            UERL_URL_WARNING("AgentId was None, automatically set to '%s'. Consider setting a unique AgentId explicitly.", *AgentId.ToString());
        }
        else
        {
            UERL_URL_ERROR("AgentId is None and no owner actor to generate one. Agent cannot be initialized. Please set AgentId.");
            return;
        }
    }
    
    // TODO: When FRLAgentConfig is defined (Task 2.3.2), pass it here.
    // For now, we'll pass a default/empty config or just the ID.
    // FRLAgentConfig Config; // Populate this
    // AgentManager->ConfigureAgent(AgentId, Config, AssociatedEnvironment);
    
    // Placeholder until FRLAgentConfig is integrated.
    // We need a way to pass training configuration. For now, let's assume a default or no config.
    // This will likely need to be expanded when FRLTrainingConfig is properly integrated.
    FRLTrainingConfig DummyTrainingConfig; // This is a placeholder
    AgentManager->ConfigureAgent(AgentId, DummyTrainingConfig, AssociatedEnvironment);

    UERL_URL_LOG("Agent component (%s) initialized and registered with AgentManager.", *AgentId.ToString());

    // TODO: Subscribe to relevant delegates from AgentManager if needed, e.g., OnPolicyUpdated for this AgentId.
}

void UURLAgentComponent::RequestAction()
{
    if (!AgentManager)
    {
        UERL_URL_WARNING("Agent component (%s): AgentManager not available. Cannot request action.", *AgentId.ToString());
        return;
    }
    if (AgentId == NAME_None)
    {
        UERL_URL_WARNING("AgentId is None. Cannot request action. InitializeAgent first.");
        return;
    }

    // The AgentManager will handle getting the observation from the AssociatedEnvironment if needed,
    // or the observation might be pushed to the manager separately.
    // For now, GetAction just needs the AgentId.
    AgentManager->GetAction(AgentId);
    UERL_URL_LOG("Agent component (%s) requested action.", *AgentId.ToString());
}

void UURLAgentComponent::OnPolicyUpdated()
{
    // This might be called by the AgentManager when a new policy is loaded for this agent.
    UERL_URL_LOG("Agent component (%s): Policy updated (placeholder).", *AgentId.ToString());
    // TODO: Implement logic if the agent needs to react to policy updates directly.
}

void UURLAgentComponent::ReceiveAction(const TArray<float>& Action)
{
    // This would be called by the AgentManager after an action is computed.
    // Typically, the agent would then apply this action to its AssociatedEnvironment.
    UERL_URL_LOG("Agent component (%s): Received action (placeholder): %s", *AgentId.ToString(), 
        *FString::Join(Action.ConvertAll<FString>([](float val){ return FString::SanitizeFloat(val); }), TEXT(",")));

    if (AssociatedEnvironment)
    {
        // TODO: Ensure the action format matches what AssociatedEnvironment expects.
        // This is a direct application. Consider if any transformation or buffering is needed.
        // AssociatedEnvironment->StepAction(Action);
        UERL_URL_LOG("Agent component (%s): Action would be applied to AssociatedEnvironment if StepAction was called here.", *AgentId.ToString());
    }
    else
    {
        UERL_URL_WARNING("Agent component (%s): Received action but no AssociatedEnvironment to apply it to.", *AgentId.ToString());
    }
    // TODO: Expose this via a BlueprintAssignable delegate so Blueprints can react.
    // OnActionReceived.Broadcast(Action);
}