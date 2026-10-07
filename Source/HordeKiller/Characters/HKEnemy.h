// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "Characters/HKHuman.h"
#include "HKEnemy.generated.h"

class AHKHordeGenerator;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

/**
 * Horde enemy.
 *
 * Runs in a straight line towards the player, hits them when close enough and dies after taking
 * MaxHealth points of damage (two projectile hits with the default values). Its colour changes after
 * the first hit so the player can tell wounded enemies apart.
 *
 * The enemy does not use the navigation system: it steers directly at the player every frame, which
 * is enough for an open arena without obstacles.
 *
 * Health, taking damage and the dead state are inherited from AHKHuman. This class adds the reaction to
 * a hit (the colour change) and what dying means for an enemy (counting the kill and disappearing).
 */
UCLASS()
class HORDEKILLER_API AHKEnemy : public AHKHuman
{
	GENERATED_BODY()

public:
	/** Builds the placeholder body and configures movement and AI possession. */
	AHKEnemy();

	/**
	 * Tells the enemy which horde generator spawned it, so it can report its death to that generator.
	 * Called by the generator right after spawning the enemy.
	 *
	 * @param InGenerator The generator this enemy belongs to.
	 */
	void SetHordeGenerator(AHKHordeGenerator* InGenerator);

	/**
	 * Chases the player and attacks when in range.
	 *
	 * @param DeltaSeconds Time elapsed since the previous frame, in seconds.
	 */
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Initialises walk speed and the body material once the enemy is in the world. */
	virtual void BeginPlay() override;

	/**
	 * Switches the body to the wounded colour. Called by AHKHuman after a hit the enemy survives.
	 *
	 * @param DamageApplied Damage removed from health by this hit.
	 */
	virtual void HandleDamaged(float DamageApplied) override;

	/** Reports the kill to the game mode and removes the enemy from the world. Called once by AHKHuman when health reaches zero. */
	virtual void HandleDeath() override;

	/** Placeholder body: an engine cylinder scaled to fill the collision capsule. Hidden when the humanoid body model is available. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

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

private:
	/**
	 * The horde generator that spawned this enemy, if any. A weak pointer: the enemy does not keep the
	 * generator alive. The actor's Owner cannot be used for this, because possession by the AI
	 * controller replaces the owner with that controller.
	 */
	TWeakObjectPtr<AHKHordeGenerator> HordeGenerator;

	/** World time of the last attack, in seconds. Starts far in the past so the first attack is immediate. */
	float LastAttackTime = -1000.f;

	/** Per-instance material used to recolour this enemy alone. UPROPERTY keeps it from being garbage collected. */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;
};
