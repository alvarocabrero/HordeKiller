// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Characters/HKHuman.h"
#include "Managers/HKActorManager.h"

void AHKHuman::BeginPlay()
{
	Super::BeginPlay();

	// Every actor of the project subscribes itself to the manager. Doing it here covers the player and
	// the enemies at once, since both inherit from this class.
	UHKActorManager::Register(this);

	// Read MaxHealth here rather than in the constructor so that values set by a subclass constructor,
	// a child Blueprint or a placed instance are respected.
	Health = MaxHealth;
}

void AHKHuman::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// EndPlay runs whenever the actor leaves the game, whatever the reason, so the manager never keeps
	// a reference to a character that no longer exists.
	UHKActorManager::Unregister(this);

	Super::EndPlay(EndPlayReason);
}

float AHKHuman::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	// The engine's base class decides whether the damage is accepted at all and returns the final amount.
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (bDead || Applied <= 0.f)
	{
		return 0.f;
	}

	Health = FMath::Max(0.f, Health - Applied);
	if (Health <= 0.f)
	{
		// Set before calling the hook so that any further damage arriving this frame is ignored and
		// the death is handled exactly once.
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
	// Nothing by default; subclasses react if they need to.
}

void AHKHuman::HandleDeath()
{
	// Nothing by default; subclasses decide what dying means for them.
}
