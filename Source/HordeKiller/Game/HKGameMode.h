// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HKGameMode.generated.h"

class AHKHordeGenerator;
class UHKHordeConfig;

/**
 * Rules of the game.
 *
 * At the start of play it makes sure the level has a horde generator, which is the actor that spawns
 * the waves of enemies. It also handles the end of the game: when the player dies it stops the horde
 * and restarts the level.
 *
 * The level itself (floor, walls, lighting, player start) is not created here: it is a map asset, such
 * as L_HKArena. The waves are not configured here either. They belong to AHKHordeGenerator and to the
 * UHKHordeConfig data asset it reads. If the level contains a generator, that one is used as it is;
 * otherwise the game mode spawns one with DefaultHordeConfig.
 *
 * It selects the player character and HUD classes too, so no Blueprint game mode is required. For the
 * player it uses the Blueprint BP_HKCharacter when it exists, and falls back to the C++ class otherwise.
 */
UCLASS()
class HORDEKILLER_API AHKGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Selects the default pawn and HUD classes and the default horde config. */
	AHKGameMode();

	/** Called by the player character when it dies. Ends the game, stops the horde and schedules a level restart. */
	void NotifyPlayerDied();

	/** @return The generator running this level's horde, or nullptr before play begins. */
	AHKHordeGenerator* GetHordeGenerator() const { return HordeGenerator; }

	/** @return True once the player has died. */
	bool IsGameOver() const { return bGameOver; }

protected:
	/** Finds or creates the horde generator. */
	virtual void BeginPlay() override;

	/**
	 * Unregisters the game mode from the actor manager.
	 *
	 * @param EndPlayReason Why play is ending (level change, game exit...).
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Uses the horde generator placed in the level, or spawns one with DefaultHordeConfig if there is
	 * none. A generator spawned this way sits at the world origin and has no spawn area limit, so levels
	 * meant to be played should place their own.
	 */
	void SetUpHordeGenerator();

	/** Reloads the current level, which resets everything to its initial state. */
	void RestartLevel();

	/**
	 * Horde used in levels that have no horde generator of their own. Defaults to the asset
	 * DA_HKHorde_Default. To give a level a different horde, place a horde generator in it and assign
	 * its config there; this property is then ignored for that level.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde")
	TObjectPtr<UHKHordeConfig> DefaultHordeConfig;

	/** Time between the player's death and the level restarting, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game", meta = (ClampMin = "0", Units = "s"))
	float RestartDelay = 3.f;

private:
	/** The generator running this level's horde. UPROPERTY keeps the reference valid for the garbage collector. */
	UPROPERTY()
	TObjectPtr<AHKHordeGenerator> HordeGenerator;

	/** True once the player has died. */
	bool bGameOver = false;

	/** Timer that restarts the level after the player dies. */
	FTimerHandle RestartTimer;
};
