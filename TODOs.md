# TODOs: Unreal Engine RLtools Plugin

This document outlines the tasks required to create an Unreal Engine plugin that integrates the `rl_tools` C++ library and exposes its functionality to Blueprints for reinforcement learning workflows.

## Phase 1: RLtools Integration and Basic Setup

-   [x] **1.1. Create Unreal Engine Plugin (`UERLTools`)**
    -   [x] 1.1.1. Use the UE Editor to create a new C++ "Third Party Plugin" or a blank C++ plugin.
    -   [x] 1.1.2. Define plugin structure (Source, Resources, etc.).
-   [ ] **1.2. Integrate `rl_tools` Library**
    -   [x] 1.2.1. Clone/copy `rl_tools` repository into `UERLTools/Source/ThirdParty/rl_tools_lib/`.
    -   [ ] 1.2.2. Ensure only necessary components (primarily the `include` directory from `rl_tools/src` for the header-only core) are included to keep the plugin lean.
    -   [ ] 1.2.3. Configure `UERLTools.Build.cs`:
        -   [ ] Add `rl_tools_lib/include` to `PublicIncludePaths` or `PrivateIncludePaths`.
        -   [ ] Set `CppStandard = CppStandardVersion.Cpp17`.
        -   [ ] Add any necessary `PublicDefinitions` if required by `rl_tools`.
        -   [ ] Handle potential compiler warnings (e.g., consider `bEnableUndefinedIdentifierWarnings = false;` cautiously, or use `THIRD_PARTY_INCLUDES_START/END`).
-   [ ] **1.3. Initial Compilation and Verification**
    -   [ ] 1.3.1. Create a test C++ class within the plugin.
    -   [ ] 1.3.2. Include core `rl_tools` headers (e.g., `<rl_tools/operations/cpu_mux.h>`, `<rl_tools/devices/cpu.h>`).
    -   [ ] 1.3.3. Instantiate basic `rl_tools` types (e.g., `rlt::devices::DefaultCPU`).
    -   [ ] 1.3.4. Compile the UE project to ensure `rl_tools` is recognized and builds correctly.
    -   [ ] 1.3.5. Address any compilation errors or warnings, potentially by wrapping includes with `THIRD_PARTY_INCLUDES_START`/`THIRD_PARTY_INCLUDES_END` and specific warning pragmas.

## Phase 2: C++ Abstraction Layer for RLtools in UE

-   [ ] **2.1. Define Core RL Structures and Types**
    -   [ ] 2.1.1. Create C++ wrappers or adaptors for `rl_tools` concepts using UE-friendly types where appropriate.
    -   [ ] 2.1.2. Define `RLTOOLS_NAMESPACE_WRAPPER` as suggested in `doc.md` if needed.
    -   [ ] 2.1.3. Establish conventions for data exchange between UE and `rl_tools` (e.g., using `TArray` for observations/actions and converting to/from `rl_tools` matrix types).
-   [ ] **2.2. Implement Custom UE Environment for `rl_tools`**
    -   [ ] 2.2.1. Design a base `UActorComponent` (e.g., `URLEnvironmentComponent`) to act as the bridge.
    -   [ ] 2.2.2. Implement the `rl_tools` custom environment API within this component:
        -   [ ] Define `ENVIRONMENT_SPEC` (Observation/Action dimensions, types T, TI).
        -   [ ] Define `ENVIRONMENT` struct to hold UE-specific state.
        -   [ ] Implement `rlt::malloc`, `rlt::free` (likely NOPs).
        -   [ ] Implement `rlt::init`.
        -   [ ] Implement `rlt::initial_state` (reset UE state, provide initial observation).
        -   [ ] Implement `rlt::step` (apply action in UE, simulate, get next state).
        -   [ ] Implement `rlt::observe` (collect observation from UE state).
        -   [ ] Implement `rlt::reward` (calculate reward based on UE state transition).
        -   [ ] Implement `rlt::terminated` (check termination conditions in UE).
    -   [ ] 2.2.3. Ensure these functions can be overridden or configured by users for specific game environments.
-   [ ] **2.3. Implement Agent Manager/Wrapper**
    -   [ ] 2.3.1. Create a C++ class (e.g., `URLAgentManager` as a `UObject` or `AActor`) to manage the RL agent's lifecycle, policy, and training/inference process.
    -   [ ] 2.3.2. This class will encapsulate `rl_tools` algorithm setup (e.g., TD3, PPO, SAC).
        -   [ ] Actor-Critic network creation (`rlt::nn_models::mlp::NeuralNetwork`).
        -   [ ] Algorithm-specific parameters and components (e.g., `rlt::rl::algorithms::td3::ActorCritic`, `rlt::rl::components::OffPolicyRunner`, `rlt::rl::components::replay_buffer::ReplayBuffer`).
    -   [ ] 2.3.3. Implement functions for initializing the agent, policy, and optimizer.

## Phase 3: Blueprint Exposure Layer

-   [ ] **3.1. Create Blueprint Function Libraries**
    -   [ ] 3.1.1. Develop `UBlueprintFunctionLibrary` classes for static utility functions related to RL.
    -   [ ] 3.1.2. Functions for creating/configuring environments, agents, or parameters from Blueprints.
-   [ ] **3.2. Design UObject Wrappers for RL Concepts**
    -   [ ] 3.2.1. Create `UObject` subclasses to represent key `rl_tools` entities in Blueprints (e.g., `URLPolicy`, `URLReplayBuffer`, `URLTrainingConfig`).
    -   [ ] 3.2.2. Expose properties of these UObjects using `UPROPERTY()` for Blueprint access.
-   [ ] **3.3. Implement Blueprint-Callable Functions (`UFUNCTION(BlueprintCallable)`)**
    -   [ ] 3.3.1. In `URLEnvironmentComponent`:
        -   [ ] Functions to get observation/action space dimensions.
        -   [ ] Functions to manually trigger environment reset, step (for debugging/custom loops).
    -   [ ] 3.3.2. In `URLAgentManager` (or similar):
        -   [ ] **Initialization**:
            -   `InitializeAgent(EnvironmentComponent, TrainingConfig)`
            -   `LoadPolicy(FString FilePath)`
        -   [ ] **Training**:
            -   `StartTraining()` / `ResumeTraining()`
            -   `PauseTraining()`
            -   `StepTraining(int NumSteps)` (for manual stepping from BP)
            -   `SavePolicy(FString FilePath)`
        -   [ ] **Inference**:
            -   `GetAction(TArray<float> Observation)` returning `TArray<float> Action`
        -   [ ] **Status & Logging**:
            -   `GetTrainingStatus()` (e.g., current step, average reward)
            -   `GetEpisodeStats()`
    -   [ ] 3.3.3. Ensure data types are Blueprint-friendly (e.g., `TArray<float>`, `FString`, custom `USTRUCTS` for configs).
-   [ ] **3.4. Handle Asynchronous Operations**
    -   [ ] 3.4.1. For long-running tasks like `StartTraining()`, implement them asynchronously (e.g., using `FAsyncTask`, `FTSTicker`, or custom threading) to avoid blocking the game thread.
    -   [ ] 3.4.2. Provide Blueprint events (`UPROPERTY(BlueprintAssignable)`) for completion or progress updates (e.g., `OnTrainingStepCompleted`, `OnTrainingFinished`).
-   [ ] **3.5. Data Type Conversions**
    -   [ ] 3.5.1. Implement robust conversion utilities between UE types (e.g., `FVector`, `FRotator`, `TArray`) and `rl_tools` matrix/tensor types.
    -   [ ] 3.5.2. Handle normalization and denormalization of observation/action data if necessary, configurable from Blueprints.

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
