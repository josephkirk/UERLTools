// Copyright 2025 NGUYEN PHI HUNG

#pragma once

#include "CoreMinimal.h"

/**
 * UERLTools Log Categories
 */
UERLTOOLS_API DECLARE_LOG_CATEGORY_EXTERN(LogUERLTools, Log, All);
UERLTOOLS_API DECLARE_LOG_CATEGORY_EXTERN(LogUERLToolsRL, Log, All);
UERLTOOLS_API DECLARE_LOG_CATEGORY_EXTERN(LogUERLToolsURL, Log, All);

// Helper macros for consistent logging
#define UE_UERL_LOG(Verbosity, Format, ...) UE_LOG(LogUERLTools, Verbosity, TEXT(Format), ##__VA_ARGS__)
#define UE_UERL_RL_LOG(Verbosity, Format, ...) UE_LOG(LogUERLToolsRL, Verbosity, TEXT("[RL] " Format), ##__VA_ARGS__)
#define UE_UERL_URL_LOG(Verbosity, Format, ...) UE_LOG(LogUERLToolsURL, Verbosity, TEXT("[URL] " Format), ##__VA_ARGS__)

// For backward compatibility
#ifndef LOG_UERLTOOLS
#define LOG_UERLTOOLS LogUERLTools
#endif

// Shorter aliases for common log levels
#define UERL_LOG(Format, ...) UE_UERL_LOG(Log, Format, ##__VA_ARGS__)
#define UERL_WARNING(Format, ...) UE_UERL_LOG(Warning, Format, ##__VA_ARGS__)
#define UERL_ERROR(Format, ...) UE_UERL_LOG(Error, Format, ##__VA_ARGS__)

// RL-specific logs
#define UERL_RL_LOG(Format, ...) UE_UERL_RL_LOG(Log, Format, ##__VA_ARGS__)
#define UERL_RL_WARNING(Format, ...) UE_UERL_RL_LOG(Warning, Format, ##__VA_ARGS__)
#define UERL_RL_ERROR(Format, ...) UE_UERL_RL_LOG(Error, Format, ##__VA_ARGS__)

// URL-specific logs
#define UERL_URL_LOG(Format, ...) UE_UERL_URL_LOG(Log, Format, ##__VA_ARGS__)
#define UERL_URL_WARNING(Format, ...) UE_UERL_URL_LOG(Warning, Format, ##__VA_ARGS__)
#define UERL_URL_ERROR(Format, ...) UE_UERL_URL_LOG(Error, Format, ##__VA_ARGS__)
