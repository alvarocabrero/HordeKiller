// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Managers/HKActorManager.h"
#include "GameFramework/Actor.h"

// Storage for the static member declared in the header.
TWeakObjectPtr<UHKActorManager> UHKActorManager::Instance;

UHKActorManager* UHKActorManager::Get()
{
	// Get() on a weak pointer returns nullptr once the object it pointed to has been destroyed.
	return Instance.Get();
}

void UHKActorManager::Register(AActor* Actor)
{
	UHKActorManager* Manager = Get();
	if (Manager && Actor)
	{
		// AddUnique makes a repeated call harmless, for example if a subclass registers again.
		Manager->Actors.AddUnique(Actor);
	}
}

void UHKActorManager::Unregister(AActor* Actor)
{
	// The manager can already be gone when this is called: at the end of a level, actors end play
	// while the world is being torn down. In that case there is nothing left to remove from.
	if (UHKActorManager* Manager = Get())
	{
		// Remove keeps the remaining actors in registration order, which GetFirstActor relies on.
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
	// The editor creates several worlds of its own (the level being edited, asset previews). A manager
	// in each of those would compete for the static instance, so only worlds running a game get one.
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHKActorManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// From now on the static functions refer to this world's manager. This happens while the world is
	// being set up, before any actor begins play, so every actor finds the manager ready when it
	// registers. A level restart creates a new world, and with it a new manager that takes over here.
	Instance = this;
}

void UHKActorManager::Deinitialize()
{
	Actors.Empty();

	// Only clear the static instance if it is still this manager. During a level change the new world's
	// manager may already have replaced it, and that one must not be discarded.
	if (Instance.Get() == this)
	{
		Instance.Reset();
	}

	Super::Deinitialize();
}
