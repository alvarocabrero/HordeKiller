// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Managers/HKActorManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

// Storage for the static member declared in the header.
TWeakObjectPtr<UHKActorManager> UHKActorManager::Instance;

UHKActorManager* UHKActorManager::Get()
{
	// Get() on a weak pointer returns nullptr once the object it pointed to has been destroyed.
	return Instance.Get();
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

	// From now on the static functions refer to this world's manager. A level restart creates a new
	// world, and with it a new manager that takes over here.
	Instance = this;

	if (UWorld* World = GetWorld())
	{
		// The world announces every actor it spawns and destroys. Subscribing here, before any gameplay
		// actor exists, means nothing spawned during the match is missed.
		ActorSpawnedHandle = World->AddOnActorSpawnedHandler(
			FOnActorSpawned::FDelegate::CreateUObject(this, &UHKActorManager::HandleActorSpawned));
		ActorDestroyedHandle = World->AddOnActorDestroyedHandler(
			FOnActorDestroyed::FDelegate::CreateUObject(this, &UHKActorManager::HandleActorDestroyed));
	}
}

void UHKActorManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Actors saved in the map are loaded with it, not spawned, so the spawn notification never fires
	// for them. This pass picks them up. AddUnique skips the ones that were spawned before play began
	// (game mode, player controller, player) and are therefore registered already.
	for (TActorIterator<AActor> It(&InWorld); It; ++It)
	{
		Actors.AddUnique(*It);
	}
}

void UHKActorManager::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
		World->RemoveOnActorDestroyedHandler(ActorDestroyedHandle);
	}

	Actors.Empty();

	// Only clear the static instance if it is still this manager. During a level change the new world's
	// manager may already have replaced it, and that one must not be discarded.
	if (Instance.Get() == this)
	{
		Instance.Reset();
	}

	Super::Deinitialize();
}

void UHKActorManager::HandleActorSpawned(AActor* Actor)
{
	if (Actor)
	{
		Actors.AddUnique(Actor);
	}
}

void UHKActorManager::HandleActorDestroyed(AActor* Actor)
{
	// Keeps the remaining actors in registration order, which GetFirstActor relies on to return the oldest.
	Actors.Remove(Actor);
}
