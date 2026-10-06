// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HordeKillerProjectile.generated.h"

class UStaticMeshComponent;

/**
 * Physics-driven projectile fired by the player's weapon.
 *
 * The projectile is a simulated rigid body, not a scripted mover: it is given a single impulse when it
 * is fired and from then on the physics engine owns its motion. That means gravity bends its path, it
 * bounces off the floor and walls, and it can pile up on the ground once it has lost its speed.
 *
 * It damages the first enemy it hits while still travelling fast enough, pushes that enemy back and
 * then destroys itself. Projectiles that never hit an enemy are removed when their life span expires.
 */
UCLASS()
class HORDEKILLER_API AHordeKillerProjectile : public AActor
{
	GENERATED_BODY()

public:
	/** Sets up the simulated sphere, its collision and the hit callback. */
	AHordeKillerProjectile();

	/**
	 * Fires the projectile by applying a one-off physics impulse.
	 *
	 * @param Direction World-space direction to fire in. It does not need to be normalised.
	 */
	void Launch(const FVector& Direction);

	/**
	 * Records the current physics velocity every frame.
	 *
	 * @param DeltaSeconds Time elapsed since the previous frame, in seconds.
	 */
	virtual void Tick(float DeltaSeconds) override;

protected:
	/**
	 * Called by the physics engine when the sphere collides with something that blocks it.
	 *
	 * @param HitComponent  The projectile's own component that was hit (always Mesh).
	 * @param OtherActor    The actor that was hit; damage is only dealt if it is an enemy.
	 * @param OtherComp     The component of OtherActor that was hit.
	 * @param NormalImpulse Impulse the physics solver applied to resolve the collision.
	 * @param Hit           Full description of the contact (location, normal, and so on).
	 */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	/** Simulated sphere. It is the root component and serves as both the visual and the collision body. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Speed the projectile leaves the weapon at, in cm/s. Applied as a velocity change, so mass does not affect it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float LaunchSpeed = 5000.f;

	/** Damage dealt to an enemy on impact. Enemies have 2 health by default, so 1 means two shots to kill. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float Damage = 1.f;

	/** Minimum speed at impact, in cm/s, for the projectile to deal damage. Slower projectiles are considered spent. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float MinDamageSpeed = 600.f;

	/** Horizontal speed given to the enemy that gets hit, in cm/s. A fraction of it is also applied upwards. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float KnockbackStrength = 350.f;

private:
	/**
	 * Velocity of the sphere as of the last frame, in cm/s.
	 *
	 * OnHit runs after the physics solver has already resolved the collision, so by then the body's real
	 * velocity is the post-bounce one. This cached value is what tells us how fast, and in which
	 * direction, the projectile was travelling when it struck.
	 */
	FVector PreHitVelocity = FVector::ZeroVector;
};
