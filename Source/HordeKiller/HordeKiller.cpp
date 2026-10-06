// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "HordeKiller.h"
#include "Modules/ModuleManager.h"

// Registers this module with the engine as the project's primary game module. Every project needs
// exactly one. FDefaultGameModuleImpl is the engine's stock implementation, used because the module
// has no start-up or shut-down logic of its own. The second argument must match the module name in
// HordeKiller.uproject and HordeKiller.Build.cs; the third is the game's name.
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, HordeKiller, "HordeKiller");
