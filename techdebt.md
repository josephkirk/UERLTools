# Technical Debt: UERLTools Plugin

## 1. Blueprint Exposure Layer (Phase 3)
- Many functions in `URLToolsBlueprintFunctionLibrary` are placeholders or stubs (e.g., `Blueprint_ConvertObservationToRLMatrix`, `Blueprint_ConvertRLActionToUEFormat`). There is no actual data marshalling between UE and rl_tools at the Blueprint level.
- **Impact:** Blueprint users cannot fully utilize or debug RL workflows without manual C++ glue or custom Blueprint nodes.
- **Remediation:** Implement actual conversion logic and expose opaque handles or serialization for rl_tools matrices.

## 2. Asynchronous Training Loop (Phase 4)
- `RLAsyncTrainingTask` and related async mechanisms exist, but robust thread safety, error propagation, and integration with `URLAgentManagerSubsystem` are not fully implemented or tested.
- **Impact:** Possible race conditions, incomplete error handling, and lack of robust progress reporting to Blueprints.
- **Remediation:** Complete thread safety mechanisms, add robust error and status reporting, and ensure proper cleanup of async tasks.

## 3. Device Abstraction (CPU/GPU) (Phase 6.2)
- All code is currently hardcoded for CPU (`rlt::devices::DefaultCPU`). No runtime or config-based device selection.
- **Impact:** No GPU acceleration, and code changes will be required for future device support.
- **Remediation:** Template and refactor core classes/utilities for device abstraction; add device selection to config.

## 4. Data Conversion/Normalization
- While `RLToolsConversionUtils` implements normalization/denormalization, there are duplicate or stubbed overloads, and explicit template instantiations are commented out or missing.
- **Impact:** Potential linker errors and inconsistent behavior if new types are used.
- **Remediation:** Clean up utility API and ensure all needed template instantiations are provided.

## 5. Documentation and Comments
- Many files have high-level comments, but detailed documentation (usage, edge cases, error modes) is missing, especially for Blueprint-facing APIs and custom adapters.
- **Impact:** Steep learning curve for new contributors and Blueprint users.
- **Remediation:** Expand doc comments, usage examples, and Blueprint node documentation.

## 6. Test Coverage
- Only `RLToolsTest` exists, testing basic matrix ops. No automated or integration tests for agent lifecycle, environment interaction, or Blueprint nodes.
- **Impact:** Bugs may go undetected, especially in edge cases or after refactors.
- **Remediation:** Add unit tests for conversion, environment, agent manager, and Blueprint exposure.

## 7. Error Handling and Validation
- Some error and dimension checks exist (e.g., in `RLEnvironmentComponent`), but many functions lack robust error propagation, especially in async and Blueprint-exposed code.
- **Impact:** Silent failures, hard-to-debug issues in production or Blueprint workflows.
- **Remediation:** Add error codes, logging, and status returns throughout.

## 8. Memory Management
- Manual allocation/free for rl_tools matrices and contexts is present, but no RAII wrappers or smart pointer usage. Cleanup is handled, but error-prone.
- **Impact:** Risk of leaks or double-free if control flow changes.
- **Remediation:** Consider RAII wrappers for rl_tools allocations.

## 9. Code Duplication and Stubs
- Duplicate or stubbed functions in conversion utilities and adapters. Some old API methods remain in `RLEnvironmentComponent`.
- **Impact:** Maintenance burden, confusion.
- **Remediation:** Remove dead code and consolidate utility functions.
