	using System;
	using System.IO;
	using UnrealBuildTool;
 
	public class UERLToolsLibrary : ModuleRules
	{
		public UERLToolsLibrary(ReadOnlyTargetRules Target) : base(Target)
		{
			Type = ModuleType.External;
 		
            // Set C++17 standard as required by rl_tools
            CppStandard = CppStandardVersion.Cpp17;
            
            // Disable some warnings for third-party code
            bEnableUndefinedIdentifierWarnings = false;

		}
	}