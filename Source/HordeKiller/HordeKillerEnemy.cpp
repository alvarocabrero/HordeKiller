#include "HordeKillerEnemy.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "HordeKillerCharacter.h"
#include "HordeKillerGameMode.h"
#include "UObject/ConstructorHelpers.h"

AHordeKillerEnemy::AHordeKillerEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Spawned enemies need a controller, otherwise their movement input is never consumed.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// The cylinder is 100 x 100 x 100 cm; scale it to fill the capsule.
	BodyMesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
	// Keeps the horde from collapsing into a single line behind the player.
	Movement->bUseRVOAvoidance = true;
	Movement->AvoidanceConsiderationRadius = 150.f;

	bUseControllerRotationYaw = false;
}

void AHordeKillerEnemy::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	BodyMaterial = BodyMesh->CreateDynamicMaterialInstance(0);
	if (BodyMaterial)
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), HealthyColor);
	}
}

void AHordeKillerEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead)
	{
		return;
	}

	const AHordeKillerCharacter* Player = Cast<AHordeKillerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player || Player->IsDead())
	{
		return;
	}

	// Direct steering: no NavMesh needed, which is enough for an open arena.
	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const float Distance = ToPlayer.Size();

	if (Distance > AttackRange * 0.8f)
	{
		AddMovementInput(ToPlayer.GetSafeNormal());
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Distance <= AttackRange && Now - LastAttackTime >= AttackCooldown)
	{
		LastAttackTime = Now;
		UGameplayStatics::ApplyDamage(const_cast<AHordeKillerCharacter*>(Player), AttackDamage, GetController(), this, nullptr);
	}
}

float AHordeKillerEnemy::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (bDead || Applied <= 0.f)
	{
		return 0.f;
	}

	Health -= Applied;
	if (Health <= 0.f)
	{
		Die();
	}
	else if (BodyMaterial)
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), WoundedColor);
	}
	return Applied;
}

void AHordeKillerEnemy::Die()
{
	bDead = true;

	if (AHordeKillerGameMode* GameMode = GetWorld()->GetAuthGameMode<AHordeKillerGameMode>())
	{
		GameMode->NotifyEnemyKilled();
	}
	Destroy();
}
