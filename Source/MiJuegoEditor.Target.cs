using UnrealBuildTool;

public class MiJuegoEditorTarget : TargetRules
{
	public MiJuegoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("MiJuego");
	}
}
