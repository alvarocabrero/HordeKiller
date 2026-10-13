// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HKEnemyAIController.generated.h"

class UBehaviorTree;

/**
 * AI controller of the horde enemies. It runs a behaviour tree on the enemy it possesses.
 *
 * The tree is an asset, assigned in BP_HKEnemyAIController, and it is where the enemy's decisions are
 * meant to live. It starts when the controller takes an enemy and keeps running while the enemy waits
 * in the pool, so its nodes have to cope with an enemy that is not in play.
 */
UCLASS()
class HORDEKILLER_API AHKEnemyAIController : public AAIController
{
	GENERATED_BODY()

protected:
	/** Registers the controller with the actor manager once it is in the world. */
	virtual void BeginPlay() override;

	/**
	 * Unregisters the controller from the actor manager.
	 *
	 * @param EndPlayReason Why play is ending for this actor (destroyed, level change, game exit...).
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Starts BehaviorTree on the pawn that was just possessed.
	 *
	 * @param InPawn The enemy this controller now drives.
	 */
	virtual void OnPossess(APawn* InPawn) override;

protected:
	/** Behaviour tree run on the possessed enemy. Empty in C++: assigned in BP_HKEnemyAIController. Leave empty to run none. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
};
