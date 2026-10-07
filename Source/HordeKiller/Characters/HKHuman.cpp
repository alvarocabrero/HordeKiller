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

	// The model's origin is at its feet; the capsule's is at its centre.
	FVector Location = Body->GetRelativeLocation();
	Location.Z = -GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	Body->SetRelativeLocation(Location);

	if (!BodyAnimClass.IsNull() && FPackageName::DoesPackageExist(BodyAnimClass.GetLongPackageName()))
	{
		if (UClass* AnimClass = BodyAnimClass.LoadSynchronous())
		{
			Body->SetAnimInstanceClass(AnimClass);
		}
	}
	return true;
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
