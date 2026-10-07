// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HKHuman.generated.h"

class UAnimInstance;
class USkeletalMesh;

/**
 * Base class for every character in the game: the player and the enemies.
 *
 * It holds what they have in common:
 *   - Being alive: a pool of health, taking damage and dying. The damage flow lives here once, and
 *     each subclass only says what is specific to it by overriding two hooks, HandleDamaged (called
 *     after a hit the character survives) and HandleDeath (called once, when health reaches zero).
 *   - A humanoid body: a skeletal mesh with an animation Blueprint, shown on the character's mesh
 *     component. The model is optional. Its assets (Epic's mannequins) are not stored in the
 *     repository, so when they are missing the character simply has no body model and subclasses
 *     keep their placeholder shapes.
 *
 * The class is abstract: it cannot be spawned or placed in a level on its own.
 */
UCLASS(Abstract)
class HORDEKILLER_API AHKHuman : public ACharacter
{
	GENERATED_BODY()

public:
	/** Sets the default body model and orients the mesh component for it. */
	AHKHuman();

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

	/** @return True if the humanoid body model was found and is being shown. */
	bool HasBodyModel() const { return bHasBodyModel; }

protected:
	/** Registers with the actor manager, fills health from MaxHealth and applies the body model. */
	virtual void BeginPlay() override;

	/**
	 * Unregisters the character from the actor manager.
	 *
	 * @param EndPlayReason Why play is ending for this actor (destroyed, level change, game exit...).
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Called after the character takes damage and survives. Does nothing by default.
	 *
	 * @param DamageApplied Damage removed from health by this hit.
	 */
	virtual void HandleDamaged(float DamageApplied);

	/** Called once when health reaches zero, after the character has been marked as dead. Does nothing by default. */
	virtual void HandleDeath();

	/** Restores full health and clears the dead state, so the character can be used again. */
	void Revive();

	/**
	 * Turns the body model into a ragdoll: its bones stop following the animation and are simulated by
	 * the physics engine, using the model's physics asset. Has no effect if there is no body model.
	 *
	 * @param Impulse Velocity change, in cm/s, given to the whole body as the ragdoll starts.
	 */
	void StartRagdoll(const FVector& Impulse);

	/** Ends the ragdoll: stops the simulation and puts the body back on the capsule, following the animation. */
	void StopRagdoll();

	/** @return The actor that dealt the most recent damage, or nullptr if it no longer exists. */
	AActor* GetLastDamageCauser() const { return LastDamageCauser.Get(); }

	/**
	 * Tints the body model. Has no effect if there is no body model.
	 *
	 * @param Color Colour applied to the paint of every material of the body.
	 */
	void SetBodyTint(const FLinearColor& Color);

private:
	/**
	 * Loads BodyModel and BodyAnimClass and puts them on the mesh component, with its feet at the bottom
	 * of the capsule.
	 *
	 * @return True if the model was found and applied.
	 */
	bool ApplyBodyModel();

	/** Attaches the mesh component to the capsule with its feet at the bottom, facing forward. */
	void PlaceBodyOnCapsule();

protected:
	/** Health the character starts with. Subclasses set their own default in their constructor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

	/**
	 * Humanoid model shown as the character's body. A soft reference: the asset is only loaded when play
	 * begins, and it may be missing. Leave empty to have no body model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	TSoftObjectPtr<USkeletalMesh> BodyModel;

	/** Animation Blueprint that drives the body model. It must be made for the model's skeleton. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	TSoftClassPtr<UAnimInstance> BodyAnimClass;

private:
	/** The actor that dealt the most recent damage. Weak, because projectiles destroy themselves on impact. */
	TWeakObjectPtr<AActor> LastDamageCauser;

	/** Current health. Set from MaxHealth in BeginPlay. */
	float Health = 0.f;

	/** True once health has reached zero. Blocks further damage. */
	bool bDead = false;

	/** True if the body model was found and applied in BeginPlay. */
	bool bHasBodyModel = false;
};
