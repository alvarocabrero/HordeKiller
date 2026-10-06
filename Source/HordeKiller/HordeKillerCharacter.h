#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HordeKillerCharacter.generated.h"

class AHordeKillerProjectile;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UStaticMeshComponent;
struct FInputActionValue;

/** First-person shooter character that fires physics projectiles, using Enhanced Input. */
UCLASS()
class HORDEKILLER_API AHordeKillerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHordeKillerCharacter();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	bool IsDead() const { return bDead; }

protected:
	virtual void BeginPlay() override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Fire();

	/** Builds WASD / mouse / Space / left click bindings in code for any input asset left unassigned. */
	void CreateDefaultInputAssets();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	// Placeholder weapon attached to the camera.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> GunMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AHordeKillerProjectile> ProjectileClass;

	// Seconds between shots while the fire button is held.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float FireInterval = 0.2f;

	// Distance in front of the camera where projectiles spawn, so they clear the player's capsule.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float MuzzleDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

	// Optional: assign these in a child Blueprint to override the bindings created in code.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

private:
	float Health = 0.f;
	float LastFireTime = -1000.f;
	bool bDead = false;
};
