using UnrealBuildTool;

public class MiJuegoTarget : TargetRules
{
	public MiJuegoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("MiJuego");
	}
}
