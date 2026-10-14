// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameProjectSummer26Character.h"
#include "AbilitySystem/GameProjectSummer26AbilitySystemComponent.h"
#include "GameProjectSummer26GameplayTags.h"
#include "GameProjectSummer26PlayerState.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "GameProjectSummer26.h"

AGameProjectSummer26Character::AGameProjectSummer26Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

UAbilitySystemComponent* AGameProjectSummer26Character::GetAbilitySystemComponent() const
{
	if (const AGameProjectSummer26PlayerState* GamePlayerState = GetPlayerState<AGameProjectSummer26PlayerState>())
	{
		return GamePlayerState->GetAbilitySystemComponent();
	}

	return nullptr;
}

void AGameProjectSummer26Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();
}

void AGameProjectSummer26Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
}

void AGameProjectSummer26Character::InitializeAbilitySystem()
{
	if (AGameProjectSummer26PlayerState* GamePlayerState = GetPlayerState<AGameProjectSummer26PlayerState>())
	{
		GamePlayerState->InitializeAbilitySystem(this);
	}
}

void AGameProjectSummer26Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		if (!PrimaryAbilityAction)
		{
			PrimaryAbilityAction = NewObject<UInputAction>(this, TEXT("IA_Sprint_Runtime"));
			PrimaryAbilityAction->ValueType = EInputActionValueType::Boolean;

			RuntimeAbilityMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Abilities_Runtime"));
			RuntimeAbilityMappingContext->MapKey(PrimaryAbilityAction, EKeys::LeftShift);

			if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
				{
					Subsystem->AddMappingContext(RuntimeAbilityMappingContext, 10);
				}
			}
		}
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGameProjectSummer26Character::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AGameProjectSummer26Character::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGameProjectSummer26Character::Look);

		if (PrimaryAbilityAction)
		{
			EnhancedInputComponent->BindAction(PrimaryAbilityAction, ETriggerEvent::Started, this, &AGameProjectSummer26Character::PrimaryAbilityPressed);
			EnhancedInputComponent->BindAction(PrimaryAbilityAction, ETriggerEvent::Completed, this, &AGameProjectSummer26Character::PrimaryAbilityReleased);
			EnhancedInputComponent->BindAction(PrimaryAbilityAction, ETriggerEvent::Canceled, this, &AGameProjectSummer26Character::PrimaryAbilityReleased);
		}

		if (SecondaryAbilityAction)
		{
			EnhancedInputComponent->BindAction(SecondaryAbilityAction, ETriggerEvent::Started, this, &AGameProjectSummer26Character::SecondaryAbilityPressed);
			EnhancedInputComponent->BindAction(SecondaryAbilityAction, ETriggerEvent::Completed, this, &AGameProjectSummer26Character::SecondaryAbilityReleased);
			EnhancedInputComponent->BindAction(SecondaryAbilityAction, ETriggerEvent::Canceled, this, &AGameProjectSummer26Character::SecondaryAbilityReleased);
		}
	}
	else
	{
		UE_LOG(LogGameProjectSummer26, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AGameProjectSummer26Character::PrimaryAbilityPressed()
{
	AbilityInputTagPressed(GameProjectSummer26GameplayTags::Input_Ability_Primary);
}

void AGameProjectSummer26Character::PrimaryAbilityReleased()
{
	AbilityInputTagReleased(GameProjectSummer26GameplayTags::Input_Ability_Primary);
}

void AGameProjectSummer26Character::SecondaryAbilityPressed()
{
	AbilityInputTagPressed(GameProjectSummer26GameplayTags::Input_Ability_Secondary);
}

void AGameProjectSummer26Character::SecondaryAbilityReleased()
{
	AbilityInputTagReleased(GameProjectSummer26GameplayTags::Input_Ability_Secondary);
}

void AGameProjectSummer26Character::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UGameProjectSummer26AbilitySystemComponent* GameASC = Cast<UGameProjectSummer26AbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		GameASC->AbilityInputTagPressed(InputTag);
	}
}

void AGameProjectSummer26Character::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UGameProjectSummer26AbilitySystemComponent* GameASC = Cast<UGameProjectSummer26AbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		GameASC->AbilityInputTagReleased(InputTag);
	}
}

void AGameProjectSummer26Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AGameProjectSummer26Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AGameProjectSummer26Character::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AGameProjectSummer26Character::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AGameProjectSummer26Character::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AGameProjectSummer26Character::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
