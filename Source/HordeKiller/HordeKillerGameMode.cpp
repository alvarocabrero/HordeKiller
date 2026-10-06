#include "HordeKillerGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "HordeKillerCharacter.h"
#include "HordeKillerEnemy.h"
#include "HordeKillerHUD.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogHordeKiller, Log, All);

AHordeKillerGameMode::AHordeKillerGameMode()
{
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

	GetWorldTimerManager().SetTimer(WaveTimer, this, &AHordeKillerGameMode::StartNextWave, TimeBetweenWaves, false);
}

void AHordeKillerGameMode::SpawnArenaBlock(const FVector& Location, const FVector& Size, const FLinearColor& Color)
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
	if (!Cube || !Block)
	{
		return;
	}

	UStaticMeshComponent* Mesh = Block->GetStaticMeshComponent();
	// A static mesh component only accepts a new mesh at runtime while it is movable.
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetStaticMesh(Cube);
	// The cube is 100 cm per side.
	Block->SetActorScale3D(Size / 100.f);

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

	SpawnArenaBlock(FVector(0.f, 0.f, ArenaFloorZ - FloorThickness * 0.5f), FVector(Side, Side, FloorThickness), FloorColor);

	const float WallZ = ArenaFloorZ + ArenaWallHeight * 0.5f;
	const float WallOffset = ArenaHalfSize + WallThickness * 0.5f;
	SpawnArenaBlock(FVector(WallOffset, 0.f, WallZ), FVector(WallThickness, Side, ArenaWallHeight), WallColor);
	SpawnArenaBlock(FVector(-WallOffset, 0.f, WallZ), FVector(WallThickness, Side, ArenaWallHeight), WallColor);
	SpawnArenaBlock(FVector(0.f, WallOffset, WallZ), FVector(Side, WallThickness, ArenaWallHeight), WallColor);
	SpawnArenaBlock(FVector(0.f, -WallOffset, WallZ), FVector(Side, WallThickness, ArenaWallHeight), WallColor);
}

void AHordeKillerGameMode::StartNextWave()
{
	if (bGameOver || !EnemyClass)
	{
		return;
	}

	++CurrentWave;
	const int32 EnemyCount = FirstWaveEnemies + (CurrentWave - 1) * EnemiesAddedPerWave;

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector Center = Player ? Player->GetActorLocation() : FVector::ZeroVector;
	// Keep spawns away from the walls.
	const float Limit = ArenaHalfSize - 200.f;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	int32 Spawned = 0;
	for (int32 Index = 0; Index < EnemyCount; ++Index)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(SpawnRadiusMin, SpawnRadiusMax);

		FVector Location = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;
		Location.X = FMath::Clamp(Location.X, -Limit, Limit);
		Location.Y = FMath::Clamp(Location.Y, -Limit, Limit);
		Location.Z = ArenaFloorZ + 100.f;

		if (GetWorld()->SpawnActor<AHordeKillerEnemy>(EnemyClass, Location, FRotator::ZeroRotator, Params))
		{
			++Spawned;
		}
	}

	EnemiesAlive += Spawned;
	UE_LOG(LogHordeKiller, Log, TEXT("Wave %d started: %d enemies spawned around %s"), CurrentWave, Spawned, *Center.ToString());
}

void AHordeKillerGameMode::NotifyEnemyKilled()
{
	++Kills;
	EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);

	if (EnemiesAlive == 0 && !bGameOver)
	{
		UE_LOG(LogHordeKiller, Log, TEXT("Wave %d cleared (%d kills)"), CurrentWave, Kills);
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AHordeKillerGameMode::StartNextWave, TimeBetweenWaves, false);
	}
}

void AHordeKillerGameMode::NotifyPlayerDied()
{
	if (bGameOver)
	{
		return;
	}

	bGameOver = true;
	UE_LOG(LogHordeKiller, Log, TEXT("Player died on wave %d with %d kills"), CurrentWave, Kills);
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().SetTimer(RestartTimer, this, &AHordeKillerGameMode::RestartLevel, RestartDelay, false);
}

void AHordeKillerGameMode::RestartLevel()
{
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}
