// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Animation/HKAnimLayerInstance.h"
#include "Animation/HKAnimInstanceHuman.h"
#include "Components/SkeletalMeshComponent.h"

void UHKAnimLayerInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// The component's own instance is the main one; a linked layer is never returned here.
	const USkeletalMeshComponent* Component = GetSkelMeshComponent();
	MainInstance = Component ? Cast<UHKAnimInstanceHuman>(Component->GetAnimInstance()) : nullptr;
}
