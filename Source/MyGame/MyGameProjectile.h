#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyGameProjectile.generated.h"

class UStaticMeshComponent;

/** Physics-driven projectile: a simulated rigid body launched with an impulse. */
UCLASS()
class MYGAME_API AMyGameProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMyGameProjectile();

	/** Launches the projectile along Direction using a physics impulse. */
	void Launch(const FVector& Direction);

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	// Simulated sphere; it is both the visual and the collision body.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// Launch speed in cm/s, applied as a velocity-change impulse.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float LaunchSpeed = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float Damage = 1.f;

	// Below this speed (cm/s) the projectile is considered spent and deals no damage.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float MinDamageSpeed = 600.f;

	// Horizontal knockback applied to the enemy that gets hit.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float KnockbackStrength = 350.f;

private:
	// Velocity sampled every tick, because OnHit runs after the physics solver has already changed it.
	FVector PreHitVelocity = FVector::ZeroVector;

public:
	virtual void Tick(float DeltaSeconds) override;
};
