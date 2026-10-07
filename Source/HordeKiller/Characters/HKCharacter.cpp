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
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// First person: the body follows the view's yaw only; the camera handles pitch.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->JumpZVelocity = 600.f;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = 600.f; // Faster than the enemies (380).
	Movement->BrakingDecelerationWalking = 2000.f;

	// Camera at eye height, copying the controller's full rotation.
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// Cosmetic weapon: a cube scaled into a bar, fixed in front of the camera.
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
	GunMesh->SetupAttachment(FirstPersonCamera);
	GunMesh->SetRelativeLocation(FVector(45.f, 22.f, -18.f));
	GunMesh->SetRelativeScale3D(FVector(0.5f, 0.08f, 0.08f));
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		GunMesh->SetStaticMesh(CubeMesh.Object);
	}

	ProjectileClass = AHKProjectile::StaticClass();
	MaxHealth = 100.f;

	// The player uses Quinn. First person: the owner does not see the body, only its shadow.
	BodyModel = FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;
}

void AHKCharacter::CreateDefaultInputAssets()
{
	// Only fills what is still empty, so assets assigned in a Blueprint win.
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
		JumpAction = NewObject<UInputAction>(this, TEXT("IA_Jump"));
	}
	if (!FireAction)
	{
		FireAction = NewObject<UInputAction>(this, TEXT("IA_Fire"));
	}

	if (DefaultMappingContext)
	{
		return;
	}

	DefaultMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Default"));
	UInputMappingContext* Context = DefaultMappingContext;

	// Move is (X = right, Y = forward). A key only produces X: swizzle moves it to Y, negate flips it.
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

	// W: forward.
	AddSwizzle(Context->MapKey(MoveAction, EKeys::W));

	// S: backward. Modifiers run in order, so negate first, while the value is still on X.
	{
		FEnhancedActionKeyMapping& Back = Context->MapKey(MoveAction, EKeys::S);
		AddNegate(Back, true, false);
		AddSwizzle(Back);
	}

	// D: right. A: left.
	Context->MapKey(MoveAction, EKeys::D);
	AddNegate(Context->MapKey(MoveAction, EKeys::A), true, false);

	// Negate mouse Y so that moving the mouse up looks up.
	AddNegate(Context->MapKey(LookAction, EKeys::Mouse2D), false, true);

	Context->MapKey(JumpAction, EKeys::SpaceBar);
	Context->MapKey(FireAction, EKeys::LeftMouseButton);
}

void AHKCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Done here, not in BeginPlay, because the controller may not be assigned yet there.
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			CreateDefaultInputAssets();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AHKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// May run before NotifyControllerChanged, so make sure the actions exist.
	CreateDefaultInputAssets();

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHKCharacter::Move);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHKCharacter::Look);

		// Called every frame while held; FireInterval limits the real rate.
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

	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void AHKCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
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

	// Rate limit.
	const float Now = World->GetTimeSeconds();
	if (Now - LastFireTime < FireInterval)
	{
		return;
	}
	LastFireTime = Now;

	// Spawn in front of the camera, along the aim line, clear of the player's capsule.
	const FRotator AimRotation = GetControlRotation();
	const FVector AimDirection = AimRotation.Vector();
	const FVector SpawnLocation = FirstPersonCamera->GetComponentLocation() + AimDirection * MuzzleDistance;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this; // Attributes the projectile's damage to the player.
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AHKProjectile* Projectile =
			World->SpawnActor<AHKProjectile>(ProjectileClass, SpawnLocation, AimRotation, Params))
	{
		Projectile->Launch(AimDirection);
	}
}

void AHKCharacter::HandleDeath()
{
	Super::HandleDeath();

	// Freeze the player: no movement, no input.
	GetCharacterMovement()->DisableMovement();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	if (AHKGameMode* GameMode = GetWorld()->GetAuthGameMode<AHKGameMode>())
	{
		GameMode->NotifyPlayerDied();
	}
}
