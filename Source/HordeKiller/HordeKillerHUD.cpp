#include "HordeKillerHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "HordeKillerCharacter.h"
#include "HordeKillerGameMode.h"

void AHordeKillerHUD::DrawHUD()
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

	if (const AHordeKillerCharacter* Player = Cast<AHordeKillerCharacter>(GetOwningPawn()))
	{
		const FString HealthText = FString::Printf(TEXT("Health: %d / %d"),
			FMath::CeilToInt(Player->GetHealth()), FMath::CeilToInt(Player->GetMaxHealth()));
		DrawText(HealthText, FLinearColor::White, 40.f, Canvas->ClipY - 70.f, Font, TextScale);
	}

	if (const AHordeKillerGameMode* GameMode = GetWorld()->GetAuthGameMode<AHordeKillerGameMode>())
	{
		const FString WaveText = FString::Printf(TEXT("Wave: %d   Enemies: %d   Kills: %d"),
			GameMode->GetCurrentWave(), GameMode->GetEnemiesAlive(), GameMode->GetKills());
		DrawText(WaveText, FLinearColor::White, 40.f, 40.f, Font, TextScale);

		if (GameMode->IsGameOver())
		{
			DrawText(TEXT("GAME OVER"), FLinearColor::Red, CenterX - 110.f, CenterY - 80.f, Font, 3.f);
		}
	}
}
