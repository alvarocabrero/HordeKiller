// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Game/HKGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Characters/HKCharacter.h"
#include "Hordes/HKHordeConfig.h"
#include "Hordes/HKHordeGenerator.h"
#include "UI/HKHUD.h"
#include "Managers/HKActorManager.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogHordeKiller, Log, All);

AHKGameMode::AHKGameMode()
{
	DefaultPawnClass = AHKCharacter::StaticClass();
	HUDClass = AHKHUD::StaticClass();

	// Prefer the player Blueprint; if the asset is missing the C++ class stays.
	static ConstructorHelpers::FClassFinder<AHKCharacter> PlayerBlueprint(TEXT("/Game/Blueprints/Characters/BP_HKCharacter"));
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

	// Arena first, so the floor exists before enemies spawn.
	if (bBuildArena)
	{
		BuildArena();
	}

	SetUpHordeGenerator();

	UE_LOG(LogHordeKiller, Log, TEXT("Player class: %s, horde config: %s"),
		*GetNameSafe(DefaultPawnClass),
		HordeGenerator ? *GetNameSafe(HordeGenerator->GetConfig()) : TEXT("no generator"));
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
	const FTransform SpawnTransform(FVector(0.f, 0.f, ArenaFloorZ));
	HordeGenerator = GetWorld()->SpawnActorDeferred<AHKHordeGenerator>(AHKHordeGenerator::StaticClass(), SpawnTransform);
	if (HordeGenerator)
	{
		HordeGenerator->SetConfig(DefaultHordeConfig);

		// Keep spawns 2 m inside the walls.
		HordeGenerator->SetSpawnAreaHalfSize(ArenaHalfSize - 200.f);

		UGameplayStatics::FinishSpawningActor(HordeGenerator, SpawnTransform);
	}
}

void AHKGameMode::SpawnArenaBlock(const FVector& Location, const FVector& Size, const FLinearColor& Color)
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
	if (!Cube || !Block)
	{
		return;
	}

	UStaticMeshComponent* Mesh = Block->GetStaticMeshComponent();

	// A static component rejects a new mesh at runtime; make it movable first.
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetStaticMesh(Cube);

	// The cube is 100 cm per side.
	Block->SetActorScale3D(Size / 100.f);

	if (UMaterialInstanceDynamic* Material = Mesh->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void AHKGameMode::BuildArena()
{
	const float Side = ArenaHalfSize * 2.f;
	const float FloorThickness = 100.f;
	const float WallThickness = 100.f;
	const FLinearColor FloorColor(0.25f, 0.27f, 0.3f);
	const FLinearColor WallColor(0.12f, 0.13f, 0.16f);

	// Blocks are positioned by their centre; the floor's top sits at ArenaFloorZ.
	SpawnArenaBlock(FVector(0.f, 0.f, ArenaFloorZ - FloorThickness * 0.5f), FVector(Side, Side, FloorThickness), FloorColor);

	// Walls stand on the floor, just outside its edge.
	const float WallZ = ArenaFloorZ + ArenaWallHeight * 0.5f;
	const float WallOffset = ArenaHalfSize + WallThickness * 0.5f;
	SpawnArenaBlock(FVector(WallOffset, 0.f, WallZ), FVector(WallThickness, Side, ArenaWallHeight), WallColor);
	SpawnArenaBlock(FVector(-WallOffset, 0.f, WallZ), FVector(WallThickness, Side, ArenaWallHeight), WallColor);
	SpawnArenaBlock(FVector(0.f, WallOffset, WallZ), FVector(Side, WallThickness, ArenaWallHeight), WallColor);
	SpawnArenaBlock(FVector(0.f, -WallOffset, WallZ), FVector(Side, WallThickness, ArenaWallHeight), WallColor);
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
