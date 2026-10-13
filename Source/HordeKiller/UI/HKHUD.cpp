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

	const float CenterX = Canvas->ClipX / 2.f;
	const float CenterY = Canvas->ClipY / 2.f;

	// Crosshair: a horizontal and a vertical bar crossing at the centre.
	DrawRect(CrosshairColor, CenterX - CrosshairLength / 2.f, CenterY - CrosshairThickness / 2.f, CrosshairLength, CrosshairThickness);
	DrawRect(CrosshairColor, CenterX - CrosshairThickness / 2.f, CenterY - CrosshairLength / 2.f, CrosshairThickness, CrosshairLength);

	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;

	// Health, bottom-left. Rounded up so it never reads 0 while alive.
	if (const AHKPlayer* Player = Cast<AHKPlayer>(GetOwningPawn()))
	{
		const FText HealthText = FText::Format(HealthFormat,
			FText::AsNumber(FMath::CeilToInt(Player->GetHealth())), FText::AsNumber(FMath::CeilToInt(Player->GetMaxHealth())));
		DrawText(HealthText.ToString(), TextColor, TextMargin, Canvas->ClipY - HealthBottomMargin, Font, TextScale);
	}

	if (const AHKGameMode* GameMode = GetWorld()->GetAuthGameMode<AHKGameMode>())
	{
		// Wave counters, top-left.
		if (const AHKHordeGenerator* Generator = GameMode->GetHordeGenerator())
		{
			const FText WaveText = FText::Format(WaveFormat, FText::AsNumber(Generator->GetCurrentWave()),
				FText::AsNumber(Generator->GetEnemiesAlive()), FText::AsNumber(Generator->GetKills()));
			DrawText(WaveText.ToString(), TextColor, TextMargin, TextMargin, Font, TextScale);

			if (Generator->IsHordeComplete() && !GameMode->IsGameOver())
			{
				DrawTitle(AllClearedText, AllClearedColor, Font);
			}
		}

		if (GameMode->IsGameOver())
		{
			DrawTitle(GameOverText, GameOverColor, Font);
		}
	}
}

void AHKHUD::DrawTitle(const FText& Text, const FLinearColor& Color, UFont* Font)
{
	const FString String = Text.ToString();

	// Measured, so the message stays centred whatever its text or scale.
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(String, Width, Height, Font, TitleScale);

	DrawText(String, Color, (Canvas->ClipX - Width) / 2.f, Canvas->ClipY / 2.f - TitleRaise, Font, TitleScale);
}