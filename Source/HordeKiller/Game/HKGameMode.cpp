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

// Log category for game events (set-up, player death). Filter the Output Log by "LogHordeKiller" to see them.
DEFINE_LOG_CATEGORY_STATIC(LogHordeKiller, Log, All);

AHKGameMode::AHKGameMode()
{
	// Start from the C++ classes. These make the game mode self-sufficient: the game runs with no
	// Blueprint or project setting beyond selecting this game mode.
	DefaultPawnClass = AHKCharacter::StaticClass();
	HUDClass = AHKHUD::StaticClass();

	// Prefer the Blueprint version of the player when it exists, so that values and meshes edited in
	// the editor are the ones used in game. FClassFinder takes the asset path without the "_C" suffix
	// of the generated class. If the asset is missing (for example in a checkout without Git LFS files)
	// the lookup fails, logs a warning and the C++ class set above stays in place.
	static ConstructorHelpers::FClassFinder<AHKCharacter> PlayerBlueprint(TEXT("/Game/Blueprints/Characters/BP_HKCharacter"));
	if (PlayerBlueprint.Succeeded())
	{
		DefaultPawnClass = PlayerBlueprint.Class;
	}

	// Horde played in levels that do not bring their own generator. FObjectFinder takes the full object
	// path, "<package>.<asset name>". If the asset is missing, DefaultHordeConfig stays empty and the
	// generator reports that it has nothing to spawn.
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

	// Every actor of the project subscribes itself to the manager, the game mode included.
	UHKActorManager::Register(this);

	// The arena goes first so that the floor exists before the generator starts dropping enemies on it.
	if (bBuildArena)
	{
		BuildArena();
	}

	SetUpHordeGenerator();

	// Records what is in use, which shows at a glance whether the Blueprint and the config were picked up.
	UE_LOG(LogHordeKiller, Log, TEXT("Player class: %s, horde config: %s"),
		*GetNameSafe(DefaultPawnClass),
		HordeGenerator ? *GetNameSafe(HordeGenerator->GetConfig()) : TEXT("no generator"));
}

void AHKGameMode::SetUpHordeGenerator()
{
	// A generator placed in the level by hand takes precedence: it carries that level's own config.
	// The world is searched directly, instead of asking the actor manager, because the order in which
	// actors begin play is not guaranteed and the generator may not have registered yet.
	for (TActorIterator<AHKHordeGenerator> It(GetWorld()); It; ++It)
	{
		HordeGenerator = *It;
		return;
	}

	// No generator in the level: create one at the centre of the arena floor. Deferred spawning creates
	// the actor but holds back its BeginPlay, which is where the horde starts, until the config and the
	// spawn area have been set.
	const FTransform SpawnTransform(FVector(0.f, 0.f, ArenaFloorZ));
	HordeGenerator = GetWorld()->SpawnActorDeferred<AHKHordeGenerator>(AHKHordeGenerator::StaticClass(), SpawnTransform);
	if (HordeGenerator)
	{
		HordeGenerator->SetConfig(DefaultHordeConfig);

		// Keep spawn points 2 m inside the walls so no enemy starts embedded in one.
		HordeGenerator->SetSpawnAreaHalfSize(ArenaHalfSize - 200.f);

		UGameplayStatics::FinishSpawningActor(HordeGenerator, SpawnTransform);
	}
}

void AHKGameMode::SpawnArenaBlock(const FVector& Location, const FVector& Size, const FLinearColor& Color)
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

void AHKGameMode::BuildArena()
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

void AHKGameMode::NotifyPlayerDied()
{
	// Guard against being called twice, which would schedule two restarts.
	if (bGameOver)
	{
		return;
	}

	bGameOver = true;

	if (HordeGenerator)
	{
		UE_LOG(LogHordeKiller, Log, TEXT("Player died on wave %d with %d kills"),
			HordeGenerator->GetCurrentWave(), HordeGenerator->GetKills());

		// No more waves once the game is over.
		HordeGenerator->StopHorde();
	}

	// Leave the game-over message on screen for a moment before starting again.
	GetWorldTimerManager().SetTimer(RestartTimer, this, &AHKGameMode::RestartLevel, RestartDelay, false);
}

void AHKGameMode::RestartLevel()
{
	// Reloading the level recreates the game mode, the player, the arena and the horde generator, so
	// no state has to be reset by hand. GetCurrentLevelName strips the prefix the editor adds when
	// playing in the editor.
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}
