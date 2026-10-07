// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HKActorManager.generated.h"

/**
 * Registry of every actor that exists in the current match.
 *
 * The manager keeps a reference to each actor from the moment it is spawned until it is destroyed:
 * the player, the enemies, the projectiles, the arena blocks, and also the engine's own actors such
 * as the game mode, the player controller and the HUD. Actors placed in the level by hand are
 * registered when play begins.
 *
 * Registration is automatic. The manager listens to the world's spawn and destroy notifications, so
 * no actor has to register or unregister itself.
 *
 * Everything is reached through static functions, from any class and without a pointer to the manager:
 *
 *     TArray<AHKEnemy*> Enemies = UHKActorManager::GetActors<AHKEnemy>();
 *     AHKCharacter* Player = UHKActorManager::GetFirstActor<AHKCharacter>();
 *     int32 Total = UHKActorManager::GetActorCount();
 *
 * It is a world subsystem: the engine creates one together with the game world and destroys it with
 * it, so the registry starts empty on every level load or restart. The static functions refer to the
 * manager of the game world that is currently running. Outside a running game (for example in the
 * editor before pressing Play) there is no manager and they return empty results.
 *
 * Queries walk the whole list, so their cost grows with the number of actors alive. That is fine for
 * occasional use; code that needs a result every frame should keep it instead of asking again.
 */
UCLASS()
class HORDEKILLER_API UHKActorManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @return The manager of the running game world, or nullptr if no game is running.
	 */
	static UHKActorManager* Get();

	/**
	 * @return A copy of the list of all registered actors, in the order they were registered. Empty if no game is running.
	 */
	static TArray<AActor*> GetAllActors();

	/**
	 * @return Number of actors currently registered. 0 if no game is running.
	 */
	static int32 GetActorCount();

	/**
	 * Finds every registered actor of a given class, including its subclasses and Blueprints.
	 *
	 * @tparam T Actor class to look for, for example AHKEnemy.
	 * @return The matching actors, in the order they were registered. Empty if there are none.
	 */
	template <typename T>
	static TArray<T*> GetActors()
	{
		TArray<T*> Result;
		if (const UHKActorManager* Manager = Get())
		{
			for (AActor* Actor : Manager->Actors)
			{
				// Cast returns nullptr for actors of other classes, which filters them out.
				if (T* Typed = Cast<T>(Actor))
				{
					Result.Add(Typed);
				}
			}
		}
		return Result;
	}

	/**
	 * Finds the first registered actor of a given class, including its subclasses and Blueprints.
	 * Intended for classes with a single instance, such as the player.
	 *
	 * @tparam T Actor class to look for, for example AHKCharacter.
	 * @return The oldest matching actor, or nullptr if there is none.
	 */
	template <typename T>
	static T* GetFirstActor()
	{
		if (const UHKActorManager* Manager = Get())
		{
			for (AActor* Actor : Manager->Actors)
			{
				if (T* Typed = Cast<T>(Actor))
				{
					return Typed;
				}
			}
		}
		return nullptr;
	}

	/**
	 * Called by the engine when the world is created. Starts listening for spawned and destroyed actors.
	 *
	 * @param Collection Other subsystems of the same world, for declaring dependencies. Not used here.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called by the engine when the world is destroyed. Stops listening and empties the registry. */
	virtual void Deinitialize() override;

	/**
	 * Called by the engine when play begins. Registers the actors that were already in the level.
	 *
	 * @param InWorld The world that is starting play.
	 */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

protected:
	/**
	 * Limits the manager to worlds where a game is being played.
	 *
	 * @param WorldType Kind of world the engine is about to create a subsystem for.
	 * @return True for the standalone game and for Play In Editor; false for the editor's own worlds.
	 */
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	/**
	 * Adds an actor to the registry.
	 *
	 * @param Actor The actor that has just been spawned.
	 */
	void HandleActorSpawned(AActor* Actor);

	/**
	 * Removes an actor from the registry.
	 *
	 * @param Actor The actor that is being destroyed.
	 */
	void HandleActorDestroyed(AActor* Actor);

	/** Every actor alive in this world, in registration order. UPROPERTY makes the references visible to the garbage collector. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;

	/** Handle of the world's "actor spawned" subscription, needed to cancel it. */
	FDelegateHandle ActorSpawnedHandle;

	/** Handle of the world's "actor destroyed" subscription, needed to cancel it. */
	FDelegateHandle ActorDestroyedHandle;

	/**
	 * The manager of the running game world, which is what makes static access possible.
	 * A weak pointer, so it never keeps a finished world's manager alive.
	 */
	static TWeakObjectPtr<UHKActorManager> Instance;
};
