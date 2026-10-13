// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Characters/HKHuman.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Managers/HKActorManager.h"

void AHKHuman::BeginPlay()
{
	Super::BeginPlay();

	UHKActorManager::Register(this);

	// Read here so that values set by subclasses or Blueprints apply.
	Health = MaxHealth;

	// The model is assigned on the mesh component in the character's Blueprint; it may be missing.
	bHasBodyModel = GetMesh()->GetSkeletalMeshAsset() != nullptr;
}

void AHKHuman::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHKActorManager::Unregister(this);

	Super::EndPlay(EndPlayReason);
}

void AHKHuman::PlaceBodyOnCapsule()
{
	// Back to the offset the Blueprint gives the mesh component, which ACharacter keeps a copy of.
	USkeletalMeshComponent* Body = GetMesh();
	Body->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Body->SetRelativeLocationAndRotation(GetBaseTranslationOffset(), GetBaseRotationOffset());
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
