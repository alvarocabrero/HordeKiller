// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HKHUD.generated.h"

class UFont;

/**
 * Minimal heads-up display drawn straight onto the canvas.
 *
 * Shows a crosshair, the player's health, the wave number, the enemies still alive, the kill count
 * and a game-over message. It uses the engine's immediate-mode canvas drawing instead of UMG widgets,
 * so it needs no assets and no extra module dependencies.
 *
 * Sizes, colours and texts are properties, editable in BP_HKHUD.
 */
UCLASS()
class HORDEKILLER_API AHKHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** Called by the engine once per frame to draw the HUD. Everything is redrawn from scratch each time. */
	virtual void DrawHUD() override;

protected:
	/** Registers the HUD with the actor manager once it is in the world. */
	virtual void BeginPlay() override;

	/**
	 * Unregisters the HUD from the actor manager.
	 *
	 * @param EndPlayReason Why play is ending (level change, game exit...).
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Draws a large message centred horizontally, TitleRaise pixels above the middle of the screen.
	 *
	 * @param Text  Message to draw.
	 * @param Color Colour of the message.
	 * @param Font  Font to draw it with.
	 */
	void DrawTitle(const FText& Text, const FLinearColor& Color, UFont* Font);

protected:
	/** Length of each of the two bars of the crosshair, in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Crosshair", meta = (ClampMin = "0"))
	float CrosshairLength = 16.f;

	/** Thickness of the bars of the crosshair, in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Crosshair", meta = (ClampMin = "0"))
	float CrosshairThickness = 2.f;

	/** Colour of the crosshair. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Crosshair")
	FLinearColor CrosshairColor = FLinearColor::White;

	/** Scale of the health and wave texts, relative to the engine's medium font. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Text", meta = (ClampMin = "0"))
	float TextScale = 1.5f;

	/** Colour of the health and wave texts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Text")
	FLinearColor TextColor = FLinearColor::White;

	/** Distance from the left and top edges of the screen to the texts, in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Text", meta = (ClampMin = "0"))
	float TextMargin = 40.f;

	/** Distance from the bottom edge of the screen to the top of the health text, in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Text", meta = (ClampMin = "0"))
	float HealthBottomMargin = 70.f;

	/** Health text. {0} is the current health and {1} the maximum. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Text")
	FText HealthFormat = NSLOCTEXT("HKHUD", "Health", "Health: {0} / {1}");

	/** Wave text. {0} is the wave number, {1} the enemies still alive and {2} the kills. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Text")
	FText WaveFormat = NSLOCTEXT("HKHUD", "Wave", "Wave: {0}   Enemies: {1}   Kills: {2}");

	/** Scale of the large messages, relative to the engine's medium font. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Title", meta = (ClampMin = "0"))
	float TitleScale = 3.f;

	/** How far above the middle of the screen the large messages are drawn, in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Title")
	float TitleRaise = 80.f;

	/** Message shown when the last wave has been cleared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Title")
	FText AllClearedText = NSLOCTEXT("HKHUD", "AllCleared", "ALL WAVES CLEARED");

	/** Colour of AllClearedText. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Title")
	FLinearColor AllClearedColor = FLinearColor::Green;

	/** Message shown when the player has died. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Title")
	FText GameOverText = NSLOCTEXT("HKHUD", "GameOver", "GAME OVER");

	/** Colour of GameOverText. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Title")
	FLinearColor GameOverColor = FLinearColor::Red;
};
