// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HKGameMode.generated.h"

class AHKEnemy;

/**
 * Rules of the game: the horde loop.
 *
 * At the start of play it builds a walled arena out of basic shapes, then spawns enemies in waves
 * around the player. A new, larger wave starts a few seconds after the last enemy of the current one
 * dies. It also keeps the counters shown on the HUD and restarts the level when the player dies.
 *
 * It selects the player character, enemy and HUD classes too, so no Blueprint game mode is required.
 * For the player and the enemy it uses the Blueprints BP_HKCharacter and BP_HKEnemy when they exist,
 * and falls back to the C++ classes otherwise.
 */
UCLASS()
class HORDEKILLER_API AHKGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Selects the default pawn, HUD and enemy classes, preferring the player and enemy Blueprints if present. */
	AHKGameMode();

	/** Called by an enemy when it dies. Updates the counters and schedules the next wave if it was the last one. */
	void NotifyEnemyKilled();

	/** Called by the player character when it dies. Ends the game and schedules a level restart. */
	void NotifyPlayerDied();

	/** @return Number of the wave in progress, starting at 1. It is 0 before the first wave. */
	int32 GetCurrentWave() const { return CurrentWave; }

	/** @return Number of enemies currently alive. */
	int32 GetEnemiesAlive() const { return EnemiesAlive; }

	/** @return Total enemies killed since the level started. */
	int32 GetKills() const { return Kills; }

	/** @return True once the player has died. */
	bool IsGameOver() const { return bGameOver; }

protected:
	/** Builds the arena and schedules the first wave. */
	virtual void BeginPlay() override;

	/**
	 * Unregisters the game mode from the actor manager.
	 *
	 * @param EndPlayReason Why play is ending (level change, game exit...).
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Spawns a floor and four walls made of scaled cubes, so the game is playable on any map. */
	void BuildArena();

	/**
	 * Spawns one box-shaped piece of the arena.
	 *
	 * @param Location World position of the centre of the box, in cm.
	 * @param Size     Full extent of the box along X, Y and Z, in cm.
	 * @param Color    Colour applied to the box.
	 */
	void SpawnArenaBlock(const FVector& Location, const FVector& Size, const FLinearColor& Color);

	/** Advances the wave counter and spawns that wave's enemies in a ring around the player. */
	void StartNextWave();

	/** Reloads the current level, which resets everything to its initial state. */
	void RestartLevel();

	/** Enemy class spawned by the waves. Defaults to the BP_HKEnemy Blueprint, or to the C++ enemy if that asset is missing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	TSubclassOf<AHKEnemy> EnemyClass;

	/** Number of enemies in the first wave. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	int32 FirstWaveEnemies = 6;

	/** Extra enemies added with each new wave. Wave N has FirstWaveEnemies + (N - 1) * this value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	int32 EnemiesAddedPerWave = 4;

	/** Pause before the first wave and between one wave being cleared and the next starting, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	float TimeBetweenWaves = 3.f;

	/** Closest distance to the player at which an enemy can spawn, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	float SpawnRadiusMin = 1500.f;

	/** Farthest distance from the player at which an enemy can spawn, in cm. Spawns are also kept inside the arena. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	float SpawnRadiusMax = 2500.f;

	/** Time between the player's death and the level restarting, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	float RestartDelay = 3.f;

	/** Whether to generate the arena at the start of play. Turn it off when using a hand-made level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	bool bBuildArena = true;

	/** Half the side of the square arena, in cm. 4000 gives an 80 x 80 m floor centred on the world origin. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	float ArenaHalfSize = 4000.f;

	/** Height of the arena walls, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	float ArenaWallHeight = 400.f;

	/** World height of the walkable floor surface, in cm. Slightly above 0 so it sits on top of any ground the base map has at zero height. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	float ArenaFloorZ = 5.f;

private:
	/** Number of the wave in progress; 0 until the first wave starts. */
	int32 CurrentWave = 0;

	/** Enemies spawned and not yet killed. The wave is over when this returns to 0. */
	int32 EnemiesAlive = 0;

	/** Total enemies killed since the level started. */
	int32 Kills = 0;

	/** True once the player has died. Stops new waves from starting. */
	bool bGameOver = false;

	/** Timer that starts the next wave. */
	FTimerHandle WaveTimer;

	/** Timer that restarts the level after the player dies. */
	FTimerHandle RestartTimer;
};
