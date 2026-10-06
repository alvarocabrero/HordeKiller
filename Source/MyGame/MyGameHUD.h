#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyGameHUD.generated.h"

/** Minimal canvas HUD: crosshair, health, wave and kill counters. */
UCLASS()
class MYGAME_API AMyGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
