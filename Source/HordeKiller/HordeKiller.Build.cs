// Copyright (c) 2026 alvarocabrero. Licensed under the MIT License. See LICENSE in the repository root.

using UnrealBuildTool;

/// <summary>
/// Build rules for the HordeKiller game module: how it is compiled and which engine modules it links against.
/// </summary>
public class HordeKiller : ModuleRules
{
	public HordeKiller(ReadOnlyTargetRules Target) : base(Target)
	{
		// Use the engine's shared precompiled headers unless a file declares its own. This is the
		// standard setting for game modules and keeps compile times short.
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Engine modules whose headers this module includes:
		//   Core, CoreUObject - basic types and the UObject system.
		//   Engine            - actors, components, game framework, physics.
		//   InputCore         - key definitions (EKeys).
		//   EnhancedInput     - input actions, mapping contexts and modifiers.
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
