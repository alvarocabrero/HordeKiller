// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HKHUD.generated.h"

/**
 * Minimal heads-up display drawn straight onto the canvas.
 *
 * Shows a crosshair, the player's health, the wave number, the enemies still alive, the kill count
 * and a game-over message. It uses the engine's immediate-mode canvas drawing instead of UMG widgets,
 * so it needs no assets and no extra module dependencies.
 */
UCLASS()
class HORDEKILLER_API AHKHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** Called by the engine once per frame to draw the HUD. Everything is redrawn from scratch each time. */
	virtual void DrawHUD() override;
};
