// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Hordes/HKHordeGenerator.h"
#include "Characters/HKEnemy.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Hordes/HKHordeConfig.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/HKActorManager.h"
#include "TimerManager.h"

// Log category for horde events. Filter the Output Log by "LogHKHorde" to see them.
DEFINE_LOG_CATEGORY_STATIC(LogHKHorde, Log, All);

AHKHordeGenerator::AHKHordeGenerator()
{
	// Everything here is driven by timers and by enemies reporting their deaths; nothing runs per frame.
	PrimaryActorTick.bCanEverTick = false;

	// An empty root gives the actor a position, so it can be placed and moved in a level. That
	// position is the centre of the spawn area.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AHKHordeGenerator::BeginPlay()
{
	Super::BeginPlay();

	// Every actor of the project subscribes itself to the manager.
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
		// Without a config there are no waves to play. Say so loudly, since a silent arena is confusing.
		UE_LOG(LogHKHorde, Warning, TEXT("%s has no horde config assigned; no waves will spawn"), *GetName());
		return;
	}

	bRunning = true;
	UE_LOG(LogHKHorde, Log, TEXT("Horde started with config %s"), *Config->GetName());

	// Give the player a moment before the first wave. The timer is one-shot; later waves are scheduled
	// from NotifyEnemyKilled when the current one is cleared.
	GetWorldTimerManager().SetTimer(WaveTimer, this, &AHKHordeGenerator::StartNextWave, Config->TimeBetweenWaves, false);
}

void AHKHordeGenerator::StopHorde()
{
	bRunning = false;

	// Cancels a wave that may be counting down.
	GetWorldTimerManager().ClearTimer(WaveTimer);
}

FVector AHKHordeGenerator::PickSpawnLocation(const FVector& Center) const
{
	// Random point in a ring: a random direction and a random distance between the two radii.
	// (Cos, Sin) of the angle is the unit vector pointing in that direction. Min and Max are applied
	// so that a config with the two radii swapped still works.
	const float Angle = FMath::FRandRange(0.f, 2.f * PI);
	const float Radius = FMath::FRandRange(
		FMath::Min(Config->SpawnRadiusMin, Config->SpawnRadiusMax),
		FMath::Max(Config->SpawnRadiusMin, Config->SpawnRadiusMax));

	FVector Location = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

	const FVector Origin = GetActorLocation();
	if (SpawnAreaHalfSize > 0.f)
	{
		// Points that fall outside the area are pulled back to its edge. Near the edge this can bring a
		// spawn closer to the player than the minimum radius.
		Location.X = FMath::Clamp(Location.X, Origin.X - SpawnAreaHalfSize, Origin.X + SpawnAreaHalfSize);
		Location.Y = FMath::Clamp(Location.Y, Origin.Y - SpawnAreaHalfSize, Origin.Y + SpawnAreaHalfSize);
	}

	// A character's location is its capsule centre, so it has to start above the floor, not on it.
	Location.Z = Origin.Z + SpawnHeightOffset;
	return Location;
}

void AHKHordeGenerator::StartNextWave()
{
	if (!bRunning || !Config)
	{
		return;
	}

	// Ask the config what the next wave contains. It answers "no wave" when a non-endless horde has
	// run out of waves, which is how the horde ends.
	FHKWaveConfig Wave;
	if (!Config->GetWave(CurrentWave + 1, Wave))
	{
		bRunning = false;
		bHordeComplete = true;
		UE_LOG(LogHKHorde, Log, TEXT("Horde complete after %d waves (%d kills)"), CurrentWave, Kills);
		return;
	}

	++CurrentWave;

	// Enemies appear around wherever the player currently is, so there is no safe corner to camp in.
	// If there is no player yet, they appear around the generator.
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector Center = Player ? Player->GetActorLocation() : GetActorLocation();

	FActorSpawnParameters Params;
	// If a spawn point is occupied (for example by another enemy from this wave), nudge the new enemy
	// to a free spot nearby, and spawn it regardless if none is found. A wave must never come up short.
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	int32 Spawned = 0;
	for (const FHKWaveEnemyGroup& Group : Wave.EnemyGroups)
	{
		// A group without a class falls back to the basic enemy, so a half-filled asset still plays.
		UClass* EnemyClass = Group.EnemyClass ? Group.EnemyClass.Get() : AHKEnemy::StaticClass();

		for (int32 Index = 0; Index < Group.Count; ++Index)
		{
			if (AHKEnemy* Enemy = GetWorld()->SpawnActor<AHKEnemy>(EnemyClass, PickSpawnLocation(Center), FRotator::ZeroRotator, Params))
			{
				// Lets the enemy report its death back to this generator.
				Enemy->SetHordeGenerator(this);
				++Spawned;
			}
		}
	}

	// Count what was really spawned, not what was requested, so the wave can always be completed.
	EnemiesAlive += Spawned;
	UE_LOG(LogHKHorde, Log, TEXT("Wave %d started: %d enemies spawned around %s"), CurrentWave, Spawned, *Center.ToString());

	if (Spawned == 0)
	{
		// An empty wave has no enemy whose death would trigger the next one, so move on directly.
		// Otherwise the horde would stall here forever.
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AHKHordeGenerator::StartNextWave, Config->TimeBetweenWaves, false);
	}
}

void AHKHordeGenerator::NotifyEnemyKilled(AHKEnemy* Enemy)
{
	++Kills;

	// Clamped at zero as a safeguard against a death being reported twice.
	EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);

	// Last enemy of the wave: schedule the next one after the usual pause.
	if (EnemiesAlive == 0 && bRunning && Config)
	{
		UE_LOG(LogHKHorde, Log, TEXT("Wave %d cleared (%d kills)"), CurrentWave, Kills);
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AHKHordeGenerator::StartNextWave, Config->TimeBetweenWaves, false);
	}
}
