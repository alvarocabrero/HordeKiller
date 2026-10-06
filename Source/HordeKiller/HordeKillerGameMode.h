#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HordeKillerGameMode.generated.h"

class AHordeKillerEnemy;

/** Runs the horde loop: builds the arena, spawns waves of enemies and tracks kills. */
UCLASS()
class HORDEKILLER_API AHordeKillerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHordeKillerGameMode();

	void NotifyEnemyKilled();
	void NotifyPlayerDied();

	int32 GetCurrentWave() const { return CurrentWave; }
	int32 GetEnemiesAlive() const { return EnemiesAlive; }
	int32 GetKills() const { return Kills; }
	bool IsGameOver() const { return bGameOver; }

protected:
	virtual void BeginPlay() override;

	/** Spawns a floor and four walls from basic shapes, so the game is playable on any map. */
	void BuildArena();
	void SpawnArenaBlock(const FVector& Location, const FVector& Size, const FLinearColor& Color);

	void StartNextWave();
	void RestartLevel();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	TSubclassOf<AHordeKillerEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	int32 FirstWaveEnemies = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	int32 EnemiesAddedPerWave = 4;

	// Delay before the first wave and between waves, in seconds.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	float TimeBetweenWaves = 3.f;

	// Enemies spawn in a ring around the player, between these two distances.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	float SpawnRadiusMin = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	float SpawnRadiusMax = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	float RestartDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	bool bBuildArena = true;

	// Half the side of the square arena, in cm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	float ArenaHalfSize = 4000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	float ArenaWallHeight = 400.f;

	// Height of the walkable floor surface.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	float ArenaFloorZ = 5.f;

private:
	int32 CurrentWave = 0;
	int32 EnemiesAlive = 0;
	int32 Kills = 0;
	bool bGameOver = false;

	FTimerHandle WaveTimer;
	FTimerHandle RestartTimer;
};
