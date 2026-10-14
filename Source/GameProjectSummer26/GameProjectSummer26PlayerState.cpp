// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameProjectSummer26PlayerState.h"

#include "AbilitySystem/GameProjectSummer26AbilitySystemComponent.h"
#include "AbilitySystem/GameProjectSummer26AttributeSet.h"
#include "AbilitySystem/GameProjectSummer26DefaultAttributesEffect.h"
#include "AbilitySystem/GameProjectSummer26DodgeAbility.h"
#include "AbilitySystem/GameProjectSummer26EnduranceRegenEffect.h"
#include "AbilitySystem/GameProjectSummer26SprintAbility.h"
#include "AbilitySystem/GameProjectSummer26StaminaRegenEffect.h"
#include "AbilitySystem/GameProjectSummer26TraversalAbility.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Blueprint/UserWidget.h"
#include "GameProjectSummer26.h"
#include "GameProjectSummer26GameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffect.h"
#include "InputCoreTypes.h"
#include "UI/GameProjectSummer26StatusWidget.h"

AGameProjectSummer26PlayerState::AGameProjectSummer26PlayerState()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = CreateDefaultSubobject<UGameProjectSummer26AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UGameProjectSummer26AttributeSet>(TEXT("CoreAttributeSet"));
	DefaultAttributesEffect = UGameProjectSummer26DefaultAttributesEffect::StaticClass();
	StaminaRegenerationEffect = UGameProjectSummer26StaminaRegenEffect::StaticClass();
	EnduranceRegenerationEffect = UGameProjectSummer26EnduranceRegenEffect::StaticClass();
	FallbackStatusWidgetClass = UGameProjectSummer26StatusWidget::StaticClass();

	FGameProjectSummer26StartupAbility& SprintAbility = StartupAbilities.AddDefaulted_GetRef();
	SprintAbility.AbilityClass = UGameProjectSummer26SprintAbility::StaticClass();
	SprintAbility.InputTag = GameProjectSummer26GameplayTags::Input_Ability_Primary;

	FGameProjectSummer26StartupAbility& DodgeAbility = StartupAbilities.AddDefaulted_GetRef();
	DodgeAbility.AbilityClass = UGameProjectSummer26DodgeAbility::StaticClass();
	DodgeAbility.InputTag = GameProjectSummer26GameplayTags::Input_Ability_Dodge;

	FGameProjectSummer26StartupAbility& TraversalAbility = StartupAbilities.AddDefaulted_GetRef();
	TraversalAbility.AbilityClass = UGameProjectSummer26TraversalAbility::StaticClass();
	TraversalAbility.InputTag = GameProjectSummer26GameplayTags::Input_Ability_Traversal;
}

UAbilitySystemComponent* AGameProjectSummer26PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGameProjectSummer26PlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			ResourceRegenerationTimer,
			this,
			&AGameProjectSummer26PlayerState::RegenerateResources,
			0.2f,
			true,
			0.2f);
	}
}

void AGameProjectSummer26PlayerState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateGenericPawnBridge(DeltaSeconds);
}

void AGameProjectSummer26PlayerState::InitializeAbilitySystem(AActor* AvatarActor)
{
	if (!AbilitySystemComponent || !AvatarActor)
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, AvatarActor);

	if (HasAuthority())
	{
		ApplyDefaultAttributes();
		GrantStartupAbilities();
	}
}

void AGameProjectSummer26PlayerState::ApplyDefaultAttributes()
{
	if (bDefaultAttributesApplied || !DefaultAttributesEffect || !AbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1.0f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogGameProjectSummer26, Error, TEXT("Failed to create the default attributes GameplayEffect spec for %s."), *GetNameSafe(this));
		return;
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	bDefaultAttributesApplied = true;
}

void AGameProjectSummer26PlayerState::GrantStartupAbilities()
{
	if (bStartupAbilitiesGranted || !AbilitySystemComponent)
	{
		return;
	}

	for (const FGameProjectSummer26StartupAbility& StartupAbility : StartupAbilities)
	{
		if (!StartupAbility.AbilityClass)
		{
			continue;
		}

		bool bAlreadyGranted = false;
		for (const FGameplayAbilitySpec& ExistingSpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (ExistingSpec.Ability && ExistingSpec.Ability->GetClass() == StartupAbility.AbilityClass)
			{
				bAlreadyGranted = true;
				break;
			}
		}

		if (bAlreadyGranted)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(StartupAbility.AbilityClass, 1);
		if (StartupAbility.InputTag.IsValid())
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(StartupAbility.InputTag);
		}

		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}

	bStartupAbilitiesGranted = true;
}

void AGameProjectSummer26PlayerState::RegenerateResources()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	if (StaminaRegenerationEffect &&
		!AbilitySystemComponent->HasMatchingGameplayTag(GameProjectSummer26GameplayTags::State_Movement_Sprinting) &&
		AttributeSet->GetStamina() < AttributeSet->GetMaxStamina())
	{
		FGameplayEffectContextHandle StaminaContext = AbilitySystemComponent->MakeEffectContext();
		StaminaContext.AddSourceObject(this);
		const FGameplayEffectSpecHandle StaminaSpec =
			AbilitySystemComponent->MakeOutgoingSpec(StaminaRegenerationEffect, 1.0f, StaminaContext);
		if (StaminaSpec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*StaminaSpec.Data.Get());
		}
	}

	if (!EnduranceRegenerationEffect || AttributeSet->GetEndurance() >= AttributeSet->GetMaxEndurance())
	{
		return;
	}

	FGameplayEffectContextHandle EnduranceContext = AbilitySystemComponent->MakeEffectContext();
	EnduranceContext.AddSourceObject(this);
	const FGameplayEffectSpecHandle EnduranceSpec =
		AbilitySystemComponent->MakeOutgoingSpec(EnduranceRegenerationEffect, 1.0f, EnduranceContext);
	if (EnduranceSpec.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EnduranceSpec.Data.Get());
	}
}

void AGameProjectSummer26PlayerState::UpdateGenericPawnBridge(float DeltaSeconds)
{
	APawn* CurrentPawn = GetPawn();
	if (!CurrentPawn)
	{
		if (ACharacter* PreviousCharacter = Cast<ACharacter>(LastInitializedPawn.Get()))
		{
			EndFallbackSlide(PreviousCharacter);
		}
		LastInitializedPawn.Reset();
		SetFallbackSprintInput(false);
		SetFallbackDodgeInput(false);
		bFallbackControlPressed = false;
		return;
	}

	if (LastInitializedPawn.Get() != CurrentPawn)
	{
		if (ACharacter* PreviousCharacter = Cast<ACharacter>(LastInitializedPawn.Get()))
		{
			EndFallbackSlide(PreviousCharacter);
		}
		InitializeAbilitySystem(CurrentPawn);
		if (ACharacter* Character = Cast<ACharacter>(CurrentPawn))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				// The Game Animation Sample pawn handles its old crouch in Blueprint.
				// Enable native Character crouching for the unified Ctrl crouch/slide system.
				Movement->GetNavAgentPropertiesRef().bCanCrouch = true;
			}
		}
		LastInitializedPawn = CurrentPawn;
		bFallbackSprintPressed = false;
		bFallbackDodgePressed = false;
		bFallbackControlPressed = false;
		bFallbackSliding = false;
		bFallbackTurnAroundPressed = false;

		UE_LOG(
			LogGameProjectSummer26,
			Display,
			TEXT("Generic GAS bridge initialized. PlayerState=%s Avatar=%s Abilities=%d Health=%.0f Stamina=%.0f Mana=%.0f Endurance=%.0f"),
			*GetNameSafe(this),
			*GetNameSafe(CurrentPawn),
			AbilitySystemComponent ? AbilitySystemComponent->GetActivatableAbilities().Num() : 0,
			AttributeSet ? AttributeSet->GetHealth() : 0.0f,
			AttributeSet ? AttributeSet->GetStamina() : 0.0f,
			AttributeSet ? AttributeSet->GetMana() : 0.0f,
			AttributeSet ? AttributeSet->GetEndurance() : 0.0f);
	}

	if (CurrentPawn->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(CurrentPawn->GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	if (!FallbackStatusWidget && FallbackStatusWidgetClass)
	{
		FallbackStatusWidget = CreateWidget<UGameProjectSummer26StatusWidget>(PlayerController, FallbackStatusWidgetClass);
		if (FallbackStatusWidget)
		{
			FallbackStatusWidget->AddToPlayerScreen(10);
			UE_LOG(LogGameProjectSummer26, Display, TEXT("Generic GAS status widget added for %s."), *GetNameSafe(CurrentPawn));
		}
	}

#if !UE_BUILD_SHIPPING
	if (!bDodgeSmokeTestScheduled && FParse::Param(FCommandLine::Get(), TEXT("GASDodgeSmokeTest")))
	{
		bDodgeSmokeTestScheduled = true;
		GetWorldTimerManager().SetTimerForNextTick(this, &AGameProjectSummer26PlayerState::RunDodgeSmokeTest);
	}
	if (!bSlideSmokeTestScheduled && FParse::Param(FCommandLine::Get(), TEXT("GASSlideSmokeTest")))
	{
		bSlideSmokeTestScheduled = true;
		GetWorldTimerManager().SetTimerForNextTick(this, &AGameProjectSummer26PlayerState::RunSlideSmokeTest);
	}
#endif

	const bool bControlKeyDown = PlayerController->IsInputKeyDown(EKeys::LeftControl);
	const bool bSprintKeyDown = PlayerController->IsInputKeyDown(EKeys::LeftShift) &&
		!bFallbackSliding && !bFallbackControlPressed;
	if (bSprintKeyDown != bFallbackSprintPressed)
	{
		SetFallbackSprintInput(bSprintKeyDown);
	}

	const bool bDodgeKeyDown = PlayerController->IsInputKeyDown(EKeys::C);
	if (bDodgeKeyDown != bFallbackDodgePressed)
	{
		SetFallbackDodgeInput(bDodgeKeyDown);
	}

	if (!bFallbackControlInputOverriddenForTest && bControlKeyDown != bFallbackControlPressed)
	{
		SetFallbackControlInput(bControlKeyDown);
	}

	if (bFallbackSliding)
	{
		UpdateFallbackSlide(Cast<ACharacter>(CurrentPawn), DeltaSeconds);
	}

	// On a Finnish 102-key keyboard Unreal reports the physical < key as FKey("<").
	static const FKey TurnAroundKey(TEXT("<"));
	const bool bTurnAroundKeyDown = PlayerController->IsInputKeyDown(TurnAroundKey);
	if (bTurnAroundKeyDown && !bFallbackTurnAroundPressed)
	{
		HandleFallbackTurnAround();
	}
	bFallbackTurnAroundPressed = bTurnAroundKeyDown;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ProcessAbilityInput(DeltaSeconds, false);
	}
}

void AGameProjectSummer26PlayerState::SetFallbackSprintInput(bool bPressed)
{
	if (!AbilitySystemComponent || bFallbackSprintPressed == bPressed)
	{
		return;
	}

	bFallbackSprintPressed = bPressed;
	if (bPressed)
	{
		AbilitySystemComponent->AbilityInputTagPressed(GameProjectSummer26GameplayTags::Input_Ability_Primary);
	}
	else
	{
		AbilitySystemComponent->AbilityInputTagReleased(GameProjectSummer26GameplayTags::Input_Ability_Primary);
	}
}

void AGameProjectSummer26PlayerState::SetFallbackDodgeInput(bool bPressed)
{
	if (!AbilitySystemComponent || bFallbackDodgePressed == bPressed)
	{
		return;
	}

	bFallbackDodgePressed = bPressed;
	if (bPressed)
	{
		AbilitySystemComponent->AbilityInputTagPressed(GameProjectSummer26GameplayTags::Input_Ability_Dodge);
	}
	else
	{
		AbilitySystemComponent->AbilityInputTagReleased(GameProjectSummer26GameplayTags::Input_Ability_Dodge);
	}
}

void AGameProjectSummer26PlayerState::SetFallbackControlInput(bool bPressed)
{
	if (bFallbackControlPressed == bPressed)
	{
		return;
	}

	bFallbackControlPressed = bPressed;
	ACharacter* Character = Cast<ACharacter>(GetPawn());
	if (!Character)
	{
		return;
	}

	if (bPressed)
	{
		if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(
			GameProjectSummer26GameplayTags::State_Movement_Traversing))
		{
			return;
		}

		const bool bIsSprinting =
			bFallbackSprintPressed ||
			(AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(
				GameProjectSummer26GameplayTags::State_Movement_Sprinting));
		if (!bIsSprinting || !StartFallbackSlide(Character))
		{
			Character->Crouch();
		}
	}
	else if (!bFallbackSliding)
	{
		Character->UnCrouch();
	}
}

bool AGameProjectSummer26PlayerState::StartFallbackSlide(ACharacter* Character)
{
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(
		GameProjectSummer26GameplayTags::State_Movement_Traversing))
	{
		return false;
	}

	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement || !Movement->IsMovingOnGround())
	{
		return false;
	}

	FVector HorizontalVelocity = Movement->Velocity;
	HorizontalVelocity.Z = 0.0f;
	constexpr float MinimumSlideStartSpeed = 500.0f;
	if (HorizontalVelocity.Size() < MinimumSlideStartSpeed)
	{
		return false;
	}

	FallbackSlideDirection = HorizontalVelocity.GetSafeNormal();
	constexpr float SlideInitialBoost = 300.0f;
	constexpr float MinimumSlideInitialSpeed = 1150.0f;
	FallbackSlideSpeed = FMath::Max(HorizontalVelocity.Size() + SlideInitialBoost, MinimumSlideInitialSpeed);

	// End the sprint ability before saving the normal movement settings.
	SetFallbackSprintInput(false);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ProcessAbilityInput(0.0f, false);
		AbilitySystemComponent->AddLooseGameplayTag(GameProjectSummer26GameplayTags::State_Movement_Sliding);
	}

	SavedGroundFriction = Movement->GroundFriction;
	SavedBrakingDecelerationWalking = Movement->BrakingDecelerationWalking;
	SavedBrakingFrictionFactor = Movement->BrakingFrictionFactor;
	SavedMaxAcceleration = Movement->MaxAcceleration;
	SavedMaxWalkSpeed = Movement->MaxWalkSpeed;
	SavedMaxWalkSpeedCrouched = Movement->MaxWalkSpeedCrouched;
	bSavedUseSeparateBrakingFriction = Movement->bUseSeparateBrakingFriction;

	Movement->GroundFriction = 0.0f;
	Movement->BrakingDecelerationWalking = 0.0f;
	Movement->BrakingFrictionFactor = 0.0f;
	Movement->MaxAcceleration = 0.0f;
	Movement->MaxWalkSpeed = FMath::Max(Movement->MaxWalkSpeed, FallbackSlideSpeed);
	Movement->MaxWalkSpeedCrouched = FMath::Max(Movement->MaxWalkSpeedCrouched, FallbackSlideSpeed);
	Movement->bUseSeparateBrakingFriction = true;
	const FVector BoostedHorizontalVelocity = FallbackSlideDirection * FallbackSlideSpeed;
	Movement->Velocity.X = BoostedHorizontalVelocity.X;
	Movement->Velocity.Y = BoostedHorizontalVelocity.Y;
	Character->Crouch();
	bFallbackSliding = true;
	return true;
}

void AGameProjectSummer26PlayerState::UpdateFallbackSlide(ACharacter* Character, float DeltaSeconds)
{
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement || !Movement->IsMovingOnGround())
	{
		EndFallbackSlide(Character);
		return;
	}

	constexpr float SlideDeceleration = 600.0f;
	constexpr float SlideEndSpeed = 50.0f;
	FallbackSlideSpeed = FMath::Max(0.0f, FallbackSlideSpeed - SlideDeceleration * DeltaSeconds);
	if (FallbackSlideSpeed <= SlideEndSpeed)
	{
		Movement->Velocity.X = 0.0f;
		Movement->Velocity.Y = 0.0f;
		EndFallbackSlide(Character);
		return;
	}

	const FVector HorizontalVelocity = FallbackSlideDirection * FallbackSlideSpeed;
	Movement->Velocity.X = HorizontalVelocity.X;
	Movement->Velocity.Y = HorizontalVelocity.Y;
}

void AGameProjectSummer26PlayerState::EndFallbackSlide(ACharacter* Character)
{
	if (!bFallbackSliding)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr)
	{
		Movement->GroundFriction = SavedGroundFriction;
		Movement->BrakingDecelerationWalking = SavedBrakingDecelerationWalking;
		Movement->BrakingFrictionFactor = SavedBrakingFrictionFactor;
		Movement->MaxAcceleration = SavedMaxAcceleration;
		Movement->MaxWalkSpeed = SavedMaxWalkSpeed;
		Movement->MaxWalkSpeedCrouched = SavedMaxWalkSpeedCrouched;
		Movement->bUseSeparateBrakingFriction = bSavedUseSeparateBrakingFriction;
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(GameProjectSummer26GameplayTags::State_Movement_Sliding);
	}
	bFallbackSliding = false;
	FallbackSlideSpeed = 0.0f;
	if (Character && !bFallbackControlPressed)
	{
		Character->UnCrouch();
	}
}

void AGameProjectSummer26PlayerState::HandleFallbackTurnAround()
{
	APawn* CurrentPawn = GetPawn();
	APlayerController* PlayerController = CurrentPawn ? Cast<APlayerController>(CurrentPawn->GetController()) : nullptr;
	if (!CurrentPawn || !PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	FRotator PawnRotation = CurrentPawn->GetActorRotation();
	PawnRotation.Yaw = FRotator::NormalizeAxis(PawnRotation.Yaw + 180.0f);
	CurrentPawn->SetActorRotation(PawnRotation);

	FRotator ControlRotation = PlayerController->GetControlRotation();
	ControlRotation.Yaw = FRotator::NormalizeAxis(ControlRotation.Yaw + 180.0f);
	PlayerController->SetControlRotation(ControlRotation);
}

#if !UE_BUILD_SHIPPING
void AGameProjectSummer26PlayerState::RunDodgeSmokeTest()
{
	ACharacter* Character = Cast<ACharacter>(GetPawn());
	if (!Character || !AbilitySystemComponent || !AttributeSet)
	{
		UE_LOG(LogGameProjectSummer26, Error, TEXT("DODGE_SMOKE FAIL: missing Character, ASC or AttributeSet."));
		return;
	}

	const float EnduranceBefore = AttributeSet->GetEndurance();
	DodgeSmokeStartLocation = Character->GetActorLocation();
	DodgeSmokeStartForward = Character->GetActorForwardVector().GetSafeNormal2D();

	SetFallbackDodgeInput(true);
	AbilitySystemComponent->ProcessAbilityInput(0.0f, false);
	SetFallbackDodgeInput(false);
	AbilitySystemComponent->ProcessAbilityInput(0.0f, false);

	DodgeSmokeEnduranceAfterActivation = AttributeSet->GetEndurance();
	const bool bActivationPassed = FMath::IsNearlyEqual(
		EnduranceBefore - DodgeSmokeEnduranceAfterActivation,
		50.0f,
		0.1f);

	UE_LOG(
		LogGameProjectSummer26,
		Display,
		TEXT("DODGE_SMOKE %s activation: Endurance %.1f -> %.1f, cost %.1f. Movement is verified after the CharacterMovement tick."),
		bActivationPassed ? TEXT("PASS") : TEXT("FAIL"),
		EnduranceBefore,
		DodgeSmokeEnduranceAfterActivation,
		EnduranceBefore - DodgeSmokeEnduranceAfterActivation);

	GetWorldTimerManager().SetTimer(
		DodgeSmokeTestTimer,
		this,
		&AGameProjectSummer26PlayerState::FinishDodgeSmokeTest,
		1.0f,
		false);
}

void AGameProjectSummer26PlayerState::FinishDodgeSmokeTest()
{
	const ACharacter* Character = Cast<ACharacter>(GetPawn());
	if (!Character || !AttributeSet)
	{
		UE_LOG(LogGameProjectSummer26, Error, TEXT("DODGE_SMOKE FAIL: missing Character or AttributeSet during regeneration check."));
		return;
	}

	const float EnduranceNow = AttributeSet->GetEndurance();
	const float Regenerated = EnduranceNow - DodgeSmokeEnduranceAfterActivation;
	const FVector HorizontalDisplacement =
		(Character->GetActorLocation() - DodgeSmokeStartLocation) * FVector(1.0f, 1.0f, 0.0f);
	const float DistanceMoved = HorizontalDisplacement.Size();
	const float ForwardDistance = FVector::DotProduct(HorizontalDisplacement, DodgeSmokeStartForward);
	const float EndHorizontalSpeed = Character->GetVelocity().Size2D();
	const bool bFinishPassed =
		Regenerated >= 8.0f && Regenerated <= 12.1f && DistanceMoved >= 260.0f && DistanceMoved <= 300.0f &&
		ForwardDistance < -50.0f && EndHorizontalSpeed <= 100.0f;

	UE_LOG(
		LogGameProjectSummer26,
		Display,
		TEXT("DODGE_SMOKE %s finish: regenerated %.1f in 1s, Endurance %.1f, distance moved %.1f, forward distance %.1f (negative is backward), end speed %.1f."),
		bFinishPassed ? TEXT("PASS") : TEXT("FAIL"),
		Regenerated,
		EnduranceNow,
		DistanceMoved,
		ForwardDistance,
		EndHorizontalSpeed);

	const float PawnYawBefore = Character->GetActorRotation().Yaw;
	const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	const float ControlYawBefore = PlayerController ? PlayerController->GetControlRotation().Yaw : 0.0f;
	HandleFallbackTurnAround();
	const float PawnYawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(PawnYawBefore, Character->GetActorRotation().Yaw));
	const float ControlYawDelta = PlayerController
		? FMath::Abs(FMath::FindDeltaAngleDegrees(ControlYawBefore, PlayerController->GetControlRotation().Yaw))
		: 0.0f;
	const FKey TurnAroundKey(TEXT("<"));
	const bool bTurnKeyRegistered = TurnAroundKey.IsValid();
	const bool bTurnPassed =
		bTurnKeyRegistered &&
		FMath::IsNearlyEqual(PawnYawDelta, 180.0f, 0.1f) &&
		FMath::IsNearlyEqual(ControlYawDelta, 180.0f, 0.1f);
	UE_LOG(
		LogGameProjectSummer26,
		Display,
		TEXT("TURN_AROUND_SMOKE %s: < key registered %s, pawn yaw delta %.1f, control yaw delta %.1f."),
		bTurnPassed ? TEXT("PASS") : TEXT("FAIL"),
		bTurnKeyRegistered ? TEXT("true") : TEXT("false"),
		PawnYawDelta,
		ControlYawDelta);
}

void AGameProjectSummer26PlayerState::RunSlideSmokeTest()
{
	ACharacter* Character = Cast<ACharacter>(GetPawn());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Character || !Movement || !AbilitySystemComponent || !Movement->IsMovingOnGround())
	{
		UE_LOG(LogGameProjectSummer26, Error, TEXT("SLIDE_SMOKE FAIL: missing grounded Character, movement component or ASC."));
		return;
	}

	bFallbackControlPressed = false;
	bFallbackControlInputOverriddenForTest = true;
	SetFallbackSprintInput(true);
	AbilitySystemComponent->ProcessAbilityInput(0.0f, false);
	Movement->Velocity = Character->GetActorForwardVector().GetSafeNormal2D() * 900.0f;
	const bool bSprintWasActive = AbilitySystemComponent->HasMatchingGameplayTag(
		GameProjectSummer26GameplayTags::State_Movement_Sprinting);

	SetFallbackControlInput(true);
	const bool bSlideStarted =
		bFallbackSliding &&
		Movement->bWantsToCrouch &&
		AbilitySystemComponent->HasMatchingGameplayTag(GameProjectSummer26GameplayTags::State_Movement_Sliding) &&
		!AbilitySystemComponent->HasMatchingGameplayTag(GameProjectSummer26GameplayTags::State_Movement_Sprinting);

	UE_LOG(
		LogGameProjectSummer26,
		Display,
		TEXT("SLIDE_SMOKE %s start: sprint active before Ctrl %s, sliding %s, crouch requested %s, speed %.1f."),
		(bSprintWasActive && bSlideStarted) ? TEXT("PASS") : TEXT("FAIL"),
		bSprintWasActive ? TEXT("true") : TEXT("false"),
		bFallbackSliding ? TEXT("true") : TEXT("false"),
		Movement->bWantsToCrouch ? TEXT("true") : TEXT("false"),
		Movement->Velocity.Size2D());

	GetWorldTimerManager().SetTimer(
		SlideSmokeTestTimer,
		this,
		&AGameProjectSummer26PlayerState::FinishSlideSmokeTest,
		2.0f,
		false);
}

void AGameProjectSummer26PlayerState::FinishSlideSmokeTest()
{
	ACharacter* Character = Cast<ACharacter>(GetPawn());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Character || !Movement || !AbilitySystemComponent)
	{
		UE_LOG(LogGameProjectSummer26, Error, TEXT("SLIDE_SMOKE FAIL: missing Character, movement component or ASC during finish."));
		return;
	}

	const float EndSpeed = Movement->Velocity.Size2D();
	const bool bSlideTagRemoved = !AbilitySystemComponent->HasMatchingGameplayTag(
		GameProjectSummer26GameplayTags::State_Movement_Sliding);
	const bool bSlideFinished = !bFallbackSliding && bSlideTagRemoved && EndSpeed <= 105.0f;
	const bool bControlStayedHeld = bFallbackControlPressed;

	SetFallbackControlInput(false);
	const bool bReleasedCrouchRequest = !Movement->bWantsToCrouch;
	UE_LOG(
		LogGameProjectSummer26,
		Display,
		TEXT("SLIDE_SMOKE %s finish: slide ended %s at speed %.1f, Ctrl stayed held %s, release cleared crouch request %s."),
		(bSlideFinished && bControlStayedHeld && bReleasedCrouchRequest)
			? TEXT("PASS")
			: TEXT("FAIL"),
		bSlideFinished ? TEXT("true") : TEXT("false"),
		EndSpeed,
		bControlStayedHeld ? TEXT("true") : TEXT("false"),
		bReleasedCrouchRequest ? TEXT("true") : TEXT("false"));

	GetWorldTimerManager().SetTimerForNextTick(this, &AGameProjectSummer26PlayerState::RunCrouchSmokeTest);
}

void AGameProjectSummer26PlayerState::RunCrouchSmokeTest()
{
	ACharacter* Character = Cast<ACharacter>(GetPawn());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement)
	{
		bFallbackControlInputOverriddenForTest = false;
		UE_LOG(LogGameProjectSummer26, Error, TEXT("CROUCH_SMOKE FAIL: missing CharacterMovement."));
		return;
	}

	Movement->Velocity.X = 0.0f;
	Movement->Velocity.Y = 0.0f;
	SetFallbackControlInput(true);
	const bool bNormalControlCrouches = Movement->bWantsToCrouch && !bFallbackSliding;
	SetFallbackControlInput(false);
	const bool bReleaseClearsCrouch = !Movement->bWantsToCrouch;
	bFallbackControlInputOverriddenForTest = false;

	UE_LOG(
		LogGameProjectSummer26,
		Display,
		TEXT("CROUCH_SMOKE %s: non-sprint Ctrl requested crouch %s, release cleared request %s."),
		(bNormalControlCrouches && bReleaseClearsCrouch) ? TEXT("PASS") : TEXT("FAIL"),
		bNormalControlCrouches ? TEXT("true") : TEXT("false"),
		bReleaseClearsCrouch ? TEXT("true") : TEXT("false"));
}
#endif
