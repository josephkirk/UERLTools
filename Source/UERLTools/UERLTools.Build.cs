// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UERLTools : ModuleRules
{
	public UERLTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Set C++17 standard as required by rl_tools
		CppStandard = CppStandardVersion.Cpp17;
		
		// Disable some warnings for third-party code
		bEnableUndefinedIdentifierWarnings = false;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// Add rl_tools include path
				System.IO.Path.Combine(ModuleDirectory, "..", "ThirdParty", "rl_tools", "include")
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
			
		// Add definitions for rl_tools if needed
		PublicDefinitions.AddRange(
			new string[]
			{
				"RL_TOOLS_BACKEND_ENABLE_CPU=1"
			}
			);
	}
}
