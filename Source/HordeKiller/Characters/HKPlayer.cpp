// Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.

#include "Characters/HKPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Game/HKGameMode.h"
#include "Weapons/HKProjectile.h"
#include "UObject/ConstructorHelpers.h"

AHKPlayer::AHKPlayer()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// The body follows the view's yaw only, so the player faces where they aim; the boom handles pitch.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->JumpZVelocity = 600.f;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = 600.f; // Faster than the enemies (380).
	Movement->BrakingDecelerationWalking = 2000.f;

	// The boom copies the controller's full rotation and pulls in when something blocks the view.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->SocketOffset = CameraOffset;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Cosmetic weapon: a cube scaled into a bar, held in the right hand.
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
	GunMesh->SetupAttachment(GetMesh());
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

	// The player uses Quinn.
	BodyModel = FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
}

void AHKPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Read here so that values edited in a Blueprint apply.
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->SocketOffset = CameraOffset;

	// The hand only exists once the body model is on the mesh component, which happens in Super::BeginPlay.
	if (HasBodyModel())
	{
		GunMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_r"));
		GunMesh->SetRelativeLocation(FVector(-10.f, 4.f, 0.f));
	}
	else
	{
		GunMesh->SetVisibility(false);
	}
}

void AHKPlayer::CreateDefaultInputAssets()
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

void AHKPlayer::NotifyControllerChanged()
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

void AHKPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// May run before NotifyControllerChanged, so make sure the actions exist.
	CreateDefaultInputAssets();

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHKPlayer::Move);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHKPlayer::Look);

		// Called every frame while held; FireInterval limits the real rate.
		Input->BindAction(FireAction, ETriggerEvent::Triggered, this, &AHKPlayer::Fire);
	}
}

void AHKPlayer::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void AHKPlayer::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AHKPlayer::Fire()
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

	// The camera is off to one side, so shots leave the chest and converge on what the crosshair points at.
	const FVector Muzzle = GetActorLocation() + FVector(0.f, 0.f, MuzzleHeight);
	const FVector AimDirection = (GetAimPoint() - Muzzle).GetSafeNormal();
	const FRotator AimRotation = AimDirection.Rotation();
	// Along the aim line, clear of the player's capsule.
	const FVector SpawnLocation = Muzzle + AimDirection * MuzzleDistance;

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

FVector AHKPlayer::GetAimPoint() const
{
	const FVector ViewLocation = FollowCamera->GetComponentLocation();
	const FVector ViewDirection = FollowCamera->GetForwardVector();

	// Start level with the player, so nothing between the camera and the player is picked.
	const float ToPlayer = FVector::DotProduct(GetActorLocation() - ViewLocation, ViewDirection);
	const FVector Start = ViewLocation + ViewDirection * FMath::Max(0.f, ToPlayer);
	const FVector End = ViewLocation + ViewDirection * AimRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(HKAim), false, this);
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.ImpactPoint;
	}
	return End;
}

void AHKPlayer::HandleDeath()
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
