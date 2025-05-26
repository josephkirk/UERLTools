# Thoughts & Recommendations: Unreal RLtools Plugin Architecture

This document provides high-level recommendations and rationale for improving the architecture and workflow of the Unreal RLtools plugin, based on the current `TODOs.md` and best practices for Unreal Engine plugin development.

## 1. Centralized RL Management via Subsystem
- **Recommendation:** Use a `UGameInstanceSubsystem` (or `UWorldSubsystem` if per-level agents are needed) for RL agent management instead of a plain `UObject` or `AActor`.
    - **Rationale:**
        - Subsystems are singletons, accessible globally, and have well-defined lifecycles.
        - They simplify access from both C++ and Blueprints, and avoid the need for manual placement in the world.
        - They are the Unreal-standard way to provide persistent, cross-level services.
- **Implementation:** Refactor `URLAgentManager` into `URLAgentManagerSubsystem` inheriting from `UGameInstanceSubsystem`.

## 2. Environment Abstraction & Adapter Pattern
- **Recommendation:** Implement an adapter class/struct that bridges Unreal's `URLEnvironmentComponent` to the C++ API expected by `rl_tools` (e.g., static `rlt::step`, `rlt::observe`, etc.).
    - **Rationale:**
        - Clean separation of engine-facing logic (Blueprint, UE events) and RLtools-facing logic (C++ API).
        - Enables flexible swapping or extension of environments without changing RL agent code.
        - Makes it easier to add support for multi-agent or custom environments in the future.

## 3. Data Conversion & Normalization Utilities
- **Recommendation:** Centralize all conversions between UE types (`TArray<float>`, `FVector`, etc.) and `rl_tools` types (`Matrix`, etc.) in a dedicated utility module or static library class.
    - **Rationale:**
        - Reduces code duplication and risk of subtle bugs.
        - Makes normalization/denormalization, scaling, and clamping configurable and testable.
        - Facilitates future support for different data layouts, batching, or device types (CPU/GPU).
- **Implementation:** Create `URLRLToolsConversionLibrary` or similar, with Blueprint-callable and C++ static functions.

## 4. Blueprint Exposure & Usability
- **Recommendation:**
    - Expose only high-level, user-friendly functions to Blueprints (e.g., `StartTraining`, `GetAction`, `ResetEnvironment`).
    - Use USTRUCTs for configs, and consider UObject wrappers only for complex stateful RL objects (e.g., policies, replay buffers) if Blueprint manipulation is needed.
    - Provide BlueprintAssignable events for training progress, completion, and error reporting.
- **Rationale:**
    - Makes the plugin accessible to non-C++ users.
    - Encourages a clean separation between engine/game logic and RL implementation details.

## 5. Asynchronous Training & Thread Safety
- **Recommendation:**
    - Use Unreal's async task system (`FAsyncTask`, `FTSTicker`, or `AsyncTask` nodes in BP) for long-running RL training.
    - Ensure all RLtools state modifications are thread-safe and use UE's synchronization primitives if needed.
- **Rationale:**
    - Prevents blocking the game thread and keeps the editor/game responsive.
    - Allows real-time progress updates and interruption from Blueprints.

## 6. Testing, Debugging, and Extensibility
- **Recommendation:**
    - Provide a suite of C++ and Blueprint integration tests (e.g., test environments, mock agents).
    - Use Unreal's logging system for RLtools events, errors, and metrics.
    - Document extension points (e.g., how to add a new RL algorithm or environment type).
- **Rationale:**
    - Ensures plugin robustness and maintainability.
    - Makes it easier for others to adopt and extend the plugin.

## 7. Future-proofing: Multi-Agent and Device Support
- **Recommendation:**
    - Architect subsystems, adapters, and conversion utilities to be agnostic to the number of agents and device type (CPU/GPU).
    - Plan for multi-agent support as a future extension (e.g., agent registry in the subsystem).

---

**Summary:**
- Move RL agent management to a Subsystem for global, persistent access.
- Use an adapter pattern for environment abstraction.
- Centralize data conversion and normalization logic.
- Prioritize Blueprint usability and async safety.
- Build for extensibility and future multi-agent/device support.

