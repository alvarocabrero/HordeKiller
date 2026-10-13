// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Characters/HKEnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Managers/HKActorManager.h"

void AHKEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	UHKActorManager::Register(this);
}

void AHKEnemyAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHKActorManager::Unregister(this);

	Super::EndPlay(EndPlayReason);
}

void AHKEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}
}
