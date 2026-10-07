// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "UI/HKHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Characters/HKPlayer.h"
#include "Game/HKGameMode.h"
#include "Hordes/HKHordeGenerator.h"
#include "Managers/HKActorManager.h"

void AHKHUD::BeginPlay()
{
	Super::BeginPlay();

	UHKActorManager::Register(this);
}

void AHKHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHKActorManager::Unregister(this);

	Super::EndPlay(EndPlayReason);
}

void AHKHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float CenterX = Canvas->ClipX * 0.5f;
	const float CenterY = Canvas->ClipY * 0.5f;

	// Crosshair.
	DrawRect(FLinearColor::White, CenterX - 8.f, CenterY - 1.f, 16.f, 2.f);
	DrawRect(FLinearColor::White, CenterX - 1.f, CenterY - 8.f, 2.f, 16.f);

	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	const float TextScale = 1.5f;

	// Health, bottom-left. Rounded up so it never reads 0 while alive.
	if (const AHKPlayer* Player = Cast<AHKPlayer>(GetOwningPawn()))
	{
		const FString HealthText = FString::Printf(TEXT("Health: %d / %d"),
			FMath::CeilToInt(Player->GetHealth()), FMath::CeilToInt(Player->GetMaxHealth()));
		DrawText(HealthText, FLinearColor::White, 40.f, Canvas->ClipY - 70.f, Font, TextScale);
	}

	if (const AHKGameMode* GameMode = GetWorld()->GetAuthGameMode<AHKGameMode>())
	{
		// Wave counters, top-left.
		if (const AHKHordeGenerator* Generator = GameMode->GetHordeGenerator())
		{
			const FString WaveText = FString::Printf(TEXT("Wave: %d   Enemies: %d   Kills: %d"),
				Generator->GetCurrentWave(), Generator->GetEnemiesAlive(), Generator->GetKills());
			DrawText(WaveText, FLinearColor::White, 40.f, 40.f, Font, TextScale);

			if (Generator->IsHordeComplete() && !GameMode->IsGameOver())
			{
				DrawText(TEXT("ALL WAVES CLEARED"), FLinearColor::Green, CenterX - 190.f, CenterY - 80.f, Font, 3.f);
			}
		}

		// Text offsets are approximate, not measured from the text size.
		if (GameMode->IsGameOver())
		{
			DrawText(TEXT("GAME OVER"), FLinearColor::Red, CenterX - 110.f, CenterY - 80.f, Font, 3.f);
		}
	}
}
