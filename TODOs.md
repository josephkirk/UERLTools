# TODOs: Unreal Engine RLtools Plugin

This document outlines the tasks required to create an Unreal Engine plugin that integrates the `rl_tools` C++ library and exposes its functionality to Blueprints for reinforcement learning workflows.

## Phase 1: RLtools Integration and Basic Setup

-   [X] **1.1. Create Unreal Engine Plugin (`UERLTools`)**
    -   [X] 1.1.1. Use the UE Editor to create a new C++ "Third Party Plugin" or a blank C++ plugin.
    -   [X] 1.1.2. Define plugin structure (Source, Resources, etc.).
-   [X] **1.2. Integrate `rl_tools` Library**
    -   [X] 1.2.1. Clone/copy `rl_tools` repository into `UERLTools/Source/ThirdParty/rl_tools/`. (Path corrected)
    -   [X] 1.2.2. Ensure only necessary components (primarily the `include` directory from `rl_tools/src` for the header-only core) are included to keep the plugin lean.                                                                                                                         -   [X] 1.2.3. Configure `UERLTools.Build.cs`:
        -   [X] Add `rl_tools/include` to `PublicIncludePaths`. (Path corrected from `rl_tools_lib/include`)
        -   [X] Set `CppStandard = CppStandardVersion.Cpp17`.
        -   [X] Add any necessary `PublicDefinitions` if required by `rl_tools`. (Assumed done/not needed if not specified as missing)
        -   [X] Handle potential compiler warnings (e.g., consider `bEnableUndefinedIdentifierWarnings = false;` cautiously, or use `THIRD_PARTY_INCLUDES_START/END`). (Confirmed done)
-   [X] **1.3. Initial Compilation and Verification**
    -   [X] 1.3.1. Create a test C++ class within the plugin (`RLToolsTest`).
    -   [X] 1.3.2. Include core `rl_tools` headers (e.g., `<rl_tools/operatio  ns/cpu_mux.h>`, `<rl_tools/devices/cpu.h>`).
    -   [X] 1.3.3. Instantiate basic `rl_tools` types (e.g., `rl_tools::devices::DefaultCPU`).
    -   [X] 1.3.4. Compile the UE project to ensure `rl_tools` is recognized and builds correctly.
    -   [X] 1.3.5. Address any compilation errors or warnings, potentially by wrapping includes with `THIRD_PARTY_INCLUDES_START`/`THIRD_PARTY_INCLUDES_END` and specific warning pragmas. (Covered by 1.2.3)

## Phase 2: C++ Abstraction Layer for RLtools in UE

-   [X] **2.1. Define Core RL Structures and Types **
    -   [X] **2.1.1. Implement `URLAgentManagerSubsystem` (Higher Priority)**
        -   [X] Inherit from `UGameInstanceSubsystem` (or `UWorldSubsystem` if per-level agent lifecycle is strictly needed, but `UGameInstanceSubsystem` is generally preferred for global managers).
        -   [X] This subsystem will be the central point for managing RL agent lifecycles, policies, training, and inference processes.
        -   [X] Implement `Initialize(FSubsystemCollectionBase& Collection)` and `Deinitialize()` methods for robust lifecycle management. (Stubs created, basic structure in place)
            -   [X] Manage `rl_tools` resource allocation (e.g., using `rl_tools::malloc`) in `Initialize()` or specific agent creation methods.
                -   Full implementation: Agent context, training config storage, and allocation logic for rl_tools components (policy, optimizer, actor-critic, replay buffer) are now implemented (see URLAgentManagerSubsystem.cpp, 2025-05-26).
            -   [X] Ensure proper deallocation of `rl_tools` resources (e.g., using `rl_tools::free`) in `Deinitialize()` or agent destruction methods. (**Critical Priority for memory safety**)
                -   Cleanup logic for allocated rl_tools resources is now implemented (2025-05-26).
        -   [X] Ensure subsystem is easily accessible from both C++ (e.g., `GetGameInstance()->GetSubsystem<URLAgentManagerSubsystem>()`) and Blueprints. (Achieved by `UGameInstanceSubsystem` nature)
    -   [X] **2.1.2. Implement Environment Adapter (Critical Priority)**
        -   [X] Create a templated C++ adapter class/struct (e.g., `UEEnvironmentAdapter<SPEC>`) to bridge `URLEnvironmentComponent` (or other UE environment representations) with the C++ API expected by `rl_tools`.
        -   [X] The adapter should implement the necessary static or member functions like `rl_tools::init`, `rl_tools::initial_state`, `rl_tools::step`, `rl_tools::observe`, `rl_tools::reward`, and `rl_tools::terminated`.
        -   [X] Design the adapter to be flexible, allowing for different UE environment sources and efficient data exchange (e.g., minimizing data copies).
        -   [X] Encapsulate environment-specific logic within the adapter, separating it from the core RL agent logic in the subsystem.
    -   [X] **2.1.3. Develop Core C++ Data Conversion & Normalization Utilities (Critical Priority)**
        -   [X] Create a dedicated C++ utility module/namespace (e.g., `RLToolsConversionUtils`) or static methods within the subsystem for robust and efficient data conversions.
        -   [X] Implement functions to convert between UE types (`TArray<float>`, `FVector`, `FRotator`, custom `USTRUCT`s for observations/actions) and `rl_tools::Matrix` or other tensor types.
        -   [X] Implement configurable normalization/denormalization logic for observation and action data within these C++ conversion utilities (see 3.5.2.1 for config).

-   [X] **2.2. Refine Core UE Components for RL Integration**
    -   [X] **2.2.1. Refactor `URLEnvironmentComponent` for Adapter Compatibility (Supporting 2.1.2)**
        -   [X] Align C++ API with `UEEnvironmentAdapter` requirements (renamed methods, added `IsDone`, `GetMaxEpisodeSteps`).
        -   [X] Ensure BlueprintImplementableEvents are correctly invoked by C++ wrappers.
        -   [X] Implement dimension validation for observations and actions.
    - [X] 2.2.2 Create URLAgentComponent
    - [X] **Resolve USTRUCT name collisions** for `FRLEnvironmentConfig` and `FRLTrainingConfig` to ensure proper UHT processing and compilation (Completed 2025-05-27).

## Phase 2.3: Code Review and Refinement (Addressing Tech Debt)

-   [x] **2.3.1. Refactor Agent Management Architecture**
    -   [x] Refactor `URLAgentManagerSubsystem` to delegate to `URLAgentManager` instances or a non-UObject helper class, per `techdebt.md` (Issue #1).
-   [ ] **2.3.2. Consolidate Configuration Structures**
    -   [ ] Consolidate `FLocalRLTrainingConfig` and `FRLTrainingConfig` into a single `FRLTrainingConfig`, per `techdebt.md` (Issue #2).
-   [ ] **2.3.3. Refactor Training Status Representation**
    -   [ ] Refactor `FRLAgentContext` to use `FRLTrainingStatus` USTRUCT for status tracking, per `techdebt.md` (Issue #3).
-   [ ] **2.3.4. Clean Up Conversion Utilities**
    -   [ ] Remove duplicated template functions and `LOG_UERLTOOLS` macro in `RLToolsConversionUtils.cpp`, per `techdebt.md` (Issue #4).
-   [ ] **2.3.5. Standardize Naming Conventions**
    -   [ ] Establish and apply consistent naming conventions (RL, URL, UERL) throughout the plugin, per `techdebt.md` (Issue #6).


## Phase 3: Blueprint Exposure Layer

*(*Note: `techdebt.md` analysis confirms many items below are still placeholders and require full implementation.*)*

-   [ ] **3.1. Expand Blueprint Function Library (Medium Priority)**
    -   [ ] Clarify and consolidate normalization approaches (Z-score vs. min-max), per `techdebt.md` (Issue #5).
    -   [ ] Implement placeholder conversion functions in `URLToolsBlueprintFunctionLibrary.cpp` to call `RLToolsConversionUtils`, per `techdebt.md` (Issue #5).
    -   [ ] Implement actual data marshalling and conversion logic for rl_tools matrices in Blueprint nodes (currently stubs).
    -   [ ] Expose opaque handles or serialization for rl_tools matrices for Blueprint use.
    -   [ ] Expand `URLToolsBlueprintFunctionLibrary` for common RL-related utilities accessible from Blueprints.
        -   [ ] Expose high-level data conversion utilities (wrapping functionalities from 2.1.3) for Blueprint
    -   [ ] Add error/status reporting to Blueprint nodes. use (e.g., `ConvertObservationToRLMatrix`, `ConvertRLActionToUEFormat`).
        -   [ ] Expose normalization/denormalization utilities for Blueprint users if direct manipulation/configuration is needed (alternative to purely config-driven normalization).
        -   [ ] Provide Blueprint-callable functions for debugging and visualizing RL data (e.g., printing `rl_tools::Matrix` content, checking dimensions).
        -   [ ] Add utilities for creating/validating `FRLTrainingConfig` and `FRLEnvironmentConfig` USTRUCTs from Blueprints.
-   [ ] **3.2. Design UObject Wrappers for RL Concepts**
    -   [P] 3.2.1. Create `UObject` subclasses to represent key `rl_tools` entities in Blueprints (e.g., `URLPolicy`, `URLReplayBuffer`, `URLTrainingConfig`). (**Note:** `FRLEnvironmentConfig` and `FRLTrainingConfig` are USTRUCTs. USTRUCTs are generally preferred for configuration data. UObject wrappers might be considered for complex, stateful RL objects if they need independent Blueprint lifecycle management or polymorphism, but start with USTRUCTs where possible.)
    -   [X] 3.2.2. Expose properties of these UObjects/USTRUCTs using `UPROPERTY()` for Blueprint access. (Done for existing USTRUCTs).
-   [ ] **3.3. Implement Blueprint-Callable Functions in `URLAgentManagerSubsystem`**
    -   [X] 3.3.1. In `URLEnvironmentComponent` (Existing, ensure consistency):
        -   [X] Functions to get observation/action space dimensions.
        -   [X] Functions to manually trigger environment reset, step (for debugging/custom loops).
    -   [ ] 3.3.2. In `URLAgentManagerSubsystem`: (**Note:** Aim for a high-level, task-oriented API. Declarations exist, but C++ implementations are mostly placeholders - See `NEEDFIX.md`)
        -   [ ] **Initialization & Configuration**:
            -   [P] `ConfigureAgent(FName AgentName, URLEnvironmentComponent* EnvironmentComponent, FRLTrainingConfig TrainingConfig)` (Structural C++ implementation for rl_tools component setup in place with detailed TODOs; actual `rl_tools` type selection and init calls still pending within the function)
            -   [ ] `LoadPolicy(FName AgentName, FString FilePath)` (C++ Placeholder)
        -   [ ] **Training (Asynchronous)**:
            -   [ ] `StartTraining(FName AgentName)` / `ResumeTraining(FName AgentName)` (C++ Placeholder, see 3.4, 4.1)
            -   [ ] `PauseTraining(FName AgentName)` (C++ Placeholder)
            -   [ ] `StopTraining(FName AgentName)` (C++ Placeholder)
            -   [ ] `SavePolicy(FName AgentName, FString FilePath)` (C++ Placeholder)
        -   [ ] **Inference (Synchronous, performant)**:
            -   [ ] `GetAction(FName AgentName, const TArray<float>& Observation)` returning `TArray<float> Action` (C++ Placeholder - currently returns random, needs actual policy evaluation)
        -   [ ] **Status & Logging**:
            -   [ ] `GetAgentTrainingStatus(FName AgentName)` (e.g., current step, average reward) (C++ Placeholder)
            -   [ ] `GetAgentEpisodeStats(FName AgentName)` (C++ Placeholder)
    -   [X] 3.3.3. Ensure data types are Blueprint-friendly (e.g., `TArray<float>`, `FString`, custom `USTRUCTS` for configs). (Declarations use BP-friendly types).
-   [ ] **3.4. Design for Asynchronous Operations & Blueprint Events (Critical for Training Implementation)**
    -   [ ] 3.4.1. Design long-running tasks in `URLAgentManagerSubsystem` (e.g., `StartTraining`, `StepTraining` if exposed) to be executed asynchronously using Unreal's `FAsyncTask` or `FTSTicker` to avoid blocking the game thread. (**Not Done** - See `NEEDFIX.md`)
    -   [P] 3.4.2. Provide `BlueprintAssignable` delegates (`UPROPERTY(BlueprintAssignable)`) in `URLAgentManagerSubsystem` for asynchronous operation updates. (Declarations in place) Examples:
            -   [X] `OnAgentTrainingStepCompleted(FName AgentName, int32 Step, float Reward)` (Declaration done)
            -   [X] `OnAgentTrainingFinished(FName AgentName, bool bSuccess)` (Declaration done)
            -   [X] `OnAgentPolicySaved(FName AgentName, FString FilePath)` (Declaration done)
            -   [X] `OnAgentPolicyLoaded(FName AgentName, FString FilePath)` (Declaration done)
-   [ ] **3.5. Data Type Conversions and Normalization (Blueprint Facing)**
    -   [ ] 3.5.1. Robust conversion utilities between UE types and `rl_tools` types are primarily C++ concerns (covered by 2.1.3). Blueprint exposure is via `URLToolsBlueprintFunctionLibrary` (3.1) or implicitly handled by subsystem functions.
    -   [ ] 3.5.2. Handle normalization and denormalization of observation/action data, configurable from Blueprints. (**Not Done** - See `NEEDFIX.md`)
        -   [ ] 3.5.2.1. Add `bNormalizeObservations`, `ObservationMean`, `ObservationStd`, `bNormalizeActions`, `ActionMean`, `ActionStd` (or similar, possibly per-element arrays) to `FRLTrainingConfig` or `FRLEnvironmentConfig`.
        -   [ ] 3.5.2.2. Implement normalization logic within the C++ data conversion utilities (2.1.3), using values from the config structure, when converting from UE types to `rl_tools` types.
        -   [ ] 3.5.2.3. Implement denormalization logic similarly when converting from `rl_tools` types back to UE types.

## Phase 4: Training Workflow Implementation

-   [ ] **4.1. Implement Asynchronous Training Loop (Critical Priority)**
    -   [ ] Complete thread safety and robust error propagation in async training code (`RLAsyncTrainingTask`).
    -   [ ] Integrate async training with `URLAgentManagerSubsystem` and Blueprint events.
    -   [ ] Implement the main training loop(s) using asynchronous mechanisms (`FAsyncTask` with a custom task class, or `FTSTicker` for periodic updates).
    -   [ ] Ensure the loop correctly interfaces with the environment adapter (2.1.2) for environment interaction (reset, step, observe) and `rl_tools` training algorithms/components (actor_critic, replay buffer, optimizer updates).
    -   [ ] Trigger Blueprint events (3.4.2) for progress, completion, and other significant training events.
    -   [ ] Implement robust thread safety for any data shared between the game thread (e.g., for observation gathering if environment runs on game thread) and the training thread. Use UE's synchronization primitives (Mutexes, Critical Sections, Atomic variables).
    -   [ ] Add robust status/progress reporting and error handling for async tasks.
-   [ ] **4.2. Replay Buffer Management (for Off-Policy Algorithms)**
    -   [ ] 4.2.1. Expose replay buffer capacity and other relevant configurations (e.g., batch size) in `FRLTrainingConfig`.
    -   [ ] 4.2.2. Potentially visualize replay buffer status (current size, total samples collected) via `GetAgentTrainingStatus` or dedicated Blueprint functions/events.
-   [ ] **4.3. Checkpointing and Model Saving/Loading**
    -   [ ] 4.3.1. Implement functionality in `URLAgentManagerSubsystem` to save trained policies and optimizer states. `rl_tools` supports saving to HDF5 (if compiled with HDF5 support) or its own header-file format. Choose a suitable format.
    -   [ ] 4.3.2. Implement loading of pre-trained policies (and optimizer states if continuing training) in `URLAgentManagerSubsystem`.
    -   [ ] 4.3.3. Expose these via Blueprint functions (e.g., `SavePolicy`, `LoadPolicy` in 3.3.2) and corresponding `BlueprintAssignable` events (3.4.2).
-   [ ] **4.4. Logging and Visualization Hooks**
    -   [ ] 4.4.1. Log training progress (rewards, losses, episode lengths, custom metrics) to UE's Output Log (`UE_LOG`) and potentially to CSV files for external analysis.
    -   [ ] 4.4.2. Explore integration with `rl_tools`' TensorBoard logging if feasible and desired. This might require compiling `rl_tools` with specific flags and handling file paths correctly.
    -   [ ] 4.4.3. Provide Blueprint events or functions to retrieve key metrics (e.g., `GetAgentTrainingStatus` in 3.3.2) for display in UMG or other UE visualization tools.

## Phase 5: Inference Workflow Implementation

-   [ ] **5.1. Optimize Policy Deployment for Inference**
    -   [ ] Ensure `rl_tools` policy evaluation (`rl_tools::evaluate`) is efficient for real-time inference within the game loop.
    -   [ ] Provide options in `URLAgentManagerSubsystem` or via `FRLInferenceConfig` (if needed) for deterministic vs. stochastic policy evaluation modes during inference.
-   [ ] **5.2. Real-time Action Generation**
    -   [ ] 5.2.1. Implement the C++ core of `GetAction(FName AgentName, ...)` in `URLAgentManagerSubsystem`. This function should:
        -   Take an observation (already converted to `TArray<float>`).
        -   Convert it to the `rl_tools::Matrix` format (using 2.1.3 utilities).
        -   Pass it to the loaded `rl_tools` policy for evaluation (`rl_tools::evaluate`).
        -   Convert the resulting `rl_tools::Matrix` action back to `TArray<float>` (using 2.1.3 utilities).
        -   Return the action.
    -   [ ] 5.2.2. Profile and ensure this entire `GetAction` pipeline is highly performant for use in real-time game loops (e.g., called every frame or AI tick).
-   [ ] **5.3. Integration with UE AI Systems**
    -   [ ] 5.3.1. Develop examples or clear guidance on how to use the Blueprint-exposed RL agent (`GetAction` from `URLAgentManagerSubsystem`) within UE Behavior Trees (e.g., by creating a custom `UBTTask_GetRLAction`).
    -   [ ] 5.3.2. Provide examples or guidance on how `URLEnvironmentComponent` (or a similar observation provider) can integrate with UE's AI Perception system to gather observations for the RL agent.

## Phase 6: Advanced Features & Future-Proofing

-   [ ] **6.1. Multi-Agent Support (Architectural Consideration from Start)**
    -   [ ] Design `URLAgentManagerSubsystem` and related data structures to inherently support managing multiple, independent, named agents.
    -   [ ] Implement an internal registry within the subsystem (e.g., `TMap<FName, FRLAgentContext>`) to store state, policy, configs, etc., for each agent.
    -   [ ] Most Blueprint functions in the subsystem (e.g., `ConfigureAgent`, `StartTraining`, `GetAction`) should take an `FName AgentName` parameter.
-   [ ] **6.2. Device Support Extension (CPU/GPU Flexibility)**
    -   [ ] Refactor all core code to support device abstraction (currently hardcoded for CPU).
    -   [ ] Template key C++ functions (e.g., in conversion utilities (2.1.3), environment adapter (2.1.2), core `rl_tools` interactions within the subsystem) on `rl_tools` device types (e.g., `rl_tools::devices::DefaultCPU`, `rl_tools::devices::DefaultGPU`).
    -   [ ] Design data structures and workflows to be adaptable for future GPU support with minimal code duplication. This involves using `rl_tools` device objects appropriately.
    -   [ ] Add configuration options (e.g., in `FRLTrainingConfig`) to select the computation device (CPU/GPU) where applicable, once `rl_tools` GPU operations are integrated.
-   [ ] **6.3. Parameter Tuning Utilities**
    -   [ ] Allow dynamic adjustment of some training parameters (e.g., learning rate, discount factor, exploration parameters) from Blueprints or via UE console commands for easier experimentation and tuning, if feasible with `rl_tools` architecture.
-   [ ] **6.4. Error Handling, Logging, and Validation**
    -   [ ] Add robust error handling, logging, and validation throughout the plugin, especially for Blueprint-exposed APIs and async code.
    -   [ ] Ensure all async and Blueprint-exposed functions propagate errors and status codes.
    -   [ ] Propagate user-friendly error messages or status codes to Blueprint operations (e.g., return values, output pins on BP nodes, specific error events).
    -   [ ] Handle potential exceptions or error codes from `rl_tools` operations gracefully.
-   [ ] **6.5. Memory Management and Cleanup**
    -   [ ] Ensure proper memory management for all dynamically allocated resources (e.g., `rl_tools` matrices, UE objects).
    -   [ ] Implement cleanup logic for all resources in `URLAgentManagerSubsystem` and other components.
-   [ ] **6.6. Test Coverage**
    -   [ ] Expand test coverage: add unit and integration tests for data conversion, environment interaction, agent lifecycle, and Blueprint nodes.
    -   [ ] Add tests for error handling and async task edge cases.

## Phase 7: Documentation and Examples

-   [ ] **7.1. Plugin README**
    -   [ ] 7.1.1. Write/Update a comprehensive `README.md` for the `UERLTools` plugin.
        -   [ ] Installation instructions (including `rl_tools` setup if manual steps involved).
        -   [ ] Overview of features, architecture (subsystem, adapter), and Blueprint API.
        -   [ ] Basic usage examples and workflow descriptions.
-   [ ] **7.2. Example Project/Map**
    -   [ ] 7.2.1. Create a simple, functional example UE project or map demonstrating:
        -   [ ] Setting up an `URLEnvironmentComponent` for a basic task (e.g., a pawn learning to navigate to a target).
        -   [ ] Configuring and training an agent using the `URLAgentManagerSubsystem` via Blueprints.
        -   [ ] Using a trained agent for inference in a simple game scenario.
-   [ ] **7.3. Code Comments and API Documentation**
    -   [ ] 7.3.1. Ensure all C++ classes, functions, and Blueprint-exposed elements are well-commented (using Unreal's documentation comment standards).
    -   [ ] 7.3.2. Consider generating API documentation (e.g., using Doxygen or Unreal's built-in tools if applicable) for C++ developers.

