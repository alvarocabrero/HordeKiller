// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Animation/HKAnimInstanceHuman.h"
#include "Characters/HKHuman.h"
#include "GameFramework/CharacterMovementComponent.h"

void UHKAnimInstanceHuman::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Null in the animation editor preview, where the owner is not an AHKHuman.
	Human = Cast<AHKHuman>(TryGetPawnOwner());
	CharacterMovement = Human ? Human->GetCharacterMovement() : nullptr;
}

void UHKAnimInstanceHuman::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Human || !CharacterMovement)
	{
		return;
	}

	Velocity = CharacterMovement->Velocity;
	GroundSpeed = Velocity.Size2D();

	// Yaw of the velocity relative to the facing direction.
	Direction = GroundSpeed > 0.f
		? FRotator::NormalizeAxis(Velocity.Rotation().Yaw - Human->GetActorRotation().Yaw)
		: 0.f;

	// Requiring acceleration stops the walk cycle while sliding to a halt or being pushed.
	const bool bHasAcceleration = !CharacterMovement->GetCurrentAcceleration().IsNearlyZero();
	bShouldMove = GroundSpeed > MoveSpeedThreshold && bHasAcceleration;

	bIsFalling = CharacterMovement->IsFalling();
	bIsDead = Human->IsDead();
}
