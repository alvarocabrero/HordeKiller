#include "MyGameCharacter.h"
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
#include "MyGameGameMode.h"
#include "MyGameProjectile.h"
#include "UObject/ConstructorHelpers.h"

AMyGameCharacter::AMyGameCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// First person: the character turns with the camera yaw.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->JumpZVelocity = 600.f;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = 600.f;
	Movement->BrakingDecelerationWalking = 2000.f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

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

	ProjectileClass = AMyGameProjectile::StaticClass();
}

void AMyGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
}

void AMyGameCharacter::CreateDefaultInputAssets()
{
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

	// Move is (X = right, Y = forward). Keys produce X by default, so W/S are swizzled onto Y.
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

	AddSwizzle(Context->MapKey(MoveAction, EKeys::W));
	{
		FEnhancedActionKeyMapping& Back = Context->MapKey(MoveAction, EKeys::S);
		AddNegate(Back, true, false);
		AddSwizzle(Back);
	}
	Context->MapKey(MoveAction, EKeys::D);
	AddNegate(Context->MapKey(MoveAction, EKeys::A), true, false);

	// Mouse Y is inverted so that moving the mouse up looks up.
	AddNegate(Context->MapKey(LookAction, EKeys::Mouse2D), false, true);

	Context->MapKey(JumpAction, EKeys::SpaceBar);
	Context->MapKey(FireAction, EKeys::LeftMouseButton);
}

void AMyGameCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

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

void AMyGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	CreateDefaultInputAssets();

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyGameCharacter::Move);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyGameCharacter::Look);
		// Triggered fires every frame while held; FireInterval limits the actual rate.
		Input->BindAction(FireAction, ETriggerEvent::Triggered, this, &AMyGameCharacter::Fire);
	}
}

void AMyGameCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void AMyGameCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AMyGameCharacter::Fire()
{
	UWorld* World = GetWorld();
	if (bDead || !ProjectileClass || !World)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (Now - LastFireTime < FireInterval)
	{
		return;
	}
	LastFireTime = Now;

	const FRotator AimRotation = GetControlRotation();
	const FVector AimDirection = AimRotation.Vector();
	const FVector SpawnLocation = FirstPersonCamera->GetComponentLocation() + AimDirection * MuzzleDistance;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AMyGameProjectile* Projectile =
			World->SpawnActor<AMyGameProjectile>(ProjectileClass, SpawnLocation, AimRotation, Params))
	{
		Projectile->Launch(AimDirection);
	}
}

float AMyGameCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (bDead || Applied <= 0.f)
	{
		return 0.f;
	}

	Health = FMath::Max(0.f, Health - Applied);
	if (Health <= 0.f)
	{
		bDead = true;
		GetCharacterMovement()->DisableMovement();
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			DisableInput(PC);
		}
		if (AMyGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AMyGameGameMode>())
		{
			GameMode->NotifyPlayerDied();
		}
	}
	return Applied;
}
