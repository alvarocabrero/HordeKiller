// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HKActorManager.generated.h"

/**
 * Registry of the actors that take part in the current match.
 *
 * Actors subscribe themselves: each of the project's actor classes calls Register on the manager when
 * it begins play and Unregister when it ends play. The manager does not look for actors on its own,
 * so it holds exactly the actors that chose to subscribe: the player, the enemies, the projectiles,
 * the game mode and the HUD. Engine actors that have no code of ours, such as lights or the arena
 * blocks, are not in it.
 *
 * Everything is reached through static functions, from any class and without a pointer to the manager:
 *
 *     UHKActorManager::Register(this);       // in the actor's BeginPlay
 *     UHKActorManager::Unregister(this);     // in the actor's EndPlay
 *
 *     TArray<AHKEnemy*> Enemies = UHKActorManager::GetActors<AHKEnemy>();
 *     AHKCharacter* Player = UHKActorManager::GetFirstActor<AHKCharacter>();
 *     int32 Total = UHKActorManager::GetActorCount();
 *
 * It is a world subsystem: the engine creates one together with the game world and destroys it with
 * it, so the registry starts empty on every level load or restart. The static functions refer to the
 * manager of the game world that is currently running. Outside a running game (for example in the
 * editor before pressing Play) there is no manager: queries return empty results and Register and
 * Unregister do nothing.
 *
 * Queries walk the whole list, so their cost grows with the number of actors registered. That is fine
 * for occasional use; code that needs a result every frame should keep it instead of asking again.
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
	 * Adds an actor to the registry. Meant to be called by the actor itself from BeginPlay.
	 * Registering an actor that is already registered has no effect.
	 *
	 * @param Actor The actor subscribing itself. Ignored if null.
	 */
	static void Register(AActor* Actor);

	/**
	 * Removes an actor from the registry. Meant to be called by the actor itself from EndPlay, which
	 * the engine runs both when the actor is destroyed and when the level ends.
	 * Unregistering an actor that is not registered has no effect.
	 *
	 * @param Actor The actor unsubscribing itself.
	 */
	static void Unregister(AActor* Actor);

	/**
	 * @return A copy of the list of all registered actors, in the order they registered. Empty if no game is running.
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
	 * @return The matching actors, in the order they registered. Empty if there are none.
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
	 * @return The matching actor that registered earliest, or nullptr if there is none.
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
	 * Called by the engine when the world is created. Makes this manager the one the static functions use.
	 *
	 * @param Collection Other subsystems of the same world, for declaring dependencies. Not used here.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called by the engine when the world is destroyed. Empties the registry and releases the static instance. */
	virtual void Deinitialize() override;

protected:
	/**
	 * Limits the manager to worlds where a game is being played.
	 *
	 * @param WorldType Kind of world the engine is about to create a subsystem for.
	 * @return True for the standalone game and for Play In Editor; false for the editor's own worlds.
	 */
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	/** Every registered actor, in registration order. UPROPERTY makes the references visible to the garbage collector. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;

	/**
	 * The manager of the running game world, which is what makes static access possible.
	 * A weak pointer, so it never keeps a finished world's manager alive.
	 */
	static TWeakObjectPtr<UHKActorManager> Instance;
};
