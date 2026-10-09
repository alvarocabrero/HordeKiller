// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

using UnrealBuildTool;

/// <summary>
/// Build target for the editor: the game module compiled as a library that Unreal Editor loads.
/// This is the target built during day-to-day development ("Development Editor" in Visual Studio).
/// </summary>
public class HordeKillerEditorTarget : TargetRules
{
	public HordeKillerEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		// Same settings as the game target; see HordeKiller.Target.cs.
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		// Game modules to compile into this target.
		ExtraModuleNames.Add("HordeKiller");
	}
}
