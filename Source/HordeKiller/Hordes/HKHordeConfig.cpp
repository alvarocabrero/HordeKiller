// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Hordes/HKHordeConfig.h"
#include "Characters/HKEnemy.h"

int32 FHKWaveConfig::GetEnemyCount() const
{
	int32 Total = 0;
	for (const FHKWaveEnemyGroup& Group : EnemyGroups)
	{
		// Negative counts cannot be entered in the editor, but values set from code are not clamped.
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

	// Waves that are written in the asset are used exactly as they are. Wave numbers start at 1 and
	// array indices at 0, hence the offset.
	if (WaveNumber <= Waves.Num())
	{
		OutWave = Waves[WaveNumber - 1];
		return true;
	}

	// Past the end of the list the horde is over, unless it is endless.
	if (!bEndless)
	{
		return false;
	}

	// Endless mode: repeat the last wave, larger each time. Repetition 1 is the first wave after the
	// list, so it gets the extra amount once, repetition 2 twice, and so on.
	OutWave = Waves.Last();
	const int32 Repetition = WaveNumber - Waves.Num();
	const int32 ExtraEnemies = Repetition * EndlessEnemiesAddedPerWave;

	if (OutWave.EnemyGroups.IsEmpty())
	{
		// A last wave with no groups has nothing to grow from; give the extra enemies a group of their
		// own, which spawns the basic enemy class.
		OutWave.EnemyGroups.AddDefaulted();
		OutWave.EnemyGroups[0].Count = 0;
	}
	OutWave.EnemyGroups[0].Count += ExtraEnemies;
	return true;
}
