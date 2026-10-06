// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HordeKillerEnemy.generated.h"

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
 */
UCLASS()
class HORDEKILLER_API AHordeKillerEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	/** Builds the placeholder body and configures movement and AI possession. */
	AHordeKillerEnemy();

	/**
	 * Chases the player and attacks when in range.
	 *
	 * @param DeltaSeconds Time elapsed since the previous frame, in seconds.
	 */
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * Applies incoming damage, updates the body colour and kills the enemy when its health runs out.
	 *
	 * @param DamageAmount    Raw damage requested by the caller.
	 * @param DamageEvent     Extra data describing the kind of damage.
	 * @param EventInstigator Controller responsible for the damage (the player's).
	 * @param DamageCauser    Actor that dealt the damage (the projectile).
	 * @return The damage actually applied, or 0 if it was ignored.
	 */
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	/** Initialises health, walk speed and the body material once the enemy is in the world. */
	virtual void BeginPlay() override;

	/** Reports the kill to the game mode and removes the enemy from the world. */
	void Die();

	/** Placeholder body: an engine cylinder scaled to fill the collision capsule. It has no collision of its own. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	/** Starting health. Each projectile deals 1 damage by default, so 2 means two shots to kill. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MaxHealth = 2.f;

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
	/** Current health. Set from MaxHealth in BeginPlay. */
	float Health = 0.f;

	/** World time of the last attack, in seconds. Starts far in the past so the first attack is immediate. */
	float LastAttackTime = -1000.f;

	/** True once the enemy has died, so it is never counted as a kill twice. */
	bool bDead = false;

	/** Per-instance material used to recolour this enemy alone. UPROPERTY keeps it from being garbage collected. */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;
};
