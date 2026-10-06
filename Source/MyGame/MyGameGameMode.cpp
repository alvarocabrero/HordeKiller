#include "MyGameGameMode.h"
#include "MyGameCharacter.h"

AMyGameGameMode::AMyGameGameMode()
{
	// Uses the C++ character by default. You can swap it for a child Blueprint
	// (BP_MyGameCharacter) in Project Settings > Maps & Modes.
	DefaultPawnClass = AMyGameCharacter::StaticClass();
}
