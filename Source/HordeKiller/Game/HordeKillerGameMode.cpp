// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Game/HordeKillerGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Characters/HordeKillerCharacter.h"
#include "Characters/HordeKillerEnemy.h"
#include "UI/HordeKillerHUD.h"
#include "TimerManager.h"

// Log category for gameplay events (waves, deaths). Filter the Output Log by "LogHordeKiller" to see them.
DEFINE_LOG_CATEGORY_STATIC(LogHordeKiller, Log, All);

AHordeKillerGameMode::AHordeKillerGameMode()
{
	// These make the game mode self-sufficient: the player spawns as the C++ character with the C++ HUD
	// without any Blueprint or project setting beyond selecting this game mode.
	DefaultPawnClass = AHordeKillerCharacter::StaticClass();
	HUDClass = AHordeKillerHUD::StaticClass();
	EnemyClass = AHordeKillerEnemy::StaticClass();
}

void AHordeKillerGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bBuildArena)
	{
		BuildArena();
	}

	// Give the player a moment to get their bearings before the first wave. The timer is one-shot;
	// later waves are scheduled from NotifyEnemyKilled.
	GetWorldTimerManager().SetTimer(WaveTimer, this, &AHordeKillerGameMode::StartNextWave, TimeBetweenWaves, false);
}

void AHordeKillerGameMode::SpawnArenaBlock(const FVector& Location, const FVector& Size, const FLinearColor& Color)
{
	// LoadObject is used here, instead of the constructor-only FObjectFinder, because this runs during play.
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
	if (!Cube || !Block)
	{
		return;
	}

	UStaticMeshComponent* Mesh = Block->GetStaticMeshComponent();

	// A static mesh actor is "Static" by default, and a static component refuses to change its mesh
	// once the game is running. Making it movable first allows the mesh to be assigned.
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetStaticMesh(Cube);

	// The basic cube is 100 cm per side, so the scale is the wanted size divided by 100. The cube has
	// box collision built in, which scales with it, so the block is solid without further setup.
	Block->SetActorScale3D(Size / 100.f);

	// The engine's basic shape material exposes a vector parameter named "Color".
	if (UMaterialInstanceDynamic* Material = Mesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void AHordeKillerGameMode::BuildArena()
{
	const float Side = ArenaHalfSize * 2.f;
	const float FloorThickness = 100.f;
	const float WallThickness = 100.f;
	const FLinearColor FloorColor(0.25f, 0.27f, 0.3f);
	const FLinearColor WallColor(0.12f, 0.13f, 0.16f);

	// Blocks are positioned by their centre. Lowering the floor by half its thickness puts its top
	// surface exactly at ArenaFloorZ.
	SpawnArenaBlock(FVector(0.f, 0.f, ArenaFloorZ - FloorThickness * 0.5f), FVector(Side, Side, FloorThickness), FloorColor);

	// Walls stand on the floor (centre half a wall-height above it) and sit just outside the floor's
	// edge (centre half a wall-thickness beyond it), so the playable area is the full floor.
	const float WallZ = ArenaFloorZ + ArenaWallHeight * 0.5f;
	const float WallOffset = ArenaHalfSize + WallThickness * 0.5f;
	SpawnArenaBlock(FVector(WallOffset, 0.f, WallZ), FVector(WallThickness, Side, ArenaWallHeight), WallColor);  // +X side
	SpawnArenaBlock(FVector(-WallOffset, 0.f, WallZ), FVector(WallThickness, Side, ArenaWallHeight), WallColor); // -X side
	SpawnArenaBlock(FVector(0.f, WallOffset, WallZ), FVector(Side, WallThickness, ArenaWallHeight), WallColor);  // +Y side
	SpawnArenaBlock(FVector(0.f, -WallOffset, WallZ), FVector(Side, WallThickness, ArenaWallHeight), WallColor); // -Y side
}

void AHordeKillerGameMode::StartNextWave()
{
	if (bGameOver || !EnemyClass)
	{
		return;
	}

	// Linear difficulty ramp: 6, 10, 14, 18... enemies with the default values.
	++CurrentWave;
	const int32 EnemyCount = FirstWaveEnemies + (CurrentWave - 1) * EnemiesAddedPerWave;

	// Enemies appear around wherever the player currently is, so there is no safe corner to camp in.
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector Center = Player ? Player->GetActorLocation() : FVector::ZeroVector;

	// Keep spawn points 2 m inside the walls so no enemy starts embedded in one.
	const float Limit = ArenaHalfSize - 200.f;

	// If a spawn point is occupied (for example by another enemy from this wave), nudge the new enemy
	// to a free spot nearby, and spawn it regardless if none is found. A wave must never come up short.
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	int32 Spawned = 0;
	for (int32 Index = 0; Index < EnemyCount; ++Index)
	{
		// Random point in a ring around the player: a random direction and a random distance between
		// the two radii. (Cos, Sin) of the angle is the unit vector pointing in that direction.
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(SpawnRadiusMin, SpawnRadiusMax);

		FVector Location = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

		// Points that fall outside the arena are pulled back to its edge. Near a wall this can bring a
		// spawn closer to the player than SpawnRadiusMin.
		Location.X = FMath::Clamp(Location.X, -Limit, Limit);
		Location.Y = FMath::Clamp(Location.Y, -Limit, Limit);

		// A character's location is its capsule centre. 100 cm above the floor leaves the capsule
		// (half-height 88 cm) just clear of the ground, and the enemy drops into place.
		Location.Z = ArenaFloorZ + 100.f;

		if (GetWorld()->SpawnActor<AHordeKillerEnemy>(EnemyClass, Location, FRotator::ZeroRotator, Params))
		{
			++Spawned;
		}
	}

	// Count what was really spawned, not what was requested, so the wave can always be completed.
	EnemiesAlive += Spawned;
	UE_LOG(LogHordeKiller, Log, TEXT("Wave %d started: %d enemies spawned around %s"), CurrentWave, Spawned, *Center.ToString());
}

void AHordeKillerGameMode::NotifyEnemyKilled()
{
	++Kills;

	// Clamped at zero as a safeguard against an enemy that was not spawned by a wave being killed.
	EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);

	// Last enemy of the wave: schedule the next one after the usual pause.
	if (EnemiesAlive == 0 && !bGameOver)
	{
		UE_LOG(LogHordeKiller, Log, TEXT("Wave %d cleared (%d kills)"), CurrentWave, Kills);
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AHordeKillerGameMode::StartNextWave, TimeBetweenWaves, false);
	}
}

void AHordeKillerGameMode::NotifyPlayerDied()
{
	// Guard against being called twice, which would schedule two restarts.
	if (bGameOver)
	{
		return;
	}

	bGameOver = true;
	UE_LOG(LogHordeKiller, Log, TEXT("Player died on wave %d with %d kills"), CurrentWave, Kills);

	// Cancel a wave that may be pending, then leave the game-over message on screen for a moment
	// before starting again.
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().SetTimer(RestartTimer, this, &AHordeKillerGameMode::RestartLevel, RestartDelay, false);
}

void AHordeKillerGameMode::RestartLevel()
{
	// Reloading the level recreates the game mode, the player and the arena, so no state has to be
	// reset by hand. GetCurrentLevelName strips the prefix the editor adds when playing in the editor.
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}
