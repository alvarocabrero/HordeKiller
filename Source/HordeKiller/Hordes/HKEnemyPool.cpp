// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Hordes/HKEnemyPool.h"
#include "Characters/HKEnemy.h"
#include "Engine/World.h"
#include "Hordes/HKHordeGenerator.h"

DEFINE_LOG_CATEGORY_STATIC(LogHKEnemyPool, Log, All);

UHKEnemyPool::UHKEnemyPool()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVector UHKEnemyPool::GetParkLocation() const
{
	const AActor* Owner = GetOwner();
	return (Owner ? Owner->GetActorLocation() : FVector::ZeroVector) - FVector(0.f, 0.f, ParkDepth);
}

AHKEnemy* UHKEnemyPool::CreateEnemy(TSubclassOf<AHKEnemy> EnemyClass)
{
	UWorld* World = GetWorld();
	if (!World || !EnemyClass)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector ParkLocation = GetParkLocation();
	AHKEnemy* Enemy = World->SpawnActor<AHKEnemy>(EnemyClass, ParkLocation, FRotator::ZeroRotator, Params);
	if (!Enemy)
	{
		return nullptr;
	}

	Enemy->SetHordeGenerator(Cast<AHKHordeGenerator>(GetOwner()));
	Enemy->DeactivateToPool(ParkLocation);

	AllEnemies.Add(Enemy);
	Available.Add(Enemy);
	return Enemy;
}

void UHKEnemyPool::EnsureCapacity(TSubclassOf<AHKEnemy> EnemyClass, int32 Count)
{
	if (!EnemyClass)
	{
		return;
	}

	int32 Owned = 0;
	for (const AHKEnemy* Enemy : AllEnemies)
	{
		if (Enemy && Enemy->GetClass() == EnemyClass)
		{
			++Owned;
		}
	}

	const int32 Missing = Count - Owned;
	for (int32 Index = 0; Index < Missing; ++Index)
	{
		CreateEnemy(EnemyClass);
	}

	if (Missing > 0)
	{
		UE_LOG(LogHKEnemyPool, Log, TEXT("Created %d %s (pool now owns %d of that class, %d in total)"),
			Missing, *EnemyClass->GetName(), Owned + Missing, AllEnemies.Num());
	}
}

AHKEnemy* UHKEnemyPool::Acquire(TSubclassOf<AHKEnemy> EnemyClass, const FVector& Location)
{
	AHKEnemy* Enemy = nullptr;

	// From the end, so removal is cheap.
	for (int32 Index = Available.Num() - 1; Index >= 0; --Index)
	{
		if (Available[Index] && Available[Index]->GetClass() == EnemyClass)
		{
			Enemy = Available[Index];
			Available.RemoveAtSwap(Index);
			break;
		}
	}

	// Ran dry: create one now, so the wave is not left short.
	if (!Enemy)
	{
		UE_LOG(LogHKEnemyPool, Warning, TEXT("No %s available; creating one during play"), *GetNameSafe(EnemyClass));
		Enemy = CreateEnemy(EnemyClass);
		if (!Enemy)
		{
			return nullptr;
		}
		Available.Remove(Enemy);
	}

	++AcquiredCount;
	Enemy->ActivateFromPool(Location);
	return Enemy;
}

void UHKEnemyPool::Release(AHKEnemy* Enemy)
{
	if (!Enemy || !Enemy->IsActiveInPool() || !AllEnemies.Contains(Enemy))
	{
		return;
	}

	Enemy->DeactivateToPool(GetParkLocation());
	Available.Add(Enemy);
}
