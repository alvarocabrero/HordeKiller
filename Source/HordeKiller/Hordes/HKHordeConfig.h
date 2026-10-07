// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HKHordeConfig.generated.h"

class AHKEnemy;

/**
 * A number of enemies of one class, as part of a wave.
 */
USTRUCT(BlueprintType)
struct HORDEKILLER_API FHKWaveEnemyGroup
{
	GENERATED_BODY()

	/** Enemy class to spawn. If left empty, the basic C++ enemy (AHKEnemy) is used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TSubclassOf<AHKEnemy> EnemyClass;

	/** How many enemies of this class the wave spawns. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0"))
	int32 Count = 1;
};

/**
 * One wave of a horde: the groups of enemies that are spawned together.
 */
USTRUCT(BlueprintType)
struct HORDEKILLER_API FHKWaveConfig
{
	GENERATED_BODY()

public:
	/** @return Total number of enemies in the wave, adding up all its groups. */
	int32 GetEnemyCount() const;

public:
	/** Enemies in this wave. Add one entry per enemy class; all of them spawn at the start of the wave. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<FHKWaveEnemyGroup> EnemyGroups;
};

/**
 * Data asset describing a whole horde: the list of waves and how they are spawned.
 *
 * It holds configuration only. The logic that reads it lives in AHKHordeGenerator. Create one asset
 * per level (or per difficulty) in the editor with Add > Miscellaneous > Data Asset > HK Horde Config,
 * and assign it to the horde generator of that level.
 */
UCLASS(BlueprintType)
class HORDEKILLER_API UHKHordeConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Works out the contents of a wave, including the extra waves generated in endless mode.
	 *
	 * @param WaveNumber Number of the wave, starting at 1.
	 * @param OutWave    Receives the groups to spawn for that wave.
	 * @return True if the wave exists. False once the horde is over, or if no waves are configured.
	 */
	bool GetWave(int32 WaveNumber, FHKWaveConfig& OutWave) const;

public:
	/** Waves of the horde, played in order from top to bottom. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	TArray<FHKWaveConfig> Waves;

	/** Pause before the first wave and between one wave being cleared and the next one starting, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves", meta = (ClampMin = "0", Units = "s"))
	float TimeBetweenWaves = 3.f;

	/**
	 * What happens after the last wave in the list. If enabled, the last wave keeps repeating, growing
	 * each time; the horde never ends. If disabled, the horde is complete once the last wave is cleared.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Endless")
	bool bEndless = true;

	/**
	 * Enemies added on each repetition of the last wave in endless mode. They are added to the first
	 * group of that wave. With 4, the repetitions have 4, 8, 12... more enemies than the last wave.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Endless", meta = (ClampMin = "0", EditCondition = "bEndless"))
	int32 EndlessEnemiesAddedPerWave = 4;

	/** Closest distance to the player at which an enemy can spawn, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0", Units = "cm"))
	float SpawnRadiusMin = 1500.f;

	/** Farthest distance from the player at which an enemy can spawn, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0", Units = "cm"))
	float SpawnRadiusMax = 2500.f;
};
