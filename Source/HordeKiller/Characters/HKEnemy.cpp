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

	// By default a character is only given an AI controller when it is placed in a level by hand.
	// Enemies here are spawned at runtime by the game mode, and a character without a controller never
	// consumes its movement input, so it would stand still.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Radius and half-height of the collision capsule, in cm. Slightly slimmer and shorter than the
	// player's (42 x 96) so that groups pack together more tightly.
	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);

	// Visual only: projectiles and the player collide with the capsule, never with this mesh.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// The basic cylinder is 100 cm wide and 100 cm tall. These factors make it 68 cm wide (capsule
	// diameter) and 176 cm tall (capsule height).
	BodyMesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();

	// Face the direction of travel, turning at up to 540 degrees per second.
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);

	// RVO avoidance makes nearby enemies steer around each other. Without it, every enemy heads for the
	// same point and the horde collapses into a single queue behind the player.
	Movement->bUseRVOAvoidance = true;
	Movement->AvoidanceConsiderationRadius = 150.f;

	// Rotation comes from movement (above), not from the AI controller.
	bUseControllerRotationYaw = false;

	// Starting health, inherited from AHKHuman. Each projectile deals 1 damage by default, so 2 means two
	// shots to kill.
	MaxHealth = 2.f;
}

void AHKEnemy::SetHordeGenerator(AHKHordeGenerator* InGenerator)
{
	HordeGenerator = InGenerator;
}

void AHKEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Read the tunable property here rather than in the constructor so that values changed in a child
	// Blueprint or on a placed instance are respected. Health is initialised by AHKHuman.
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	// A dynamic material instance lets this enemy change colour without affecting the others. The
	// engine's basic shape material exposes a vector parameter named "Color".
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

	// Single-player game: the target is always player 0. Stop chasing once the player is dead so the
	// horde does not keep attacking during the game-over delay.
	const AHKCharacter* Player = Cast<AHKCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player || Player->IsDead())
	{
		return;
	}

	// Direct steering: head straight for the player, ignoring height. No NavMesh is involved, which is
	// fine in an open arena but means enemies cannot path around obstacles.
	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const float Distance = ToPlayer.Size();

	// Stop pushing a little inside attack range. This keeps the enemy from shoving the player around
	// while still leaving it close enough to keep attacking.
	if (Distance > AttackRange * 0.8f)
	{
		AddMovementInput(ToPlayer.GetSafeNormal());
	}

	// Melee attack: simply being in range deals damage, limited by the cooldown.
	const float Now = GetWorld()->GetTimeSeconds();
	if (Distance <= AttackRange && Now - LastAttackTime >= AttackCooldown)
	{
		LastAttackTime = Now;

		// ApplyDamage takes a non-const actor; the player pointer is const here only because this
		// function does not otherwise modify the player.
		UGameplayStatics::ApplyDamage(const_cast<AHKCharacter*>(Player), AttackDamage, GetController(), this, nullptr);
	}
}

void AHKEnemy::HandleDamaged(float DamageApplied)
{
	Super::HandleDamaged(DamageApplied);

	// Survived the hit: show that this enemy has been wounded.
	if (BodyMaterial)
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), WoundedColor);
	}
}

void AHKEnemy::HandleDeath()
{
	Super::HandleDeath();

	// Report the death to the horde generator that spawned this enemy. It keeps the kill count and
	// decides when the wave is over. An enemy placed in a level by hand has no generator to report to.
	if (AHKHordeGenerator* Generator = HordeGenerator.Get())
	{
		Generator->NotifyEnemyKilled(this);
	}

	// No death animation or ragdoll yet: the enemy just disappears.
	Destroy();
}
