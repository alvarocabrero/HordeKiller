// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HKAnimLayerInstance.generated.h"

class UHKAnimInstanceHuman;

/**
 * Base class for the animation layers of humanoid characters.
 *
 * Every humanoid shares one animation Blueprint, whose parent is UHKAnimInstanceHuman. What differs
 * between the player and the enemies, such as which animations they use to stand and to move, lives in
 * animation layers: animation Blueprints that derive from this class and implement the layers of the
 * shared animation layer interface. AHKHuman links the layer Blueprint chosen in its AnimLayerClass
 * property onto the shared one when play begins.
 *
 * A layer has no state of its own. It reads the character's state from the main instance, which is
 * the one that keeps it up to date every frame.
 */
UCLASS()
class HORDEKILLER_API UHKAnimLayerInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/** Called once when the layer is created. Finds the main animation instance of the mesh the layer runs on. */
	virtual void NativeInitializeAnimation() override;

	/**
	 * @return The main animation instance of the mesh, which holds GroundSpeed, Direction, bShouldMove and
	 *         the rest of the character's state. Null while previewing the layer Blueprint on its own.
	 */
	UFUNCTION(BlueprintPure, Category = "Human", meta = (BlueprintThreadSafe))
	UHKAnimInstanceHuman* GetMainInstance() const { return MainInstance; }

protected:
	/** The main animation instance this layer is linked to. Null while previewing the layer Blueprint on its own. */
	UPROPERTY(BlueprintReadOnly, Category = "Human")
	TObjectPtr<UHKAnimInstanceHuman> MainInstance;
};
