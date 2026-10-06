#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HordeKillerEnemy.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;

/** Horde enemy: runs straight at the player, hits on contact and dies after two projectile hits. */
UCLASS()
class HORDEKILLER_API AHordeKillerEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AHordeKillerEnemy();

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	void Die();

	// Placeholder body built from an engine basic shape.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	// Each projectile deals 1 damage, so 2 means two shots to kill.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MaxHealth = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MoveSpeed = 380.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackDamage = 10.f;

	// Distance between capsule centres at which the enemy can hit the player.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackRange = 110.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackCooldown = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FLinearColor HealthyColor = FLinearColor(0.8f, 0.05f, 0.05f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FLinearColor WoundedColor = FLinearColor(1.f, 0.55f, 0.f);

private:
	float Health = 0.f;
	float LastAttackTime = -1000.f;
	bool bDead = false;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;
};
