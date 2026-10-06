#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HordeKillerHUD.generated.h"

/** Minimal canvas HUD: crosshair, health, wave and kill counters. */
UCLASS()
class HORDEKILLER_API AHordeKillerHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
