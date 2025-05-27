// Copyright 2025 NGUYEN PHI HUNG

using UnrealBuildTool;
using System.IO;

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
				Path.Combine(ModuleDirectory, "..", "ThirdParty", "rl_tools", "include"),
				// Add our module's public include path
				Path.Combine(ModuleDirectory, "Public")
			}
			);

		// Add our private include path
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
				
		
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
				"Projects",
				"DeveloperSettings"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
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
