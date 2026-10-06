// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Weapons/HKProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/HKEnemy.h"
#include "UObject/ConstructorHelpers.h"

AHKProjectile::AHKProjectile()
{
	// Ticking is needed only to sample the velocity each frame (see PreHitVelocity).
	PrimaryActorTick.bCanEverTick = true;

	// Safety net: a projectile that never hits an enemy is destroyed after this many seconds, so missed
	// shots do not accumulate forever.
	InitialLifeSpan = 5.f;

	// The mesh is the root so that the actor's transform follows the simulated body directly.
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// Engine-provided sphere, so the project needs no art assets of its own. FObjectFinder may only be
	// used inside constructors; "static" makes the lookup happen once for the whole class.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	// The basic sphere is 100 cm across; 0.2 makes it a 20 cm ball.
	Mesh->SetWorldScale3D(FVector(0.2f));

	// "PhysicsActor" is the engine's preset for simulated bodies: object type PhysicsBody, blocking
	// everything, including pawns (enemies) and static geometry (the arena).
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	// Hand the body over to the physics engine. From here on gravity and collisions drive its motion.
	Mesh->SetSimulatePhysics(true);

	// Simulated bodies do not raise hit events unless this is enabled ("Simulation Generates Hit
	// Events" in the editor). Without it OnHit would never be called.
	Mesh->SetNotifyRigidBodyCollision(true);

	// Fixed 2 kg mass instead of one derived from the mesh volume. This is written straight to the body
	// instance because the component-level setter touches the physics state, which does not exist yet
	// while the class default object is being constructed.
	Mesh->BodyInstance.SetMassOverride(2.f);

	// The ball is small and fast: at 5000 cm/s it covers more than its own diameter in a single frame.
	// Continuous collision detection sweeps the body between frames so it cannot tunnel through enemies.
	Mesh->BodyInstance.bUseCCD = true;

	Mesh->OnComponentHit.AddDynamic(this, &AHKProjectile::OnHit);
}

void AHKProjectile::Launch(const FVector& Direction)
{
	const FVector LaunchVelocity = Direction.GetSafeNormal() * LaunchSpeed;

	// The last argument (bVelChange) makes the impulse a direct change in velocity, ignoring mass. The
	// body starts at rest, so its velocity right after this call is exactly LaunchVelocity.
	Mesh->AddImpulse(LaunchVelocity, NAME_None, true);

	// Seed the cache so that a hit on the very first frame, before Tick has run, still sees a valid speed.
	PreHitVelocity = LaunchVelocity;
}

void AHKProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Remember how the body was moving before the next physics step can change it.
	PreHitVelocity = Mesh->GetPhysicsLinearVelocity();
}

void AHKProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// Only enemies take damage, and only from a projectile that is still moving fast. The speed check
	// stops spent balls lying on the floor from hurting enemies that walk into them.
	AHKEnemy* Enemy = Cast<AHKEnemy>(OtherActor);
	if (!Enemy || PreHitVelocity.Size() < MinDamageSpeed)
	{
		// Everything else (floor, walls, the player, slow impacts) is left to the physics simulation,
		// which is what makes the projectile bounce and roll.
		return;
	}

	// Push the enemy away along the projectile's horizontal direction of travel, with a small lift so it
	// briefly leaves the ground. Enemies are characters, not simulated bodies, so the physical impact
	// alone would not move them; LaunchCharacter does it through their movement component. The two
	// "true" arguments replace the enemy's current velocity instead of adding to it.
	FVector Knockback = PreHitVelocity.GetSafeNormal2D() * KnockbackStrength;
	Knockback.Z = KnockbackStrength * 0.3f;
	Enemy->LaunchCharacter(Knockback, true, true);

	// Standard engine damage path: this ends up in AHKEnemy::TakeDamage. The instigator
	// controller is the player's, taken from the Instigator set when the projectile was spawned.
	UGameplayStatics::ApplyDamage(Enemy, Damage, GetInstigatorController(), this, nullptr);

	// A projectile hits one enemy only.
	Destroy();
}
