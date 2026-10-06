// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "HordeKillerHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "HordeKillerCharacter.h"
#include "HordeKillerGameMode.h"

void AHordeKillerHUD::DrawHUD()
{
	Super::DrawHUD();

	// The canvas only exists while the HUD is being drawn; without it there is nothing to draw on.
	if (!Canvas)
	{
		return;
	}

	// ClipX and ClipY are the width and height of the drawable area, in pixels. The origin is the
	// top-left corner, with Y growing downwards.
	const float CenterX = Canvas->ClipX * 0.5f;
	const float CenterY = Canvas->ClipY * 0.5f;

	// Crosshair: a 16 x 2 horizontal bar and a 2 x 16 vertical bar crossing at the screen centre, which
	// is the line projectiles are fired along.
	DrawRect(FLinearColor::White, CenterX - 8.f, CenterY - 1.f, 16.f, 2.f);
	DrawRect(FLinearColor::White, CenterX - 1.f, CenterY - 8.f, 2.f, 16.f);

	// Built-in engine font, so no font asset is needed. DrawText falls back to a default if it is null.
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	const float TextScale = 1.5f;

	// Health, bottom-left. Rounded up so that the last fraction of health never reads as 0 while alive.
	if (const AHordeKillerCharacter* Player = Cast<AHordeKillerCharacter>(GetOwningPawn()))
	{
		const FString HealthText = FString::Printf(TEXT("Health: %d / %d"),
			FMath::CeilToInt(Player->GetHealth()), FMath::CeilToInt(Player->GetMaxHealth()));
		DrawText(HealthText, FLinearColor::White, 40.f, Canvas->ClipY - 70.f, Font, TextScale);
	}

	// Wave information, top-left. The game mode exists only on the machine running the game rules,
	// which in this single-player game is always the local one.
	if (const AHordeKillerGameMode* GameMode = GetWorld()->GetAuthGameMode<AHordeKillerGameMode>())
	{
		const FString WaveText = FString::Printf(TEXT("Wave: %d   Enemies: %d   Kills: %d"),
			GameMode->GetCurrentWave(), GameMode->GetEnemiesAlive(), GameMode->GetKills());
		DrawText(WaveText, FLinearColor::White, 40.f, 40.f, Font, TextScale);

		// Shown during the delay between the player's death and the level restart. The offsets roughly
		// centre the text at this scale; they are not measured from the actual text size.
		if (GameMode->IsGameOver())
		{
			DrawText(TEXT("GAME OVER"), FLinearColor::Red, CenterX - 110.f, CenterY - 80.f, Font, 3.f);
		}
	}
}
