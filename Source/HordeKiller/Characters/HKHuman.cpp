// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Characters/HKHuman.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Managers/HKActorManager.h"
#include "Misc/PackageName.h"

AHKHuman::AHKHuman()
{
	// Epic's mannequin. Soft paths, so a project without these assets still loads.
	BodyModel = FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	BodyAnimClass = FSoftObjectPath(TEXT("/Game/Characters/Animation/ABP_HKHuman.ABP_HKHuman_C"));

	// The mannequin faces +Y; characters face +X.
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHKHuman::BeginPlay()
{
	Super::BeginPlay();

	UHKActorManager::Register(this);

	// Read here so that values set by subclasses or Blueprints apply.
	Health = MaxHealth;

	bHasBodyModel = ApplyBodyModel();
}

void AHKHuman::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHKActorManager::Unregister(this);

	Super::EndPlay(EndPlayReason);
}

bool AHKHuman::ApplyBodyModel()
{
	// Checked first because loading a missing asset logs a warning, and missing is a normal case here.
	if (BodyModel.IsNull() || !FPackageName::DoesPackageExist(BodyModel.GetLongPackageName()))
	{
		return false;
	}

	USkeletalMesh* Model = BodyModel.LoadSynchronous();
	if (!Model)
	{
		return false;
	}

	USkeletalMeshComponent* Body = GetMesh();
	Body->SetSkeletalMesh(Model);
	PlaceBodyOnCapsule();

	if (!BodyAnimClass.IsNull() && FPackageName::DoesPackageExist(BodyAnimClass.GetLongPackageName()))
	{
		if (UClass* AnimClass = BodyAnimClass.LoadSynchronous())
		{
			Body->SetAnimInstanceClass(AnimClass);
		}
	}
	return true;
}

void AHKHuman::PlaceBodyOnCapsule()
{
	// The model's origin is at its feet and it faces +Y; the capsule's origin is its centre and it faces +X.
	USkeletalMeshComponent* Body = GetMesh();
	Body->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Body->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0.f, -90.f, 0.f));
}

void AHKHuman::Revive()
{
	Health = MaxHealth;
	bDead = false;
	LastDamageCauser.Reset();
}

void AHKHuman::StartRagdoll(const FVector& Impulse)
{
	if (!bHasBodyModel)
	{
		return;
	}

	// "Ragdoll" is the engine preset for this: collides with the world, ignores pawns.
	USkeletalMeshComponent* Body = GetMesh();
	Body->SetCollisionProfileName(TEXT("Ragdoll"));
	// Explicit, because StopRagdoll turns collision off and the profile name alone would not turn it back on.
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Body->SetSimulatePhysics(true);
	Body->AddImpulse(Impulse, NAME_None, true);
}

void AHKHuman::StopRagdoll()
{
	if (!bHasBodyModel)
	{
		return;
	}

	USkeletalMeshComponent* Body = GetMesh();
	Body->SetSimulatePhysics(false);
	Body->SetPhysicsBlendWeight(0.f);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Simulating moved the body away from the capsule.
	PlaceBodyOnCapsule();
}

void AHKHuman::SetBodyTint(const FLinearColor& Color)
{
	if (!bHasBodyModel)
	{
		return;
	}

	// "Paint Tint" is the colour parameter of the mannequin's materials.
	USkeletalMeshComponent* Body = GetMesh();
	for (int32 Index = 0; Index < Body->GetNumMaterials(); ++Index)
	{
		if (UMaterialInstanceDynamic* Material = Body->CreateDynamicMaterialInstance(Index))
		{
			Material->SetVectorParameterValue(TEXT("Paint Tint"), Color);
		}
	}
}

float AHKHuman::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (bDead || Applied <= 0.f)
	{
		return 0.f;
	}

	LastDamageCauser = DamageCauser;
	Health = FMath::Max(0.f, Health - Applied);
	if (Health <= 0.f)
	{
		// Set before the hook, so the death is handled exactly once.
		bDead = true;
		HandleDeath();
	}
	else
	{
		HandleDamaged(Applied);
	}
	return Applied;
}

void AHKHuman::HandleDamaged(float DamageApplied)
{
}

void AHKHuman::HandleDeath()
{
}
