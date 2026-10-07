// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Hordes/HKHordeConfig.h"
#include "Characters/HKEnemy.h"

int32 FHKWaveConfig::GetEnemyCount() const
{
	int32 Total = 0;
	for (const FHKWaveEnemyGroup& Group : EnemyGroups)
	{
		Total += FMath::Max(0, Group.Count);
	}
	return Total;
}

bool UHKHordeConfig::GetWave(int32 WaveNumber, FHKWaveConfig& OutWave) const
{
	if (WaveNumber < 1 || Waves.IsEmpty())
	{
		return false;
	}

	// Waves written in the asset are used as they are.
	if (WaveNumber <= Waves.Num())
	{
		OutWave = Waves[WaveNumber - 1];
		return true;
	}

	if (!bEndless)
	{
		return false;
	}

	// Endless: repeat the last wave, larger on each repetition.
	OutWave = Waves.Last();
	const int32 Repetition = WaveNumber - Waves.Num();
	const int32 ExtraEnemies = Repetition * EndlessEnemiesAddedPerWave;

	// A last wave with no groups gets one, for the extra enemies.
	if (OutWave.EnemyGroups.IsEmpty())
	{
		OutWave.EnemyGroups.AddDefaulted();
		OutWave.EnemyGroups[0].Count = 0;
	}
	OutWave.EnemyGroups[0].Count += ExtraEnemies;
	return true;
}
