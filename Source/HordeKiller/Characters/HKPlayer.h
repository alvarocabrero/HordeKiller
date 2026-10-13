// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#pragma once

#include "CoreMinimal.h"
#include "Characters/HKHuman.h"
#include "HKPlayer.generated.h"

class AHKProjectile;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UStaticMeshComponent;
struct FInputActionValue;

/**
 * The player: a third-person character armed with a weapon that fires physics projectiles.
 *
 * The camera sits behind and over the right shoulder, on a boom that shortens when something gets
 * between it and the player. The body turns with the view, so the player always faces where they aim,
 * and shots go to the point under the crosshair.
 *
 * Input uses the Enhanced Input system. So that the game is playable without creating any assets in
 * the editor, the input actions and the key mappings are built in code the first time they are needed.
 * Any of them can be replaced by assigning a real asset to the matching property in a child Blueprint.
 *
 * Health, taking damage and the dead state are inherited from AHKHuman. This class adds what dying
 * means for the player: freezing in place and telling the game mode that the game is over.
 */
UCLASS()
class HORDEKILLER_API AHKPlayer : public AHKHuman
{
	GENERATED_BODY()

public:
	/** Creates the camera and weapon components and configures third-person movement. */
	AHKPlayer();

protected:
	/** Stops the player's movement and input and notifies the game mode. Called once by AHKHuman when health reaches zero. */
	virtual void HandleDeath() override;

	/** Applies CameraDistance and CameraOffset to the boom, so values edited in a Blueprint take effect. */
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

	/** Fires one projectile from the player's chest towards the point under the crosshair, if the fire interval has elapsed. */
	void Fire();

	/** @return The world position the crosshair points at: the first thing the camera's view line hits, or a far point along it. */
	FVector GetAimPoint() const;

	/**
	 * Creates, in code, every input action and the mapping context that have not been assigned.
	 * Default bindings: WASD to move, mouse to look, Space to jump, left mouse button to fire.
	 * Safe to call more than once; it only fills what is still missing.
	 */
	void CreateDefaultInputAssets();

protected:
	/** Boom that holds the camera behind the player. It follows the controller's rotation and shortens to avoid walls. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Third-person camera at the end of the boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** Placeholder weapon: a stretched cube in the body's right hand, with no collision. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> GunMesh;

	/** Distance from the player to the camera, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0", Units = "cm"))
	float CameraDistance = 350.f;

	/** Offset of the camera from the end of the boom, in cm: X forward, Y to the right, Z up. Y moves the player off the crosshair. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector CameraOffset = FVector(0.f, 60.f, 70.f);

	/** Height above the centre of the capsule from which projectiles are fired, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (Units = "cm"))
	float MuzzleHeight = 50.f;

	/** How far the crosshair's view line is checked for something to aim at, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0", Units = "cm"))
	float AimRange = 20000.f;

	/** Projectile class spawned on each shot. Replace it with a child Blueprint to change the projectile's look or values. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AHKProjectile> ProjectileClass;

	/** Minimum time between two shots while the fire button is held, in seconds. 0.2 means five shots per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float FireInterval = 0.2f;

	/** Distance in front of the player at which projectiles spawn, in cm. Must be large enough to clear the player's own capsule. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float MuzzleDistance = 100.f;

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
	/** World time of the last shot, in seconds. Starts far in the past so the first shot is immediate. */
	float LastFireTime = -1000.f;
};
