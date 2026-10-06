#include "MiJuegoGameMode.h"
#include "MiJuegoCharacter.h"

AMiJuegoGameMode::AMiJuegoGameMode()
{
	// Por defecto usa el personaje C++. Puedes cambiarlo por un Blueprint hijo
	// (BP_MiJuegoCharacter) desde Project Settings > Maps & Modes.
	DefaultPawnClass = AMiJuegoCharacter::StaticClass();
}
