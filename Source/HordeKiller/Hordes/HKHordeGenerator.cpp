// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Hordes/HKHordeGenerator.h"
#include "Characters/HKEnemy.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Hordes/HKHordeConfig.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/HKActorManager.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogHKHorde, Log, All);

AHKHordeGenerator::AHKHordeGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	// Gives the actor a position: the centre of the spawn area.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AHKHordeGenerator::BeginPlay()
{
	Super::BeginPlay();

	UHKActorManager::Register(this);

	if (bAutoStart)
	{
		StartHorde();
	}
}

void AHKHordeGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHKActorManager::Unregister(this);

	Super::EndPlay(EndPlayReason);
}

void AHKHordeGenerator::StartHorde()
{
	if (bRunning)
	{
		return;
	}

	if (!Config)
	{
		UE_LOG(LogHKHorde, Warning, TEXT("%s has no horde config assigned; no waves will spawn"), *GetName());
		return;
	}

	bRunning = true;
	UE_LOG(LogHKHorde, Log, TEXT("Horde started with config %s"), *Config->GetName());

	// First wave after a pause; later ones are scheduled from NotifyEnemyKilled.
	GetWorldTimerManager().SetTimer(WaveTimer, this, &AHKHordeGenerator::StartNextWave, Config->TimeBetweenWaves, false);
}

void AHKHordeGenerator::StopHorde()
{
	bRunning = false;
	GetWorldTimerManager().ClearTimer(WaveTimer);
}

FVector AHKHordeGenerator::PickSpawnLocation(const FVector& Center) const
{
	// Random point in a ring. Min/Max tolerate a config with the radii swapped.
	const float Angle = FMath::FRandRange(0.f, 2.f * PI);
	const float Radius = FMath::FRandRange(
		FMath::Min(Config->SpawnRadiusMin, Config->SpawnRadiusMax),
		FMath::Max(Config->SpawnRadiusMin, Config->SpawnRadiusMax));

	FVector Location = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

	// Pull points outside the spawn area back to its edge.
	const FVector Origin = GetActorLocation();
	if (SpawnAreaHalfSize > 0.f)
	{
		Location.X = FMath::Clamp(Location.X, Origin.X - SpawnAreaHalfSize, Origin.X + SpawnAreaHalfSize);
		Location.Y = FMath::Clamp(Location.Y, Origin.Y - SpawnAreaHalfSize, Origin.Y + SpawnAreaHalfSize);
	}

	Location.Z = Origin.Z + SpawnHeightOffset;
	return Location;
}

void AHKHordeGenerator::StartNextWave()
{
	if (!bRunning || !Config)
	{
		return;
	}

	// No wave left means a non-endless horde is over.
	FHKWaveConfig Wave;
	if (!Config->GetWave(CurrentWave + 1, Wave))
	{
		bRunning = false;
		bHordeComplete = true;
		UE_LOG(LogHKHorde, Log, TEXT("Horde complete after %d waves (%d kills)"), CurrentWave, Kills);
		return;
	}

	++CurrentWave;

	// Spawn around the player, so there is no safe corner.
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector Center = Player ? Player->GetActorLocation() : GetActorLocation();

	// Always spawn, so a wave never comes up short.
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	int32 Spawned = 0;
	for (const FHKWaveEnemyGroup& Group : Wave.EnemyGroups)
	{
		// A group without a class uses the basic enemy.
		UClass* EnemyClass = Group.EnemyClass ? Group.EnemyClass.Get() : AHKEnemy::StaticClass();

		for (int32 Index = 0; Index < Group.Count; ++Index)
		{
			if (AHKEnemy* Enemy = GetWorld()->SpawnActor<AHKEnemy>(EnemyClass, PickSpawnLocation(Center), FRotator::ZeroRotator, Params))
			{
				Enemy->SetHordeGenerator(this);
				++Spawned;
			}
		}
	}

	EnemiesAlive += Spawned;
	UE_LOG(LogHKHorde, Log, TEXT("Wave %d started: %d enemies spawned around %s"), CurrentWave, Spawned, *Center.ToString());

	// An empty wave has no death to trigger the next one, so schedule it here.
	if (Spawned == 0)
	{
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AHKHordeGenerator::StartNextWave, Config->TimeBetweenWaves, false);
	}
}

void AHKHordeGenerator::NotifyEnemyKilled(AHKEnemy* Enemy)
{
	++Kills;
	EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);

	// Wave cleared: schedule the next one.
	if (EnemiesAlive == 0 && bRunning && Config)
	{
		UE_LOG(LogHKHorde, Log, TEXT("Wave %d cleared (%d kills)"), CurrentWave, Kills);
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AHKHordeGenerator::StartNextWave, Config->TimeBetweenWaves, false);
	}
}
