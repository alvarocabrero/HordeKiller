// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HKEnemyPool.generated.h"

class AHKEnemy;

/**
 * Pool of enemy actors, owned by a horde generator.
 *
 * Spawning and destroying an actor is expensive, and a horde game does both constantly. The pool
 * avoids that: enemies are created once, kept switched off, handed out when a wave needs them
 * (Acquire) and taken back when their corpse is removed (Release). An enemy that is handed out again
 * is the same actor as before, reset to its initial state.
 *
 * The pool keeps a separate stock for each enemy class. EnsureCapacity creates enemies in advance so
 * that waves do not have to spawn any while the game is being played. If the pool still runs out, it
 * creates one more on the spot and logs a warning, so a wave is never left short.
 *
 * It is a component rather than a global service because its size depends on one horde's config, and
 * a level could contain more than one generator.
 */
UCLASS()
class HORDEKILLER_API UHKEnemyPool : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets up the component. It does not tick. */
	UHKEnemyPool();

	/**
	 * Makes sure the pool owns at least Count enemies of a class, counting both those in play and
	 * those waiting, and creates the missing ones switched off.
	 *
	 * @param EnemyClass Class of enemy to stock.
	 * @param Count      Minimum number of enemies of that class the pool must own.
	 */
	void EnsureCapacity(TSubclassOf<AHKEnemy> EnemyClass, int32 Count);

	/**
	 * Takes an enemy from the pool and brings it into play.
	 *
	 * @param EnemyClass Exact class of enemy wanted.
	 * @param Location   World position for it to appear at, in cm.
	 * @return The enemy, active. Null only if one had to be created and spawning failed.
	 */
	AHKEnemy* Acquire(TSubclassOf<AHKEnemy> EnemyClass, const FVector& Location);

	/**
	 * Takes an enemy out of play and puts it back in the pool. Enemies that are not active, or that
	 * this pool did not create, are ignored.
	 *
	 * @param Enemy The enemy to return.
	 */
	void Release(AHKEnemy* Enemy);

	/** @return Number of enemy actors this pool has created, of all classes. */
	int32 GetCreatedCount() const { return AllEnemies.Num(); }

	/** @return Number of enemies currently waiting in the pool, of all classes. */
	int32 GetAvailableCount() const { return Available.Num(); }

	/** @return Number of times an enemy has been handed out. Compare with GetCreatedCount to see the reuse. */
	int32 GetAcquiredCount() const { return AcquiredCount; }

protected:
	/**
	 * Creates one enemy of the given class, switched off, and adds it to the pool.
	 *
	 * @param EnemyClass Class of enemy to create.
	 * @return The new enemy, or nullptr if spawning failed.
	 */
	AHKEnemy* CreateEnemy(TSubclassOf<AHKEnemy> EnemyClass);

	/** @return World position where inactive enemies wait: far below the owner, out of the play area. */
	FVector GetParkLocation() const;

protected:
	/** Distance below the owning actor at which inactive enemies wait, in cm. */
	UPROPERTY(EditAnywhere, Category = "Pool", meta = (ClampMin = "0", Units = "cm"))
	float ParkDepth = 5000.f;

private:
	/** Every enemy created by this pool, in play or not. UPROPERTY keeps the references valid for the garbage collector. */
	UPROPERTY()
	TArray<TObjectPtr<AHKEnemy>> AllEnemies;

	/** The enemies that are switched off and ready to be handed out. */
	UPROPERTY()
	TArray<TObjectPtr<AHKEnemy>> Available;

	/** Number of times an enemy has been handed out. */
	int32 AcquiredCount = 0;
};
