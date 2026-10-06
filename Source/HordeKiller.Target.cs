// Copyright (c) 2026 alvarocabrero. Licensed under the MIT License. See LICENSE in the repository root.

using UnrealBuildTool;

/// <summary>
/// Build target for the standalone game: the executable without the editor, used for packaged builds.
/// </summary>
public class HordeKillerTarget : TargetRules
{
	public HordeKillerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		// Use the build defaults introduced with the current engine generation, and the newest include
		// order. Both avoid deprecation warnings and keep the project aligned with new engine projects.
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		// Game modules to compile into this target.
		ExtraModuleNames.Add("HordeKiller");
	}
}
