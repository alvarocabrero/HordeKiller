// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Game/HKGameMode.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/HKPlayer.h"
#include "Hordes/HKHordeConfig.h"
#include "Hordes/HKHordeGenerator.h"
#include "UI/HKHUD.h"
#include "Managers/HKActorManager.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogHordeKiller, Log, All);

AHKGameMode::AHKGameMode()
{
	DefaultPawnClass = AHKPlayer::StaticClass();
	HUDClass = AHKHUD::StaticClass();

	// Prefer the player Blueprint; if the asset is missing the C++ class stays.
	static ConstructorHelpers::FClassFinder<AHKPlayer> PlayerBlueprint(TEXT("/Game/Blueprints/Characters/BP_HKPlayer"));
	if (PlayerBlueprint.Succeeded())
	{
		DefaultPawnClass = PlayerBlueprint.Class;
	}

	// Horde for levels without their own generator.
	static ConstructorHelpers::FObjectFinder<UHKHordeConfig> DefaultHorde(TEXT("/Game/Data/Hordes/DA_HKHorde_Default.DA_HKHorde_Default"));
	if (DefaultHorde.Succeeded())
	{
		DefaultHordeConfig = DefaultHorde.Object;
	}
}

void AHKGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHKActorManager::Unregister(this);

	Super::EndPlay(EndPlayReason);
}

void AHKGameMode::BeginPlay()
{
	Super::BeginPlay();

	UHKActorManager::Register(this);

	SetUpHordeGenerator();

	UE_LOG(LogHordeKiller, Log, TEXT("Level: %s, player class: %s, horde generator: %s, horde config: %s"),
		*UGameplayStatics::GetCurrentLevelName(this),
		*GetNameSafe(DefaultPawnClass),
		*GetNameSafe(HordeGenerator),
		HordeGenerator ? *GetNameSafe(HordeGenerator->GetConfig()) : TEXT("none"));
}

void AHKGameMode::SetUpHordeGenerator()
{
	// A generator placed in the level wins. Searched in the world, not the actor manager,
	// because it may not have registered yet.
	for (TActorIterator<AHKHordeGenerator> It(GetWorld()); It; ++It)
	{
		HordeGenerator = *It;
		return;
	}

	// Deferred spawn: set the config before BeginPlay starts the horde.
	const FTransform SpawnTransform = FTransform::Identity;
	HordeGenerator = GetWorld()->SpawnActorDeferred<AHKHordeGenerator>(AHKHordeGenerator::StaticClass(), SpawnTransform);
	if (HordeGenerator)
	{
		HordeGenerator->SetConfig(DefaultHordeConfig);
		UGameplayStatics::FinishSpawningActor(HordeGenerator, SpawnTransform);
	}
}

void AHKGameMode::NotifyPlayerDied()
{
	if (bGameOver)
	{
		return;
	}

	bGameOver = true;

	if (HordeGenerator)
	{
		UE_LOG(LogHordeKiller, Log, TEXT("Player died on wave %d with %d kills"),
			HordeGenerator->GetCurrentWave(), HordeGenerator->GetKills());
		HordeGenerator->StopHorde();
	}

	GetWorldTimerManager().SetTimer(RestartTimer, this, &AHKGameMode::RestartLevel, RestartDelay, false);
}

void AHKGameMode::RestartLevel()
{
	// Reloading the level resets everything.
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}
