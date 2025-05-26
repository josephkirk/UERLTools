// Copyright 2025 NGUYEN PHI HUNG

#include "UERLTools.h"
#include "RLToolsTest.h"
#include "UERLLog.h"

#define LOCTEXT_NAMESPACE "FUERLToolsModule"

void FUERLToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UERL_LOG("UERLTools module started");
	
	// Test rl_tools integration on startup
	URLToolsTest* TestObject = NewObject<URLToolsTest>();
	if (TestObject)
	{
		TestObject->TestRLToolsIntegration();
	}
}

void FUERLToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.
	// For modules that support dynamic reloading, we call this function before unloading the module.
	UERL_LOG("UERLTools module shutdown");
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUERLToolsModule, UERLTools)
