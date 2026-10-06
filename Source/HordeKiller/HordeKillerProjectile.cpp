#include "HordeKillerProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "HordeKillerEnemy.h"
#include "UObject/ConstructorHelpers.h"

AHordeKillerProjectile::AHordeKillerProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 5.f;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}
	Mesh->SetWorldScale3D(FVector(0.2f));

	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetSimulatePhysics(true);
	Mesh->SetNotifyRigidBodyCollision(true);
	Mesh->BodyInstance.SetMassOverride(2.f);
	// Fast and small: continuous collision detection avoids tunnelling through enemies.
	Mesh->BodyInstance.bUseCCD = true;
	Mesh->OnComponentHit.AddDynamic(this, &AHordeKillerProjectile::OnHit);
}

void AHordeKillerProjectile::Launch(const FVector& Direction)
{
	const FVector LaunchVelocity = Direction.GetSafeNormal() * LaunchSpeed;
	Mesh->AddImpulse(LaunchVelocity, NAME_None, true);
	PreHitVelocity = LaunchVelocity;
}

void AHordeKillerProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PreHitVelocity = Mesh->GetPhysicsLinearVelocity();
}

void AHordeKillerProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	AHordeKillerEnemy* Enemy = Cast<AHordeKillerEnemy>(OtherActor);
	if (!Enemy || PreHitVelocity.Size() < MinDamageSpeed)
	{
		// Anything else (floor, walls, spent shots) is left to the physics simulation.
		return;
	}

	FVector Knockback = PreHitVelocity.GetSafeNormal2D() * KnockbackStrength;
	Knockback.Z = KnockbackStrength * 0.3f;
	Enemy->LaunchCharacter(Knockback, true, true);

	UGameplayStatics::ApplyDamage(Enemy, Damage, GetInstigatorController(), this, nullptr);
	Destroy();
}
