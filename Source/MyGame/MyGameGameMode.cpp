#include "MyGameGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MyGameCharacter.h"
#include "MyGameEnemy.h"
#include "MyGameHUD.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMyGame, Log, All);

AMyGameGameMode::AMyGameGameMode()
{
	DefaultPawnClass = AMyGameCharacter::StaticClass();
	HUDClass = AMyGameHUD::StaticClass();
	EnemyClass = AMyGameEnemy::StaticClass();
}

void AMyGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bBuildArena)
	{
		BuildArena();
	}

	GetWorldTimerManager().SetTimer(WaveTimer, this, &AMyGameGameMode::StartNextWave, TimeBetweenWaves, false);
}

void AMyGameGameMode::SpawnArenaBlock(const FVector& Location, const FVector& Size, const FLinearColor& Color)
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

void AMyGameGameMode::BuildArena()
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

void AMyGameGameMode::StartNextWave()
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

		if (GetWorld()->SpawnActor<AMyGameEnemy>(EnemyClass, Location, FRotator::ZeroRotator, Params))
		{
			++Spawned;
		}
	}

	EnemiesAlive += Spawned;
	UE_LOG(LogMyGame, Log, TEXT("Wave %d started: %d enemies spawned around %s"), CurrentWave, Spawned, *Center.ToString());
}

void AMyGameGameMode::NotifyEnemyKilled()
{
	++Kills;
	EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);

	if (EnemiesAlive == 0 && !bGameOver)
	{
		UE_LOG(LogMyGame, Log, TEXT("Wave %d cleared (%d kills)"), CurrentWave, Kills);
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AMyGameGameMode::StartNextWave, TimeBetweenWaves, false);
	}
}

void AMyGameGameMode::NotifyPlayerDied()
{
	if (bGameOver)
	{
		return;
	}

	bGameOver = true;
	UE_LOG(LogMyGame, Log, TEXT("Player died on wave %d with %d kills"), CurrentWave, Kills);
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().SetTimer(RestartTimer, this, &AMyGameGameMode::RestartLevel, RestartDelay, false);
}

void AMyGameGameMode::RestartLevel()
{
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}
