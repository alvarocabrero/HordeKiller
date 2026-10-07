// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "Characters/HKHuman.h"
#include "HKEnemy.generated.h"

class AHKHordeGenerator;
class UMaterialInstanceDynamic;
class UPhysicsControlComponent;
class UStaticMeshComponent;

/**
 * Horde enemy.
 *
 * Runs in a straight line towards the player, hits them when close enough and dies after taking
 * MaxHealth points of damage (two projectile hits with the default values). Its colour changes after
 * the first hit so the player can tell wounded enemies apart.
 *
 * When it dies its body becomes a ragdoll: physics takes over and it falls, pushed by the shot that
 * killed it. The corpse stays for CorpseLifetime seconds and is then removed.
 *
 * Enemies are pooled. They are not destroyed when their corpse is removed: they are handed back to the
 * horde generator's pool, switched off, and switched on again for a later wave. ActivateFromPool and
 * DeactivateToPool do that switching, and between them an enemy must be fully reset.
 *
 * The enemy does not use the navigation system: it steers directly at the player every frame, which
 * is enough for an open arena without obstacles.
 *
 * Health, taking damage, the dead state and the ragdoll itself are inherited from AHKHuman.
 */
UCLASS()
class HORDEKILLER_API AHKEnemy : public AHKHuman
{
	GENERATED_BODY()

public:
	/** Builds the placeholder body and configures movement, AI possession and the physics control component. */
	AHKEnemy();

	/**
	 * Tells the enemy which horde generator it belongs to, so it can report its death and return to
	 * that generator's pool.
	 *
	 * @param InGenerator The generator this enemy belongs to.
	 */
	void SetHordeGenerator(AHKHordeGenerator* InGenerator);

	/**
	 * Brings a pooled enemy into play: full health, standing, visible, colliding and chasing.
	 *
	 * @param Location World position to appear at, in cm (the capsule centre).
	 */
	void ActivateFromPool(const FVector& Location);

	/**
	 * Takes the enemy out of play without destroying it: hidden, not colliding, not ticking and moved
	 * out of the way, ready to be activated again.
	 *
	 * @param ParkLocation World position to wait at while inactive, away from the play area.
	 */
	void DeactivateToPool(const FVector& ParkLocation);

	/** @return True between ActivateFromPool and DeactivateToPool, including while it is a corpse. */
	bool IsActiveInPool() const { return bActiveInPool; }

	/**
	 * Chases the player and attacks when in range.
	 *
	 * @param DeltaSeconds Time elapsed since the previous frame, in seconds.
	 */
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Initialises walk speed and the body colour once the enemy is in the world. */
	virtual void BeginPlay() override;

	/**
	 * Switches the body to the wounded colour. Called by AHKHuman after a hit the enemy survives.
	 *
	 * @param DamageApplied Damage removed from health by this hit.
	 */
	virtual void HandleDamaged(float DamageApplied) override;

	/**
	 * Reports the kill, stops the enemy and turns its body into a ragdoll, then schedules the removal
	 * of the corpse. Called once by AHKHuman when health reaches zero.
	 */
	virtual void HandleDeath() override;

	/** Removes the corpse: returns the enemy to its generator's pool, or destroys it if it has none. */
	void RemoveCorpse();

	/** Applies the given colour to whichever body is showing: the humanoid model or the placeholder cylinder. */
	void SetBodyColor(const FLinearColor& Color);

	/** Placeholder body: an engine cylinder scaled to fill the collision capsule. Hidden when the humanoid body model is available. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	/**
	 * Physics Control component for the body, from the engine's PhysicsControl plugin. It can drive
	 * simulated bones towards the animation, for partial ragdolls or physical hit reactions. It is added
	 * for that future use and has no controls set up yet: the death ragdoll does not go through it.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UPhysicsControlComponent> PhysicsControl;

	/** Running speed, in cm/s. The player moves at 600 cm/s by default, so slower values can be outrun. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MoveSpeed = 380.f;

	/** Damage dealt to the player by each attack. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackDamage = 10.f;

	/** Maximum horizontal distance to the player, in cm and measured between actor centres, at which an attack lands. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackRange = 110.f;

	/** Minimum time between two attacks by this enemy, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackCooldown = 1.f;

	/** Body colour while at full health. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FLinearColor HealthyColor = FLinearColor(0.8f, 0.05f, 0.05f);

	/** Body colour after taking damage without dying. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FLinearColor WoundedColor = FLinearColor(1.f, 0.55f, 0.f);

	/** Time the corpse stays in the level after death before it is removed, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Death", meta = (ClampMin = "0", Units = "s"))
	float CorpseLifetime = 10.f;

	/** Speed given to the ragdoll at the moment of death, in cm/s, away from whatever killed the enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Death", meta = (ClampMin = "0", Units = "cm/s"))
	float DeathImpulse = 500.f;

private:
	/**
	 * The horde generator this enemy belongs to, if any. A weak pointer: the enemy does not keep the
	 * generator alive. The actor's Owner cannot be used for this, because possession by the AI
	 * controller replaces the owner with that controller.
	 */
	TWeakObjectPtr<AHKHordeGenerator> HordeGenerator;

	/** World time of the last attack, in seconds. Starts far in the past so the first attack is immediate. */
	float LastAttackTime = -1000.f;

	/** True between ActivateFromPool and DeactivateToPool. */
	bool bActiveInPool = false;

	/** Timer that removes the corpse after CorpseLifetime. */
	FTimerHandle CorpseTimer;

	/** Per-instance material of the placeholder cylinder. UPROPERTY keeps it from being garbage collected. */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;
};
