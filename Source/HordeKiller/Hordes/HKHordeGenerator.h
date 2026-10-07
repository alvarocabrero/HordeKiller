// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HKHordeGenerator.generated.h"

class AHKEnemy;
class UHKEnemyPool;
class UHKHordeConfig;

/**
 * Runs a horde: spawns its waves of enemies and keeps track of their progress.
 *
 * All the wave functionality lives here: the countdown between waves, choosing where enemies appear,
 * spawning them, counting how many are alive and how many have been killed, and deciding when the
 * horde is over. What the waves contain is not written in this class; it is read from a data asset,
 * UHKHordeConfig, assigned to the Config property.
 *
 * To give a level its own horde, place one of these actors in it and assign that level's config asset.
 * If a level has no generator, the game mode spawns one with its default config.
 *
 * Enemies appear in a ring around the player and inside a square area centred on this actor.
 * Each enemy is told which generator it belongs to, which is how it reports its death back to it.
 *
 * Enemies are not spawned for each wave. The generator owns a pool (UHKEnemyPool) that creates them
 * in advance and reuses them. The pool is sized from the config: for each enemy class, the largest
 * total of one wave plus the wave after it. Two waves, because the corpses of a wave are still in
 * play when the next one arrives. In endless mode, where waves keep growing, the pool is topped up
 * to the same rule before each wave starts.
 */
UCLASS()
class HORDEKILLER_API AHKHordeGenerator : public AActor
{
	GENERATED_BODY()

public:
	/** Sets up the actor. It has no visual representation in game. */
	AHKHordeGenerator();

	/** Starts the countdown to the first wave. Does nothing if the horde is already running or has no config. */
	void StartHorde();

	/** Stops the horde: no further waves start. Enemies already spawned are left alone. */
	void StopHorde();

	/**
	 * Called by an enemy spawned by this generator when it dies. Updates the counters and, if it was
	 * the last enemy of the wave, schedules the next wave.
	 *
	 * @param Enemy The enemy that died.
	 */
	void NotifyEnemyKilled(AHKEnemy* Enemy);

	/**
	 * Called by an enemy when its corpse is to be removed. Puts the enemy back in the pool.
	 *
	 * @param Enemy The enemy to take out of play.
	 */
	void ReleaseEnemy(AHKEnemy* Enemy);

	/** @return The pool this generator takes its enemies from. */
	UHKEnemyPool* GetEnemyPool() const { return EnemyPool; }

	/**
	 * Changes the horde configuration. Meant to be called before the horde starts.
	 *
	 * @param InConfig Data asset describing the waves.
	 */
	void SetConfig(UHKHordeConfig* InConfig) { Config = InConfig; }

	/**
	 * Changes the size of the area in which enemies may spawn.
	 *
	 * @param InHalfSize Half the side of the square area centred on this actor, in cm. 0 removes the limit.
	 */
	void SetSpawnAreaHalfSize(float InHalfSize) { SpawnAreaHalfSize = InHalfSize; }

	/** @return The data asset this generator reads its waves from. May be null. */
	const UHKHordeConfig* GetConfig() const { return Config; }

	/** @return Number of the wave in progress, starting at 1. It is 0 before the first wave. */
	int32 GetCurrentWave() const { return CurrentWave; }

	/** @return Number of enemies spawned by this generator that are still alive. */
	int32 GetEnemiesAlive() const { return EnemiesAlive; }

	/** @return Total enemies of this generator killed since the level started. */
	int32 GetKills() const { return Kills; }

	/** @return True once every wave of a non-endless horde has been cleared. */
	bool IsHordeComplete() const { return bHordeComplete; }

protected:
	/** Registers with the actor manager and, if bAutoStart is set, starts the horde. */
	virtual void BeginPlay() override;

	/**
	 * Unregisters from the actor manager.
	 *
	 * @param EndPlayReason Why play is ending for this actor (destroyed, level change, game exit...).
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Advances to the next wave and spawns its enemies, or completes the horde if there are no waves left. */
	void StartNextWave();

	/**
	 * Picks a random spawn position for one enemy.
	 *
	 * @param Center Position the ring of spawn points is centred on (the player's location).
	 * @return A point between the config's two spawn radii from Center, kept inside the spawn area.
	 */
	FVector PickSpawnLocation(const FVector& Center) const;

	/**
	 * Makes sure the pool holds enough enemies of each class for a wave and the wave after it.
	 *
	 * @param WaveNumber Number of the first of the two waves, starting at 1.
	 */
	void EnsurePoolForWave(int32 WaveNumber);

	/** Pool that creates the enemies in advance and reuses them between waves. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Horde")
	TObjectPtr<UHKEnemyPool> EnemyPool;

	/** Waves and spawn settings of this horde. Assign a different asset in each level to give each one its own horde. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde")
	TObjectPtr<UHKHordeConfig> Config;

	/** Whether the horde starts by itself when play begins. Turn it off to start it from code with StartHorde. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde")
	bool bAutoStart = true;

	/**
	 * Half the side of the square area, centred on this actor, in which enemies may spawn, in cm.
	 * Spawn points outside it are moved to its edge. 0 means no limit.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde", meta = (ClampMin = "0", Units = "cm"))
	float SpawnAreaHalfSize = 0.f;

	/**
	 * Height above this actor at which enemies spawn, in cm. Place the actor on the floor: the default
	 * leaves an enemy's capsule just clear of the ground.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde", meta = (Units = "cm"))
	float SpawnHeightOffset = 100.f;

private:
	/** Number of the wave in progress; 0 until the first wave starts. */
	int32 CurrentWave = 0;

	/** Enemies spawned and not yet killed. The wave is over when this returns to 0. */
	int32 EnemiesAlive = 0;

	/** Total enemies killed since the level started. */
	int32 Kills = 0;

	/** True between StartHorde and StopHorde or the end of the horde. */
	bool bRunning = false;

	/** True once the last wave of a non-endless horde has been cleared. */
	bool bHordeComplete = false;

	/** Timer that starts the next wave. */
	FTimerHandle WaveTimer;
};
