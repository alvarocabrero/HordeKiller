// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Characters/HKHuman.h"
#include "Managers/HKActorManager.h"

void AHKHuman::BeginPlay()
{
	Super::BeginPlay();

	UHKActorManager::Register(this);

	// Read here so that values set by subclasses or Blueprints apply.
	Health = MaxHealth;
}

void AHKHuman::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHKActorManager::Unregister(this);

	Super::EndPlay(EndPlayReason);
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
