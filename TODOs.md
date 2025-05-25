# TODOs: Unreal Engine RLtools Plugin

This document outlines the tasks required to create an Unreal Engine plugin that integrates the `rl_tools` C++ library and exposes its functionality to Blueprints for reinforcement learning workflows.

## Phase 1: RLtools Integration and Basic Setup

-   [X] **1.1. Create Unreal Engine Plugin (`UERLTools`)**
    -   [X] 1.1.1. Use the UE Editor to create a new C++ "Third Party Plugin" or a blank C++ plugin.
    -   [X] 1.1.2. Define plugin structure (Source, Resources, etc.).
-   [X] **1.2. Integrate `rl_tools` Library**
    -   [X] 1.2.1. Clone/copy `rl_tools` repository into `UERLTools/Source/ThirdParty/rl_tools/`. (Path corrected)
    -   [X] 1.2.2. Ensure only necessary components (primarily the `include` directory from `rl_tools/src` for the header-only core) are included to keep the plugin lean.
    -   [X] 1.2.3. Configure `UERLTools.Build.cs`:
        -   [X] Add `rl_tools/include` to `PublicIncludePaths`. (Path corrected from `rl_tools_lib/include`)
        -   [X] Set `CppStandard = CppStandardVersion.Cpp17`.
        -   [X] Add any necessary `PublicDefinitions` if required by `rl_tools`. (Assumed done/not needed if not specified as missing)
        -   [X] Handle potential compiler warnings (e.g., consider `bEnableUndefinedIdentifierWarnings = false;` cautiously, or use `THIRD_PARTY_INCLUDES_START/END`). (Confirmed done)
-   [X] **1.3. Initial Compilation and Verification**
    -   [X] 1.3.1. Create a test C++ class within the plugin (`RLToolsTest`).
    -   [X] 1.3.2. Include core `rl_tools` headers (e.g., `<rl_tools/operations/cpu_mux.h>`, `<rl_tools/devices/cpu.h>`).
    -   [X] 1.3.3. Instantiate basic `rl_tools` types (e.g., `rlt::devices::DefaultCPU`).
    -   [X] 1.3.4. Compile the UE project to ensure `rl_tools` is recognized and builds correctly.
    -   [X] 1.3.5. Address any compilation errors or warnings, potentially by wrapping includes with `THIRD_PARTY_INCLUDES_START`/`THIRD_PARTY_INCLUDES_END` and specific warning pragmas. (Covered by 1.2.3)

## Phase 2: C++ Abstraction Layer for RLtools in UE

-   [ ] **2.1. Define Core RL Structures and Types**
    -   [P] 2.1.1. Create C++ wrappers or adaptors for `rl_tools` concepts using UE-friendly types where appropriate. (`FRLEnvironmentConfig`, `FRLTrainingConfig` USTRUCTs exist; `URLAgentManager`, `URLEnvironmentComponent` UCLASSES exist. P = Partially done)
    -   [ ] 2.1.2. Define `RLTOOLS_NAMESPACE_WRAPPER` as suggested in `doc.md` if needed. (**Note:** Currently not used. Decide if this is still desired or can be removed.)
    -   [ ] 2.1.3. Establish conventions for data exchange between UE and `rl_tools`. (**Critical - Not Done** - See `NEEDFIX.md`)
        -   [ ] 2.1.3.1. Implement C++ function in `URLAgentManager` to convert `TArray<float>` (UE observation/action) to `rl_tools::Matrix<...>`. 
        -   [ ] 2.1.3.2. Implement C++ function in `URLAgentManager` to convert `rl_tools::Matrix<...>` (rl_tools action/policy output) to `TArray<float>`.
-   [ ] **2.2. Implement Custom UE Environment for `rl_tools`**
    -   [X] 2.2.1. Design a base `UActorComponent` (`URLEnvironmentComponent`) to act as the bridge.
    -   [ ] 2.2.2. Implement the `rl_tools` custom environment C++ API via an adapter class. (**Critical - Not Done** - See `NEEDFIX.md`)
        -   [ ] 2.2.2.1. Design an adapter struct/class (e.g., `FRLEnvironmentAdapter`) that takes a `URLEnvironmentComponent*`.
        -   [ ] 2.2.2.2. This adapter must implement the `rl_tools` environment concept (e.g., by providing `rlt::ENVIRONMENT_SPEC`, `rlt::ENVIRONMENT` typedefs or template specializations).
        -   [ ] 2.2.2.3. Implement `static void rlt::malloc(DEVICE, ENVIRONMENT&)` and `static void rlt::free(DEVICE, ENVIRONMENT&)` for the adapter (likely NOPs or manage adapter-specific state if any).
        -   [ ] 2.2.2.4. Implement `static void rlt::init(DEVICE, ENVIRONMENT&, typename ENVIRONMENT::Parameters&)` for the adapter.
        -   [ ] 2.2.2.5. Implement `static void rlt::initial_state(DEVICE, ENVIRONMENT&, typename ENVIRONMENT::State&, RNG&)`: Calls `URLEnvironmentComponent::Reset()` and populates the observation in `State`.
        -   [ ] 2.2.2.6. Implement `static void rlt::step(DEVICE, ENVIRONMENT&, typename ENVIRONMENT::State&, ACTION_TYPE&, typename ENVIRONMENT::State&, RNG&)`: Calls `URLEnvironmentComponent::Step(Action)` and updates `State`.
        -   [ ] 2.2.2.7. Implement `static void rlt::observe(DEVICE, ENVIRONMENT&, typename ENVIRONMENT::State&, OBSERVATION_TYPE&, RNG&)`: Gets current observation from `URLEnvironmentComponent`.
        -   [ ] 2.2.2.8. Implement `static typename ENVIRONMENT::T rlt::reward(DEVICE, ENVIRONMENT&, typename ENVIRONMENT::State&, ACTION_TYPE&, typename ENVIRONMENT::State&, RNG&)`: Gets current reward from `URLEnvironmentComponent`.
        -   [ ] 2.2.2.9. Implement `static bool rlt::terminated(DEVICE, ENVIRONMENT&, typename ENVIRONMENT::State&, RNG&)`: Gets termination status from `URLEnvironmentComponent`.
        -   [ ] 2.2.2.10. Ensure `URLEnvironmentComponent` has corresponding Blueprint-implementable or C++ functions to provide the necessary data (e.g., `GetCurrentObservation()`, `GetCurrentReward()`, `IsTerminated()`).
    -   [X] 2.2.3. Ensure these functions can be overridden or configured by users for specific game environments (via `URLEnvironmentComponent`'s Blueprint events and virtual functions).
-   [ ] **2.3. Implement Agent Manager/Wrapper**
    -   [ ] 2.3.1. Refactor/Create `URLAgentManager` as an Unreal Engine Subsystem (e.g., inheriting from `UGameInstanceSubsystem` or `UWorldSubsystem`) to manage the RL agent's lifecycle, policy, and training/inference process. (**Decision:** Changed from UObject to Subsystem).
    -   [ ] 2.3.2. This class will encapsulate `rl_tools` algorithm setup (e.g., TD3). (**Critical - Placeholder C++ Implementation** - See `NEEDFIX.md`)
        -   [ ] 2.3.2.1. Define `DEVICE` and `Parameters` (e.g., `rlt::rl::algorithms::td3::ActorCritic<Parameters<...>>`).
        -   [ ] 2.3.2.2. Implement memory allocation (`rlt::malloc(device, agent_struct)`) for all `rl_tools` components (Actor, Critic, Target Actor, Target Critic, Optimizers, Replay Buffer, OffPolicyRunner, main algorithm struct) in `URLAgentManager::InitializeAgent`.
        -   [ ] 2.3.2.3. Implement initialization (`rlt::init(device, component, params)`) for all these components.
        -   [ ] 2.3.2.4. Implement memory deallocation (`rlt::free(device, agent_struct)`) in `URLAgentManager::BeginDestroy` or a dedicated cleanup function.
    -   [ ] 2.3.3. Implement functions for initializing the agent, policy, and optimizer. (Covered by 2.3.2, C++ implementations are placeholders).

## Phase 3: Blueprint Exposure Layer

-   [ ] **3.1. Create Blueprint Function Libraries**
    -   [ ] 3.1.1. Develop `UBlueprintFunctionLibrary` classes for static utility functions related to RL. (**Note:** `RLBlueprintFunctionLibrary.h` exists, check implementation status.)
    -   [ ] 3.1.2. Functions for creating/configuring environments, agents, or parameters from Blueprints.
-   [ ] **3.2. Design UObject Wrappers for RL Concepts**
    -   [P] 3.2.1. Create `UObject` subclasses to represent key `rl_tools` entities in Blueprints (e.g., `URLPolicy`, `URLReplayBuffer`, `URLTrainingConfig`). (**Note:** `FRLEnvironmentConfig` and `FRLTrainingConfig` are USTRUCTs. Decide if UObject wrappers are still needed or if USTRUCTs are sufficient. P = Partially done with USTRUCTs.)
    -   [X] 3.2.2. Expose properties of these UObjects/USTRUCTs using `UPROPERTY()` for Blueprint access. (Done for existing USTRUCTs).
-   [ ] **3.3. Implement Blueprint-Callable Functions (`UFUNCTION(BlueprintCallable)`)**
    -   [X] 3.3.1. In `URLEnvironmentComponent`:
        -   [X] Functions to get observation/action space dimensions.
        -   [X] Functions to manually trigger environment reset, step (for debugging/custom loops).
    -   [ ] 3.3.2. In `URLAgentManager` (or similar): (**Note:** Declarations exist, but C++ implementations are mostly placeholders - See `NEEDFIX.md`)
        -   [ ] **Initialization**:
            -   [ ] `InitializeAgent(EnvironmentComponent, TrainingConfig)` (C++ Placeholder)
            -   [ ] `LoadPolicy(FString FilePath)` (C++ Placeholder)
        -   [ ] **Training**:
            -   [ ] `StartTraining()` / `ResumeTraining()` (C++ Placeholder)
            -   [ ] `PauseTraining()` (C++ Placeholder)
            -   [ ] `StepTraining(int NumSteps)` (C++ Placeholder)
            -   [ ] `SavePolicy(FString FilePath)` (C++ Placeholder)
        -   [ ] **Inference**:
            -   [ ] `GetAction(TArray<float> Observation)` returning `TArray<float> Action` (C++ Placeholder - returns random)
        -   [ ] **Status & Logging**:
            -   [ ] `GetTrainingStatus()` (e.g., current step, average reward) (C++ Placeholder)
            -   [ ] `GetEpisodeStats()` (C++ Placeholder)
    -   [X] 3.3.3. Ensure data types are Blueprint-friendly (e.g., `TArray<float>`, `FString`, custom `USTRUCTS` for configs). (Declarations use BP-friendly types).
-   [ ] **3.4. Handle Asynchronous Operations**
    -   [ ] 3.4.1. For long-running tasks like `StartTraining()`, implement them asynchronously (e.g., using `FAsyncTask`, `FTSTicker`, or custom threading) to avoid blocking the game thread. (**Not Done** - See `NEEDFIX.md`)
    -   [ ] 3.4.2. Provide Blueprint events (`UPROPERTY(BlueprintAssignable)`) for completion or progress updates (e.g., `OnTrainingStepCompleted`, `OnTrainingFinished`). (**Not Done**)
-   [ ] **3.5. Data Type Conversions**
    -   [ ] 3.5.1. Implement robust conversion utilities between UE types (e.g., `FVector`, `FRotator`, `TArray`) and `rl_tools` matrix/tensor types. (**Note:** This is the same as 2.1.3. **Critical - Not Done**)
    -   [ ] 3.5.2. Handle normalization and denormalization of observation/action data if necessary, configurable from Blueprints. (**Not Done** - See `NEEDFIX.md`)
        -   [ ] 3.5.2.1. Add `bNormalizeObservations`, `ObservationMean`, `ObservationStd`, `bNormalizeActions`, `ActionMean`, `ActionStd` (or similar) to `FRLTrainingConfig` or `FRLEnvironmentConfig`.
        -   [ ] 3.5.2.2. Implement normalization logic in the data conversion step from UE to `rl_tools` (e.g., after 2.1.3.1).
        -   [ ] 3.5.2.3. Implement denormalization logic in the data conversion step from `rl_tools` to UE (e.g., before 2.1.3.2).

## Phase 4: Training Workflow Implementation

-   [ ] **4.1. Training Loop Management**
    -   [ ] 4.1.1. Implement the main training loop within `URLAgentManager` (or a dedicated training class) using `rl_tools`' "Loop Interface" (`rlt::LoopState`, `rlt::step`) or direct calls to `collect`, `train_actor_critic`, etc.
    -   [ ] 4.1.2. Integrate with the `URLEnvironmentComponent` for environment interactions.
    -   [ ] 4.1.3. Allow configuration of training parameters (batch size, learning rates, discount factor, algorithm-specific hyperparameters) from Blueprints via `URLTrainingConfig` UObject or UStructs.
-   [ ] **4.2. Replay Buffer Management (for Off-Policy)**
    -   [ ] 4.2.1. Expose replay buffer capacity and configuration.
    -   [ ] 4.2.2. Potentially visualize replay buffer status (size, etc.).
-   [ ] **4.3. Checkpointing and Model Saving/Loading**
    -   [ ] 4.3.1. Implement functionality to save trained policies and optimizers (e.g., to HDF5 if `rl_tools` HDF5 support is compiled, or to its `.h` format).
    -   [ ] 4.3.2. Implement loading of pre-trained policies for continued training or inference.
    -   [ ] 4.3.3. Expose these via Blueprint functions in `URLAgentManager`.
-   [ ] **4.4. Logging and Visualization Hooks**
    -   [ ] 4.4.1. Log training progress (rewards, losses, episode lengths) to UE's output log and potentially to CSV files.
    -   [ ] 4.4.2. Explore integration with `rl_tools`' TensorBoard logging if feasible and desired (might require compiling `rl_tools` with TensorBoard support).
    -   [ ] 4.4.3. Provide Blueprint events or functions to retrieve key metrics for display in UMG.

## Phase 5: Inference Workflow Implementation

-   [ ] **5.1. Policy Loading for Inference**
    -   [ ] 5.1.1. Ensure `URLAgentManager` can load a pre-trained policy specifically for inference mode (e.g., no optimizers, replay buffers needed).
-   [ ] **5.2. Real-time Action Generation**
    -   [ ] 5.2.1. Implement `GetAction` in `URLAgentManager` to take an observation from UE (via `URLEnvironmentComponent::Observe`), pass it to the loaded `rl_tools` policy (`rlt::evaluate`), and return the action.
    -   [ ] 5.2.2. Ensure this is performant for real-time game loops.
-   [ ] **5.3. Integration with UE AI Systems**
    -   [ ] 5.3.1. Develop examples or guidance on how to use the Blueprint-exposed RL agent with UE Behavior Trees (e.g., custom BTTask "GetRLAction").
    -   [ ] 5.3.2. Show how to integrate with UE AI Perception system for providing observations.

## Phase 6: Advanced Features & Polish

-   [ ] **6.1. Multi-Agent Support (Future Consideration)**
    -   [ ] 6.1.1. Investigate `rl_tools` capabilities for multi-agent RL (e.g., Multi-Agent PPO mentioned in `rl_tools` README).
    -   [ ] 6.1.2. Plan for potential extensions to the plugin architecture to support multiple interacting agents.
-   [ ] **6.2. Support for Different `rl_tools` Devices (CPU/GPU)**
    -   [ ] 6.2.1. Initially focus on CPU (`rlt::devices::DefaultCPU`).
    -   [ ] 6.2.2. Investigate requirements for supporting `rl_tools` CUDA backend (`rlt::devices::CUDA`) if available and desired, including build system changes and UE integration complexities.
-   [ ] **6.3. Parameter Tuning Utilities**
    -   [ ] 6.3.1. Allow dynamic adjustment of some training parameters from Blueprints or via console commands for easier tuning.
-   [ ] **6.4. Error Handling and Reporting**
    -   [ ] 6.4.1. Implement robust error checking and reporting to Blueprints and UE logs (e.g., for invalid configurations, file I/O errors, `rl_tools` exceptions).

## Phase 7: Documentation and Examples

-   [ ] **7.1. Plugin README**
    -   [ ] 7.1.1. Write a comprehensive `README.md` for the `UERLTools` plugin.
        -   [ ] Installation instructions.
        -   [ ] Overview of features and Blueprint API.
        -   [ ] Basic usage examples.
-   [ ] **7.2. Example Project/Map**
    -   [ ] 7.2.1. Create a simple example UE project or map demonstrating:
        -   [ ] Setting up an `URLEnvironmentComponent` for a simple task (e.g., a pawn learning to reach a target).
        -   [ ] Configuring and training an agent using Blueprints.
        -   [ ] Using a trained agent for inference in a simple scenario.
-   [ ] **7.3. Code Comments and API Documentation**
    -   [ ] 7.3.1. Ensure all C++ and Blueprint-exposed code is well-commented.
    -   [ ] 7.3.2. Generate API documentation if feasible.
