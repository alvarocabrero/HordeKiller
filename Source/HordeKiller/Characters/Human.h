// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Human.generated.h"

/**
 * Base class for every character in the game: the player and the enemies.
 *
 * It holds what they have in common, which is being alive: a pool of health, taking damage and dying.
 * The damage flow lives here once, and each subclass only says what is specific to it by overriding
 * two hooks:
 *   - HandleDamaged: called after a hit the character survives.
 *   - HandleDeath:   called once, when health reaches zero.
 *
 * The class is abstract: it cannot be spawned or placed in a level on its own.
 */
UCLASS(Abstract)
class HORDEKILLER_API AHuman : public ACharacter
{
	GENERATED_BODY()

public:
	/**
	 * Applies incoming damage and triggers the damaged or death hook.
	 *
	 * Damage is ignored once the character is dead, so HandleDeath runs exactly once.
	 *
	 * @param DamageAmount    Raw damage requested by the caller.
	 * @param DamageEvent     Extra data describing the kind of damage.
	 * @param EventInstigator Controller responsible for the damage.
	 * @param DamageCauser    Actor that dealt the damage.
	 * @return The damage actually applied, or 0 if it was ignored.
	 */
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

	/** @return Current health, between 0 and MaxHealth. */
	float GetHealth() const { return Health; }

	/** @return Health the character starts with. */
	float GetMaxHealth() const { return MaxHealth; }

	/** @return True once health has reached zero. */
	bool IsDead() const { return bDead; }

protected:
	/** Fills health from MaxHealth once the character is in the world. */
	virtual void BeginPlay() override;

	/**
	 * Called after the character takes damage and survives. Does nothing by default.
	 *
	 * @param DamageApplied Damage removed from health by this hit.
	 */
	virtual void HandleDamaged(float DamageApplied);

	/** Called once when health reaches zero, after the character has been marked as dead. Does nothing by default. */
	virtual void HandleDeath();

	/** Health the character starts with. Subclasses set their own default in their constructor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

private:
	/** Current health. Set from MaxHealth in BeginPlay. */
	float Health = 0.f;

	/** True once health has reached zero. Blocks further damage. */
	bool bDead = false;
};
