// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HKAnimInstanceHuman.generated.h"

class AHKHuman;
class UCharacterMovementComponent;

/**
 * Base animation instance for humanoid characters: the player and the enemies.
 *
 * It reads the state of the AHKHuman that owns the mesh once per frame and stores it in a few
 * variables. An animation Blueprint that derives from this class (such as ABP_HKHuman) only has to
 * read those variables in its graph to choose what to play; it does not need any logic of its own in
 * its event graph.
 *
 * All variables are read-only from Blueprint and are listed under the "Human" category.
 */
UCLASS()
class HORDEKILLER_API UHKAnimInstanceHuman : public UAnimInstance
{
	GENERATED_BODY()

public:
	/** Called once when the animation instance is created. Finds the owning human and its movement component. */
	virtual void NativeInitializeAnimation() override;

	/**
	 * Called every frame before the animation graph is evaluated. Refreshes the variables below.
	 *
	 * @param DeltaSeconds Time elapsed since the previous frame, in seconds.
	 */
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** The character this instance animates. Null while previewing the Blueprint in the editor. */
	UPROPERTY(BlueprintReadOnly, Category = "Human")
	TObjectPtr<AHKHuman> Human;

	/** Movement component of the character. Null while previewing the Blueprint in the editor. */
	UPROPERTY(BlueprintReadOnly, Category = "Human")
	TObjectPtr<UCharacterMovementComponent> HumanMovement;

	/** Current velocity of the character in world space, in cm/s. */
	UPROPERTY(BlueprintReadOnly, Category = "Human")
	FVector HumanVelocity = FVector::ZeroVector;

	/** Horizontal speed of the character, in cm/s. Use it to drive an idle / walk / run blend space. */
	UPROPERTY(BlueprintReadOnly, Category = "Human")
	float HumanGroundSpeed = 0.f;

	/** Angle between the direction of travel and the direction the character faces, in degrees, from -180 to 180. 0 is straight ahead. */
	UPROPERTY(BlueprintReadOnly, Category = "Human")
	float HumanDirection = 0.f;

	/** True while the character is moving on purpose: it is above MoveSpeedThreshold and is being accelerated by input. */
	UPROPERTY(BlueprintReadOnly, Category = "Human")
	bool bHumanShouldMove = false;

	/** True while the character is in the air, jumping or falling. */
	UPROPERTY(BlueprintReadOnly, Category = "Human")
	bool bHumanIsFalling = false;

	/** True once the character's health has reached zero. */
	UPROPERTY(BlueprintReadOnly, Category = "Human")
	bool bHumanIsDead = false;

	/** Horizontal speed, in cm/s, above which the character counts as moving. Avoids flickering between idle and walk. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Human", meta = (ClampMin = "0", Units = "cm/s"))
	float MoveSpeedThreshold = 3.f;
};
