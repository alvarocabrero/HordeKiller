// Copyright (c) 2026 alvarocabrero. Licensed under the MIT License. See LICENSE in the repository root.

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

/**
 * The player: a first-person character armed with a weapon that fires physics projectiles.
 *
 * Input uses the Enhanced Input system. So that the game is playable without creating any assets in
 * the editor, the input actions and the key mappings are built in code the first time they are needed.
 * Any of them can be replaced by assigning a real asset to the matching property in a child Blueprint.
 *
 * The character has health, takes damage from enemies and notifies the game mode when it dies.
 */
UCLASS()
class HORDEKILLER_API AHordeKillerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	/** Creates the camera and weapon components and configures first-person movement. */
	AHordeKillerCharacter();

	/**
	 * Applies incoming damage and handles death.
	 *
	 * @param DamageAmount    Raw damage requested by the caller.
	 * @param DamageEvent     Extra data describing the kind of damage.
	 * @param EventInstigator Controller responsible for the damage (the enemy's AI controller).
	 * @param DamageCauser    Actor that dealt the damage (the enemy).
	 * @return The damage actually applied, or 0 if it was ignored.
	 */
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

	/** @return Current health, between 0 and MaxHealth. */
	float GetHealth() const { return Health; }

	/** @return Health the character starts with. */
	float GetMaxHealth() const { return MaxHealth; }

	/** @return True once health has reached zero. */
	bool IsDead() const { return bDead; }

protected:
	/** Fills health once the character is in the world. */
	virtual void BeginPlay() override;

	/** Registers the input mapping context with the local player whenever a player controller takes over. */
	virtual void NotifyControllerChanged() override;

	/**
	 * Binds the input actions to the functions below.
	 *
	 * @param PlayerInputComponent Input component created by the engine for this pawn.
	 */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/**
	 * Moves the character on the ground plane.
	 *
	 * @param Value 2D axis: X is right (+) / left (-), Y is forward (+) / backward (-).
	 */
	void Move(const FInputActionValue& Value);

	/**
	 * Rotates the view.
	 *
	 * @param Value 2D axis: X is yaw (turn right is positive), Y is pitch.
	 */
	void Look(const FInputActionValue& Value);

	/** Fires one projectile along the aim direction, if the fire interval has elapsed. */
	void Fire();

	/**
	 * Creates, in code, every input action and the mapping context that have not been assigned.
	 * Default bindings: WASD to move, mouse to look, Space to jump, left mouse button to fire.
	 * Safe to call more than once; it only fills what is still missing.
	 */
	void CreateDefaultInputAssets();

	/** First-person camera at eye height. It follows the controller's rotation, including pitch. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	/** Placeholder weapon: a stretched cube attached to the camera, with no collision. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> GunMesh;

	/** Projectile class spawned on each shot. Replace it with a child Blueprint to change the projectile's look or values. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AHordeKillerProjectile> ProjectileClass;

	/** Minimum time between two shots while the fire button is held, in seconds. 0.2 means five shots per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float FireInterval = 0.2f;

	/** Distance in front of the camera at which projectiles spawn, in cm. Must be large enough to clear the player's own capsule. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float MuzzleDistance = 100.f;

	/** Health the character starts with. Each enemy attack removes 10 by default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

	/** Optional mapping context asset. If left empty, default key bindings are created in code. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Optional move action asset (2D axis). If left empty, one is created in code. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Optional look action asset (2D axis). If left empty, one is created in code. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	/** Optional jump action asset (button). If left empty, one is created in code. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	/** Optional fire action asset (button). If left empty, one is created in code. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

private:
	/** Current health. Set from MaxHealth in BeginPlay. */
	float Health = 0.f;

	/** World time of the last shot, in seconds. Starts far in the past so the first shot is immediate. */
	float LastFireTime = -1000.f;

	/** True once health has reached zero. Blocks firing and further damage. */
	bool bDead = false;
};
