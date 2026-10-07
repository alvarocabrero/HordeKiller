// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Managers/HKActorManager.h"
#include "GameFramework/Actor.h"

TWeakObjectPtr<UHKActorManager> UHKActorManager::Instance;

UHKActorManager* UHKActorManager::Get()
{
	return Instance.Get();
}

void UHKActorManager::Register(AActor* Actor)
{
	UHKActorManager* Manager = Get();
	if (Manager && Actor)
	{
		Manager->Actors.AddUnique(Actor);
	}
}

void UHKActorManager::Unregister(AActor* Actor)
{
	// The manager may already be gone when a level is being torn down.
	if (UHKActorManager* Manager = Get())
	{
		Manager->Actors.Remove(Actor);
	}
}

TArray<AActor*> UHKActorManager::GetAllActors()
{
	TArray<AActor*> Result;
	if (const UHKActorManager* Manager = Get())
	{
		Result.Reserve(Manager->Actors.Num());
		for (AActor* Actor : Manager->Actors)
		{
			Result.Add(Actor);
		}
	}
	return Result;
}

int32 UHKActorManager::GetActorCount()
{
	const UHKActorManager* Manager = Get();
	return Manager ? Manager->Actors.Num() : 0;
}

bool UHKActorManager::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// Game worlds only; the editor's own worlds would compete for the static instance.
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHKActorManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Runs before any actor begins play, so the manager is ready when actors register.
	Instance = this;
}

void UHKActorManager::Deinitialize()
{
	Actors.Empty();

	// On a level change the new world's manager may already have taken over.
	if (Instance.Get() == this)
	{
		Instance.Reset();
	}

	Super::Deinitialize();
}
