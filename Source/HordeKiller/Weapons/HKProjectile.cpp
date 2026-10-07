// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Weapons/HKProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/HKEnemy.h"
#include "Managers/HKActorManager.h"
#include "UObject/ConstructorHelpers.h"

AHKProjectile::AHKProjectile()
{
	// Ticks only to sample the velocity (see PreHitVelocity).
	PrimaryActorTick.bCanEverTick = true;

	// Missed shots are removed after this many seconds.
	InitialLifeSpan = 5.f;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}
	Mesh->SetWorldScale3D(FVector(0.2f)); // 20 cm ball.

	// Simulated body that blocks everything.
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetSimulatePhysics(true);

	// Required for OnHit to fire on a simulated body.
	Mesh->SetNotifyRigidBodyCollision(true);

	// Set on the body instance: the component setter needs physics state the CDO does not have.
	Mesh->BodyInstance.SetMassOverride(2.f);

	// Small and fast: CCD stops it tunnelling through enemies.
	Mesh->BodyInstance.bUseCCD = true;

	Mesh->OnComponentHit.AddDynamic(this, &AHKProjectile::OnHit);
}

void AHKProjectile::BeginPlay()
{
	Super::BeginPlay();

	UHKActorManager::Register(this);
}

void AHKProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHKActorManager::Unregister(this);

	Super::EndPlay(EndPlayReason);
}

void AHKProjectile::Launch(const FVector& Direction)
{
	const FVector LaunchVelocity = Direction.GetSafeNormal() * LaunchSpeed;

	// bVelChange = true: a direct change in velocity, ignoring mass.
	Mesh->AddImpulse(LaunchVelocity, NAME_None, true);

	// Covers a hit on the first frame, before Tick has run.
	PreHitVelocity = LaunchVelocity;
}

void AHKProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PreHitVelocity = Mesh->GetPhysicsLinearVelocity();
}

void AHKProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// Only living enemies take damage, and only from a fast projectile; the rest, corpses
	// included, is left to physics.
	AHKEnemy* Enemy = Cast<AHKEnemy>(OtherActor);
	if (!Enemy || Enemy->IsDead() || PreHitVelocity.Size() < MinDamageSpeed)
	{
		return;
	}

	// Enemies are not simulated bodies, so the knockback goes through their movement component.
	FVector Knockback = PreHitVelocity.GetSafeNormal2D() * KnockbackStrength;
	Knockback.Z = KnockbackStrength * 0.3f;
	Enemy->LaunchCharacter(Knockback, true, true);

	UGameplayStatics::ApplyDamage(Enemy, Damage, GetInstigatorController(), this, nullptr);

	Destroy();
}
