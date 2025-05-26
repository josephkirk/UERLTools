// Copyright 2025 NGUYEN PHI HUNG

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Module-wide log categories
#include "UERLLog.h"

/**
 * The public interface to this module
 */
class UERLTOOLS_API FUERLToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
