// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Characters/HKCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Game/HKGameMode.h"
#include "Weapons/HKProjectile.h"
#include "UObject/ConstructorHelpers.h"

AHKCharacter::AHKCharacter()
{
	// Radius and half-height of the collision capsule, in cm: the engine's usual human-sized character.
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// First person: the body turns left and right with the view (yaw). Pitch and roll are not applied
	// to the body, otherwise the capsule would tilt when looking up or down; the camera handles pitch.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();

	// The body already follows the view, so it must not also try to face its direction of travel.
	Movement->bOrientRotationToMovement = false;
	Movement->JumpZVelocity = 600.f;              // Upward speed at the start of a jump, in cm/s.
	Movement->AirControl = 0.35f;                 // Fraction of normal steering available while airborne.
	Movement->MaxWalkSpeed = 600.f;               // Running speed in cm/s; faster than the enemies (380).
	Movement->BrakingDecelerationWalking = 2000.f; // How quickly the character stops when input ends, in cm/s^2.

	// Camera 64 cm above the capsule centre, which is roughly eye height. With bUsePawnControlRotation
	// it copies the controller's full rotation, so mouse pitch tilts the view but not the body.
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// The weapon is attached to the camera so it stays in the same place on screen: 45 cm forward,
	// 22 cm to the right and 18 cm down. The 100 cm cube is scaled into a 50 x 8 x 8 cm bar.
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
	GunMesh->SetupAttachment(FirstPersonCamera);
	GunMesh->SetRelativeLocation(FVector(45.f, 22.f, -18.f));
	GunMesh->SetRelativeScale3D(FVector(0.5f, 0.08f, 0.08f));

	// Purely cosmetic: it must not block projectiles or enemies, and its shadow would look wrong
	// floating in front of the camera.
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		GunMesh->SetStaticMesh(CubeMesh.Object);
	}

	// Default to the C++ projectile; a child Blueprint can point this at its own projectile class.
	ProjectileClass = AHKProjectile::StaticClass();

	// Starting health, inherited from AHKHuman. Each enemy attack removes 10 by default.
	MaxHealth = 100.f;
}

void AHKCharacter::CreateDefaultInputAssets()
{
	// Enhanced Input normally relies on assets authored in the editor. Creating them here as transient
	// objects owned by the character gives the same result with no content in the project. Each one is
	// created only if the property is still empty, so assets assigned in a Blueprint take precedence.
	if (!MoveAction)
	{
		MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
		MoveAction->ValueType = EInputActionValueType::Axis2D;
	}
	if (!LookAction)
	{
		LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
		LookAction->ValueType = EInputActionValueType::Axis2D;
	}
	if (!JumpAction)
	{
		// Actions are boolean (pressed / released) unless told otherwise, which is what a button needs.
		JumpAction = NewObject<UInputAction>(this, TEXT("IA_Jump"));
	}
	if (!FireAction)
	{
		FireAction = NewObject<UInputAction>(this, TEXT("IA_Fire"));
	}

	// A mapping context assigned from a Blueprint is used as it is; nothing below applies to it.
	if (DefaultMappingContext)
	{
		return;
	}

	DefaultMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Default"));
	UInputMappingContext* Context = DefaultMappingContext;

	// The move action is a 2D axis read as (X = right, Y = forward). A keyboard key only ever produces
	// a value on X, so modifiers are needed to turn four keys into the four directions:
	//   - Swizzle (YXZ order) swaps X and Y, moving a key's value onto the forward axis.
	//   - Negate flips the sign, turning "right" into "left" or "forward" into "backward".
	auto AddSwizzle = [Context](FEnhancedActionKeyMapping& Mapping)
	{
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(Context);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		Mapping.Modifiers.Add(Swizzle);
	};
	auto AddNegate = [Context](FEnhancedActionKeyMapping& Mapping, bool bX, bool bY)
	{
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(Context);
		Negate->bX = bX;
		Negate->bY = bY;
		Negate->bZ = false;
		Mapping.Modifiers.Add(Negate);
	};

	// W: (1, 0) swizzled to (0, 1), forward.
	AddSwizzle(Context->MapKey(MoveAction, EKeys::W));

	// S: (1, 0) negated to (-1, 0), then swizzled to (0, -1), backward. Modifiers run in the order they
	// were added, so the negate must come first, while the value is still on X.
	{
		FEnhancedActionKeyMapping& Back = Context->MapKey(MoveAction, EKeys::S);
		AddNegate(Back, true, false);
		AddSwizzle(Back);
	}

	// D: (1, 0) as it is, right.
	Context->MapKey(MoveAction, EKeys::D);

	// A: (1, 0) negated to (-1, 0), left.
	AddNegate(Context->MapKey(MoveAction, EKeys::A), true, false);

	// Mouse2D reports both mouse axes at once. Its Y grows when the mouse moves up, while positive
	// pitch input looks down, so Y is negated to make "mouse up" mean "look up".
	AddNegate(Context->MapKey(LookAction, EKeys::Mouse2D), false, true);

	Context->MapKey(JumpAction, EKeys::SpaceBar);
	Context->MapKey(FireAction, EKeys::LeftMouseButton);
}

void AHKCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// This runs every time the character gets a new controller, including after a level restart, which
	// makes it more reliable than BeginPlay: there the controller may not have been assigned yet.
	// Only a locally controlled player has an Enhanced Input subsystem, so AI controllers skip this.
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			CreateDefaultInputAssets();

			// Priority 0 is enough: this is the only mapping context in the game.
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AHKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// The engine does not guarantee whether this or NotifyControllerChanged runs first, so both make
	// sure the actions exist before using them.
	CreateDefaultInputAssets();

	// The project is configured (Config/DefaultInput.ini) to create Enhanced Input components, so this
	// cast succeeds; it would fail only if that setting were changed.
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jump and StopJumping are inherited from ACharacter: one starts the jump on key press, the
		// other tells the movement component the key was released.
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// "Triggered" fires on every frame the input is active, which is what continuous actions need.
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHKCharacter::Move);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHKCharacter::Look);

		// Fire is also called every frame while the button is held; FireInterval limits the real rate,
		// which is what gives automatic fire.
		Input->BindAction(FireAction, ETriggerEvent::Triggered, this, &AHKCharacter::Fire);
	}
}

void AHKCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	// Movement is relative to where the body faces. Because the body follows the view's yaw, "forward"
	// is always the direction the player is looking, flattened onto the ground.
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void AHKCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	// These rotate the controller, not the character. The body picks up the yaw and the camera picks up
	// the full rotation, as configured in the constructor.
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AHKCharacter::Fire()
{
	UWorld* World = GetWorld();
	if (IsDead() || !ProjectileClass || !World)
	{
		return;
	}

	// Rate limit. Comparing world times avoids needing a timer and works with the per-frame calls that
	// a held fire button produces.
	const float Now = World->GetTimeSeconds();
	if (Now - LastFireTime < FireInterval)
	{
		return;
	}
	LastFireTime = Now;

	// Aim along the view. The projectile starts on the line through the centre of the screen, in front
	// of the camera, so it flies towards the crosshair and does not start inside the player's capsule.
	const FRotator AimRotation = GetControlRotation();
	const FVector AimDirection = AimRotation.Vector();
	const FVector SpawnLocation = FirstPersonCamera->GetComponentLocation() + AimDirection * MuzzleDistance;

	FActorSpawnParameters Params;
	Params.Owner = this;
	// The instigator is what lets the projectile attribute its damage to the player.
	Params.Instigator = this;
	// Spawn even if the muzzle is touching something (for example when standing against a wall);
	// otherwise the shot would silently fail.
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AHKProjectile* Projectile =
			World->SpawnActor<AHKProjectile>(ProjectileClass, SpawnLocation, AimRotation, Params))
	{
		// Spawning only places the projectile; the impulse is what sets it in motion.
		Projectile->Launch(AimDirection);
	}
}

void AHKCharacter::HandleDeath()
{
	Super::HandleDeath();

	// Freeze the player in place: no more movement and no more input, including firing.
	GetCharacterMovement()->DisableMovement();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	// The game mode shows the game-over state and restarts the level after a delay.
	if (AHKGameMode* GameMode = GetWorld()->GetAuthGameMode<AHKGameMode>())
	{
		GameMode->NotifyPlayerDied();
	}
}
