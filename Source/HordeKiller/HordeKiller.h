// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

/**
 * Main header of the HordeKiller game module.
 *
 * It is intentionally almost empty: it exists so the module has a header to pair with HordeKiller.cpp,
 * and it is the place to put declarations shared by the whole module (log categories, common macros)
 * if they are ever needed.
 */

#pragma once

#include "CoreMinimal.h"

namespace HKConstants
{
	/**
	 * World time, in seconds, used as "this has never happened" for timestamps such as the last shot or the
	 * last attack. Far enough in the past that any cooldown has already elapsed when play begins.
	 */
	inline constexpr float NeverHappenedTime = -1000.f;
}
