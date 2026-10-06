using UnrealBuildTool;

public class HordeKillerEditorTarget : TargetRules
{
	public HordeKillerEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("HordeKiller");
	}
}
