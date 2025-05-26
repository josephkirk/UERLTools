#pragma once

#include "CoreMinimal.h"

// Module-wide log categories
#include "UERLLog.h"

// #include "Path/To/URLEnvironmentComponent.h" // Include the actual header for URLEnvironmentComponent
#include "RLToolsConversionUtils.h"

// RL Tools core includes
#include "rl_tools/rl/environments/environments.h" // Basic environment spec
#include "rl_tools/rl/utils/validation.h"         // For rlt::utils::typing (Action/Observation wrappers)
#include "rl_tools/utils/generic/memcpy.h"        // For rl_tools::copy_view

// Forward declaration if URLEnvironmentComponent.h is not included above (definition needed for template instantiation)
class URLEnvironmentComponent; 

template <typename T_DEVICE, typename T_ENVIRONMENT_SPEC>
class UEEnvironmentAdapter {
public:
    using DEVICE = T_DEVICE;
    using ENVIRONMENT_SPEC = T_ENVIRONMENT_SPEC;
    using OBSERVATION_SPEC = typename ENVIRONMENT_SPEC::OBSERVATION_SPEC;
    using ACTION_SPEC = typename ENVIRONMENT_SPEC::ACTION_SPEC;
    using TI = typename DEVICE::index_t; // Index type
    using T = typename ENVIRONMENT_SPEC::T; // Scalar type (float/double)

    // State struct for the environment, typically holds the observation
    struct State {
        typename OBSERVATION_SPEC::template Matrix<typename OBSERVATION_SPEC::SPEC> observation;
        // TI episode_step_count; // Optional: if you want to track steps within the adapter's state
    };

    URLEnvironmentComponent* LinkedEnvComponent;
    FRLNormalizationParams ObservationNormParams;
    FRLNormalizationParams ActionNormParams;

    UEEnvironmentAdapter(URLEnvironmentComponent* EnvComponent, const FRLNormalizationParams& InObsParams, const FRLNormalizationParams& InActParams)
        : LinkedEnvComponent(EnvComponent), ObservationNormParams(InObsParams), ActionNormParams(InActParams) {
        check(LinkedEnvComponent != nullptr);
        // If OBSERVATION_SPEC or ACTION_SPEC define dynamically sized matrices,
        // their memory management is typically handled by rl_tools algorithms
        // when State objects or action matrices are created/destroyed.
    }

    // This adapter is intrinsically linked to a URLEnvironmentComponent instance.
    // It's not meant to be copied or moved in a way that detaches it.
    UEEnvironmentAdapter(const UEEnvironmentAdapter&) = delete;
    UEEnvironmentAdapter& operator=(const UEEnvironmentAdapter&) = delete;
    UEEnvironmentAdapter(UEEnvironmentAdapter&&) = delete;
    UEEnvironmentAdapter& operator=(UEEnvironmentAdapter&&) = delete;
};

// Implementation of the rl_tools environment API static functions.
// These functions define the behavior of the environment for rl_tools algorithms.
// T_ENVIRONMENT in these functions will be UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>.
namespace rl_tools {

    template <typename T_DEVICE, typename T_ENVIRONMENT_SPEC>
    static void init(T_DEVICE& device, UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>& env) {
        if (env.LinkedEnvComponent) {
            // Assuming URLEnvironmentComponent has a ResetEnvironment method
            // to set it to a known initial configuration.
            env.LinkedEnvComponent->ResetEnvironment(); 
        } else {
            UERL_RL_ERROR( TEXT("UEEnvironmentAdapter::init - LinkedEnvComponent is null."));
        }
    }

    template <typename T_DEVICE, typename T_ENVIRONMENT_SPEC>
    static void initial_state(T_DEVICE& device, UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>& env, typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::State& state) {
        if (!env.LinkedEnvComponent) {
            UERL_RL_ERROR( TEXT("UEEnvironmentAdapter::initial_state - LinkedEnvComponent is null."));
            return;
        }

        env.LinkedEnvComponent->ResetEnvironment(); // Ensure it's the true initial state

        // Assuming GetCurrentObservation() returns the observation after reset.
        TArray<float> UEObservation = env.LinkedEnvComponent->GetCurrentObservation(); 

        // The state.observation matrix should be allocated by the caller (rl_tools framework)
        // or be fixed-size. We just fill it.
        bool bSuccess = RLToolsConversionUtils::UEArrayToRLMatrix(UEObservation, state.observation, env.ObservationNormParams);
        if (!bSuccess) {
            UERL_RL_ERROR( TEXT("UEEnvironmentAdapter::initial_state - Failed to convert UE observation to RL_Tools matrix."));
            // Consider how to handle error: e.g., fill state.observation with zeros or a specific error pattern.
        }
        // state.episode_step_count = 0; // If tracking steps
    }

    template <typename T_DEVICE, typename T_ENVIRONMENT_SPEC>
    static void step(
        T_DEVICE& device,
        UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>& env,
        const typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::State& current_rl_state, // rl_tools current state
        const typename rl_tools::rl::utils::typing::Action<typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::ACTION_SPEC>::Type& rl_action, // rl_tools action matrix
        typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::State& next_rl_state // rl_tools next state to be populated
    ) {
        if (!env.LinkedEnvComponent) {
            UERL_RL_ERROR( TEXT("UEEnvironmentAdapter::step - LinkedEnvComponent is null."));
            return;
        }

        TArray<float> UEAction;
        bool bConversionSuccess = RLToolsConversionUtils::RLMatrixToUEArray(rl_action, UEAction, env.ActionNormParams);
        if (!bConversionSuccess) {
            UERL_RL_ERROR( TEXT("UEEnvironmentAdapter::step - Failed to convert RL_Tools action to UEArray."));
            // Handle error: e.g., take no action or a default safe action.
            // For now, just returning. The environment won't advance.
            // A robust implementation might set next_rl_state to reflect an error or terminal state.
            return;
        }

        // URLEnvironmentComponent::StepAction should update its internal state,
        // making new observation, reward, and done status available via its getters.
        env.LinkedEnvComponent->StepAction(UEAction);

        TArray<float> UENextObservation = env.LinkedEnvComponent->GetCurrentObservation();
        
        bConversionSuccess = RLToolsConversionUtils::UEArrayToRLMatrix(UENextObservation, next_rl_state.observation, env.ObservationNormParams);
        if (!bConversionSuccess) {
            UERL_RL_ERROR( TEXT("UEEnvironmentAdapter::step - Failed to convert next UE observation to RL_Tools matrix."));
            // Handle error for next_rl_state.observation
        }
        // next_rl_state.episode_step_count = current_rl_state.episode_step_count + 1; // If tracking steps
    }

    template <typename T_DEVICE, typename T_ENVIRONMENT_SPEC>
    static void observe(
        T_DEVICE& device,
        UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>& env,
        const typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::State& rl_state, // The current rl_tools state
        typename rl_tools::rl::utils::typing::Observation<typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::OBSERVATION_SPEC>::Type& rl_observation // The output rl_tools observation matrix
    ) {
        // This function copies the observation from rl_state.observation to the rl_observation output parameter.
        // This is a common pattern if the State struct already contains the fully formed observation matrix.
        // The rl_observation matrix is assumed to be allocated by the caller.
        rl_tools::copy_view(device, device, rl_observation, rl_state.observation);
    }

    template <typename T_DEVICE, typename T_ENVIRONMENT_SPEC>
    static typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::T reward( // Return type is scalar T (float/double)
        T_DEVICE& device,
        UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>& env,
        const typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::State& current_rl_state,
        const typename rl_tools::rl::utils::typing::Action<typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::ACTION_SPEC>::Type& rl_action,
        const typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::State& next_rl_state
    ) {
        if (!env.LinkedEnvComponent) {
            UERL_RL_ERROR( TEXT("UEEnvironmentAdapter::reward - LinkedEnvComponent is null."));
            return static_cast<typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::T>(0.0f);
        }
        // Assumes GetCurrentReward() returns the reward resulting from the action that led to next_rl_state.
        return static_cast<typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::T>(env.LinkedEnvComponent->GetCurrentReward());
    }

    template <typename T_DEVICE, typename T_ENVIRONMENT_SPEC>
    static bool terminated(
        T_DEVICE& device,
        UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>& env,
        const typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::State& current_rl_state,
        const typename rl_tools::rl::utils::typing::Action<typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::ACTION_SPEC>::Type& rl_action,
        const typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::State& next_rl_state
    ) {
        if (!env.LinkedEnvComponent) {
            UERL_RL_ERROR( TEXT("UEEnvironmentAdapter::terminated - LinkedEnvComponent is null."));
            return true; // Default to terminated on error to prevent potential infinite loops.
        }
        // Assumes IsDone() returns the termination status resulting from the action that led to next_rl_state.
        bool bIsDone = env.LinkedEnvComponent->IsDone();
        
        // Optional: Check max episode steps if tracked by the adapter or component
        // if (env.LinkedEnvComponent->HasMaxEpisodeSteps() && next_rl_state.episode_step_count >= env.LinkedEnvComponent->GetMaxEpisodeSteps()){
        //     bIsDone = true;
        // }
        return bIsDone;
    }
    
    // Optional: Define max_episode_steps if your environment has a fixed limit known to rl_tools
    template <typename T_DEVICE, typename T_ENVIRONMENT_SPEC>
    static typename UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>::TI max_episode_steps(
        const UEEnvironmentAdapter<T_DEVICE, T_ENVIRONMENT_SPEC>& env
    ) {
        if (env.LinkedEnvComponent && env.LinkedEnvComponent->HasMaxEpisodeSteps()) { // Assuming HasMaxEpisodeSteps guards GetMaxEpisodeSteps
            return env.LinkedEnvComponent->GetMaxEpisodeSteps();
        }
        // Return a default or a very large number if not specifically defined by the component.
        // For PPO, this is often used for episode truncation rather than termination.
        return 1000; // Example default
    }

} // namespace rl_tools


// RL Tools includes - adjust as necessary for specific types
#include "rl_tools/rl/environments/environments.h" // For base environment concepts if any
#include "rl_tools/utils/generic/typing.h" // For rlt::utils::typing::ActionType
#include "rl_tools/operations/cpu.h" // Or operations_cpu_mux.h, depending on what's needed

// Forward declaration
// template <typename T_DEVICE, typename T_SPEC> struct UEEnvironmentAdapter;

/**
 * UEEnvironmentAdapter
 *
 * Bridges a URLEnvironmentComponent (or a similar UE-based environment representation)
 * with the C++ API expected by rl_tools.
 *
 * This adapter is designed to be used by rl_tools algorithms. It translates calls from
 * rl_tools (like step, observe) into calls on the URLEnvironmentComponent and converts
 * data between UE types and rl_tools tensor types.
 *
 * @tparam T_DEVICE The rl_tools device (e.g., rlt::devices::DefaultCPU).
 * @tparam T_SPEC The rl_tools environment specification, defining observation/action spaces.
 *                This SPEC needs to be compatible with the adapted URLEnvironmentComponent.
 */
template <typename T_DEVICE, typename T_SPEC>
struct UEEnvironmentAdapter
{
    using DEVICE = T_DEVICE;
    using SPEC = T_SPEC;
    using TI = typename DEVICE::index_t;
    using T = typename SPEC::T;

    // rl_tools Environment API requirements
    using State = typename SPEC::State; // Define what a "State" means for rl_tools in this context
    using Parameters = typename SPEC::Parameters; // Environment parameters

    // Store a pointer to the Unreal Engine environment component this adapter wraps.
    // This component must outlive the adapter or be managed carefully.
    URLEnvironmentComponent* UEEnvComponent = nullptr;

    // Store parameters if your environment spec needs them
    // Parameters parameters;


    // Constructor to link with a URLEnvironmentComponent
    UEEnvironmentAdapter(URLEnvironmentComponent* InUEEnvComponent)
        : UEEnvComponent(InUEEnvComponent)
    {
        check(UEEnvComponent != nullptr);
        // TODO: Potentially initialize 'parameters' from InUEEnvComponent or SPEC
    }

    // --- rl_tools Environment API Implementation ---
    // Note: These are often static in rl_tools examples if the environment itself is stateless
    // or if state is passed around. Here, we might need them to be non-static if they
    // depend on the UEEnvComponent instance, or we pass 'this' adapter as context.
    // For simplicity with rl_tools, let's assume for now that the 'environment' passed
    // to rl_tools functions will be an instance of this adapter.

    static_assert(SPEC::OBSERVATION_DIM > 0, "SPEC::OBSERVATION_DIM must be greater than 0.");
    static_assert(SPEC::ACTION_DIM > 0, "SPEC::ACTION_DIM must be greater than 0.");

    // It's common for rl_tools environments to have these as static functions,
    // taking the environment object (this adapter) and its parameters as arguments.
};

// --- Static implementations of rl_tools environment functions ---
// These functions will operate on an instance of UEEnvironmentAdapter

namespace rl_tools {
    // Initialize the environment (adapter instance)
    template <typename DEVICE, typename SPEC>
    void init(DEVICE& device, UEEnvironmentAdapter<DEVICE, SPEC>& env, typename UEEnvironmentAdapter<DEVICE, SPEC>::Parameters& params) {
        // env.parameters = params; // If parameters are part of the adapter
        // Any other one-time setup for the adapter based on parameters.
        // UERL_RL_LOG( TEXT("UEEnvironmentAdapter initialized via rl_tools::init"));
    }

    // Get the initial state of the environment
    template <typename DEVICE, typename SPEC, typename RNG>
    void initial_state(DEVICE& device, const UEEnvironmentAdapter<DEVICE, SPEC>& env, typename UEEnvironmentAdapter<DEVICE, SPEC>::State& state, RNG& rng) {
        check(env.UEEnvComponent != nullptr);
        // 1. Call Reset on the UEEnvComponent
        env.UEEnvComponent->ResetEnvironment();
        // 2. Get observation from UEEnvComponent
        TArray<float> InitialObservationData = env.UEEnvComponent->GetObservation();
        // 3. Convert TArray<float> to rl_tools State type (e.g., Matrix)
        //    This requires knowing the structure of SPEC::State.
        //    Assuming SPEC::State is a Matrix<T, 1, SPEC::OBSERVATION_DIM> for now.
        //    This conversion logic will be part of Task 2.1.3.
        if (InitialObservationData.Num() != SPEC::OBSERVATION_DIM) {
            UERL_RL_ERROR( TEXT("InitialObservationData.Num() [%d] != SPEC::OBSERVATION_DIM [%d]"), InitialObservationData.Num(), SPEC::OBSERVATION_DIM);
            return;
        }
        // Example: rlt::Matrix<T, 1, SPEC::OBSERVATION_DIM>* obs_matrix = static_cast<rlt::Matrix<T, 1, SPEC::OBSERVATION_DIM>*>(&state);
        // for (typename DEVICE::index_t i = 0; i < SPEC::OBSERVATION_DIM; ++i) {
        //     obs_matrix->data[i] = static_cast<T>(InitialObservationData[i]);
        // }
        UERL_RL_WARNING( TEXT("initial_state: Data conversion from TArray to rl_tools State not fully implemented."));
    }

    // Take a step in the environment
    template <typename DEVICE, typename SPEC, typename ACTION_SPEC, typename RNG>
    void step(DEVICE& device, const UEEnvironmentAdapter<DEVICE, SPEC>& env, const typename UEEnvironmentAdapter<DEVICE, SPEC>::State& current_state, const rl_tools::Matrix<ACTION_SPEC>& action, typename UEEnvironmentAdapter<DEVICE, SPEC>::State& next_state, RNG& rng) {
        check(env.UEEnvComponent != nullptr);
        // 1. Convert rl_tools action (Matrix) to TArray<float>
        //    This conversion logic will be part of Task 2.1.3.
        TArray<float> UEAction;
        // Example: for (typename DEVICE::index_t i = 0; i < SPEC::ACTION_DIM; ++i) {
        //     UEAction.Add(static_cast<float>(action.data[i]));
        // }
        UERL_RL_WARNING( TEXT("step: Action conversion from rl_tools Matrix to TArray not fully implemented."));

        // 2. Apply action to UEEnvComponent
        if (UEAction.Num() == SPEC::ACTION_DIM) { // Basic check
             env.UEEnvComponent->ApplyAction(UEAction); // ApplyAction should internally update the UE environment's state
        } else {
            UERL_RL_ERROR( TEXT("Step: Action dimension mismatch. Expected %d, Got %d"), SPEC::ACTION_DIM, UEAction.Num());
        }


        // 3. Get next observation from UEEnvComponent
        TArray<float> NextObservationData = env.UEEnvComponent->GetObservation();
        // 4. Convert TArray<float> to rl_tools State type (next_state)
        //    Similar to initial_state conversion.
        if (NextObservationData.Num() != SPEC::OBSERVATION_DIM) {
            UERL_RL_ERROR( TEXT("NextObservationData.Num() [%d] != SPEC::OBSERVATION_DIM [%d]"), NextObservationData.Num(), SPEC::OBSERVATION_DIM);
            return;
        }
        // Example: rlt::Matrix<T, 1, SPEC::OBSERVATION_DIM>* next_obs_matrix = static_cast<rlt::Matrix<T, 1, SPEC::OBSERVATION_DIM>*>(&next_state);
        // for (typename DEVICE::index_t i = 0; i < SPEC::OBSERVATION_DIM; ++i) {
        //     next_obs_matrix->data[i] = static_cast<T>(NextObservationData[i]);
        // }
        UERL_RL_WARNING( TEXT("step: Next observation conversion from TArray to rl_tools State not fully implemented."));
    }

    // Observe the current state (might be redundant if step returns next_state directly)
    // Often, observe is used to get the current observation without taking a step.
    template <typename DEVICE, typename SPEC>
    void observe(DEVICE& device, const UEEnvironmentAdapter<DEVICE, SPEC>& env, const typename UEEnvironmentAdapter<DEVICE, SPEC>::State& current_internal_state, typename SPEC::Observation& observation_matrix) {
        check(env.UEEnvComponent != nullptr);
        // 1. Get current observation from UEEnvComponent
        TArray<float> CurrentObservationData = env.UEEnvComponent->GetObservation();
        // 2. Convert TArray<float> to rl_tools Observation type (Matrix)
        if (CurrentObservationData.Num() != SPEC::OBSERVATION_DIM) {
            UERL_RL_ERROR( TEXT("Observe: CurrentObservationData.Num() [%d] != SPEC::OBSERVATION_DIM [%d]"), CurrentObservationData.Num(), SPEC::OBSERVATION_DIM);
            return;
        }
        // Example: for (typename DEVICE::index_t i = 0; i < SPEC::OBSERVATION_DIM; ++i) {
        //     observation_matrix.data[i] = static_cast<T>(CurrentObservationData[i]);
        // }
        UERL_RL_WARNING( TEXT("observe: Data conversion from TArray to rl_tools Observation not fully implemented."));
    }


    // Get the reward for the last transition
    template <typename DEVICE, typename SPEC>
    typename UEEnvironmentAdapter<DEVICE, SPEC>::T reward(DEVICE& device, const UEEnvironmentAdapter<DEVICE, SPEC>& env, const typename UEEnvironmentAdapter<DEVICE, SPEC>::State& current_state, const rl_tools::utils::typing::ActionType<typename SPEC::ACTION_SPEC>::Type& action, const typename UEEnvironmentAdapter<DEVICE, SPEC>::State& next_state) {
        check(env.UEEnvComponent != nullptr);
        return static_cast<typename UEEnvironmentAdapter<DEVICE, SPEC>::T>(env.UEEnvComponent->GetReward());
    }

    // Check if the episode is terminated
    template <typename DEVICE, typename SPEC, typename RNG>
    bool terminated(DEVICE& device, const UEEnvironmentAdapter<DEVICE, SPEC>& env, const typename UEEnvironmentAdapter<DEVICE, SPEC>::State& state, RNG& rng) {
        check(env.UEEnvComponent != nullptr);
        bool bIsDone = false;
        bool bIsTruncated = false; // Assuming URLEnvironmentComponent might provide this
        env.UEEnvComponent->IsDone(bIsDone, bIsTruncated);
        return bIsDone || bIsTruncated; // rl_tools 'terminated' usually means episode end for any reason
    }
} // namespace rl_tools
