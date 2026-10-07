// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Characters/HKEnemy.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Characters/HKCharacter.h"
#include "Hordes/HKHordeGenerator.h"
#include "UObject/ConstructorHelpers.h"

AHKEnemy::AHKEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Spawned enemies need an AI controller, or their movement input is never consumed.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);

	// Visual only; collisions use the capsule. Scaled to fill it.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);

	// Keeps the horde from collapsing into a single queue behind the player.
	Movement->bUseRVOAvoidance = true;
	Movement->AvoidanceConsiderationRadius = 150.f;

	bUseControllerRotationYaw = false;

	// Two projectile hits at the default damage of 1.
	MaxHealth = 2.f;
}

void AHKEnemy::SetHordeGenerator(AHKHordeGenerator* InGenerator)
{
	HordeGenerator = InGenerator;
}

void AHKEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Read here so that values edited in a Blueprint apply.
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	// With a body model the placeholder cylinder is not needed.
	if (HasBodyModel())
	{
		BodyMesh->SetVisibility(false);
		SetBodyTint(HealthyColor);
		return;
	}

	// Per-instance material, so this enemy can change colour alone.
	BodyMaterial = BodyMesh->CreateDynamicMaterialInstance(0);
	if (BodyMaterial)
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), HealthyColor);
	}
}

void AHKEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsDead())
	{
		return;
	}

	const AHKCharacter* Player = Cast<AHKCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player || Player->IsDead())
	{
		return;
	}

	// Direct steering, no NavMesh: fine in an open arena.
	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const float Distance = ToPlayer.Size();

	// Stop a little inside attack range, so the enemy does not shove the player.
	if (Distance > AttackRange * 0.8f)
	{
		AddMovementInput(ToPlayer.GetSafeNormal());
	}

	// Melee: being in range deals damage, limited by the cooldown.
	const float Now = GetWorld()->GetTimeSeconds();
	if (Distance <= AttackRange && Now - LastAttackTime >= AttackCooldown)
	{
		LastAttackTime = Now;
		UGameplayStatics::ApplyDamage(const_cast<AHKCharacter*>(Player), AttackDamage, GetController(), this, nullptr);
	}
}

void AHKEnemy::HandleDamaged(float DamageApplied)
{
	Super::HandleDamaged(DamageApplied);

	SetBodyTint(WoundedColor);
	if (BodyMaterial)
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), WoundedColor);
	}
}

void AHKEnemy::HandleDeath()
{
	Super::HandleDeath();

	// Enemies placed by hand have no generator to report to.
	if (AHKHordeGenerator* Generator = HordeGenerator.Get())
	{
		Generator->NotifyEnemyKilled(this);
	}

	Destroy();
}
