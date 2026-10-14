// Copyright Epic Games, Inc. All Rights Reserved.

#include "Parkour/GameProjectSummer26ParkourCharacter.h"

#include "AbilitySystem/GameProjectSummer26AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameProjectSummer26.h"
#include "GameProjectSummer26GameplayTags.h"
#include "GameProjectSummer26PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Parkour/GameProjectSummer26ParkourMovementComponent.h"
#include "Parkour/GameProjectSummer26ParkourSettings.h"
#include "Parkour/GameProjectSummer26ParkourTraversalComponent.h"
#include "UObject/StructOnScope.h"
#include "UObject/UnrealType.h"

AGameProjectSummer26ParkourCharacter::AGameProjectSummer26ParkourCharacter(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGameProjectSummer26ParkourMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	ParkourTraversalComponent =
		CreateDefaultSubobject<UGameProjectSummer26ParkourTraversalComponent>(TEXT("ParkourTraversalComponent"));

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->GetNavAgentPropertiesRef().bCanCrouch = true;
	}
}

void AGameProjectSummer26ParkourCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ApplyParkourSettings();
	UE_LOG(
		LogGameProjectSummer26,
		Verbose,
		TEXT("Sample traversal bridge on %s: %s"),
		*GetNameSafe(this),
		HasSampleTraversalSupport() ? TEXT("ready") : TEXT("unavailable"));
}

void AGameProjectSummer26ParkourCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();
}

void AGameProjectSummer26ParkourCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
}

UGameProjectSummer26ParkourMovementComponent*
AGameProjectSummer26ParkourCharacter::GetParkourMovementComponent() const
{
	return Cast<UGameProjectSummer26ParkourMovementComponent>(GetCharacterMovement());
}

UGameProjectSummer26AbilitySystemComponent*
AGameProjectSummer26ParkourCharacter::GetGameAbilitySystemComponent() const
{
	const AGameProjectSummer26PlayerState* GamePlayerState =
		GetPlayerState<AGameProjectSummer26PlayerState>();
	return GamePlayerState ? GamePlayerState->GetGameAbilitySystemComponent() : nullptr;
}

void AGameProjectSummer26ParkourCharacter::HandleTraversalInputPressed()
{
	if (IsHanging())
	{
		JumpAwayFromHang();
		return;
	}

	if (IsSampleTraversalActive())
	{
		return;
	}

	if (UGameProjectSummer26AbilitySystemComponent* ASC = GetGameAbilitySystemComponent())
	{
		ASC->AbilityInputTagPressed(GameProjectSummer26GameplayTags::Input_Ability_Traversal);
		ASC->ProcessAbilityInput(0.0f, false);
		ASC->AbilityInputTagReleased(GameProjectSummer26GameplayTags::Input_Ability_Traversal);
		ASC->ProcessAbilityInput(0.0f, false);
		return;
	}

	Jump();
}

UActorComponent* AGameProjectSummer26ParkourCharacter::FindSampleTraversalComponent() const
{
	TInlineComponentArray<UActorComponent*> Components;
	GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component && Component->FindFunction(TEXT("TryTraversalAction")))
		{
			return Component;
		}
	}

	return nullptr;
}

bool AGameProjectSummer26ParkourCharacter::HasSampleTraversalSupport() const
{
	const UActorComponent* Component = FindSampleTraversalComponent();
	const UFunction* TryFunction = Component ? Component->FindFunction(TEXT("TryTraversalAction")) : nullptr;
	const UFunction* GetInputsFunction = FindFunction(TEXT("GetTraversalCheckInputs"));
	const FStructProperty* InputsProperty = TryFunction
		? FindFProperty<FStructProperty>(TryFunction, TEXT("Inputs"))
		: nullptr;
	const FStructProperty* ReturnProperty = GetInputsFunction
		? FindFProperty<FStructProperty>(GetInputsFunction, TEXT("ReturnValue"))
		: nullptr;

	return InputsProperty && ReturnProperty && InputsProperty->Struct == ReturnProperty->Struct;
}

bool AGameProjectSummer26ParkourCharacter::TryStartSampleTraversal(
	bool& bOutTraversalCheckFailed,
	bool& bOutMontageSelectionFailed)
{
	bOutTraversalCheckFailed = true;
	bOutMontageSelectionFailed = true;

	UActorComponent* Component = FindSampleTraversalComponent();
	UFunction* TryFunction = Component ? Component->FindFunction(TEXT("TryTraversalAction")) : nullptr;
	UFunction* GetInputsFunction = FindFunction(TEXT("GetTraversalCheckInputs"));
	FStructProperty* InputsProperty = TryFunction
		? FindFProperty<FStructProperty>(TryFunction, TEXT("Inputs"))
		: nullptr;
	FStructProperty* ReturnProperty = GetInputsFunction
		? FindFProperty<FStructProperty>(GetInputsFunction, TEXT("ReturnValue"))
		: nullptr;
	FBoolProperty* CheckFailedProperty = TryFunction
		? FindFProperty<FBoolProperty>(TryFunction, TEXT("TraversalCheckFailed"))
		: nullptr;
	FBoolProperty* MontageFailedProperty = TryFunction
		? FindFProperty<FBoolProperty>(TryFunction, TEXT("MontageSelectionFailed"))
		: nullptr;

	if (!Component || !TryFunction || !GetInputsFunction || !InputsProperty || !ReturnProperty ||
		!CheckFailedProperty || !MontageFailedProperty || InputsProperty->Struct != ReturnProperty->Struct)
	{
		UE_LOG(LogGameProjectSummer26, Warning, TEXT("Sample traversal bridge is unavailable on %s."), *GetNameSafe(this));
		return false;
	}

	FStructOnScope GetInputsParameters(GetInputsFunction);
	ProcessEvent(GetInputsFunction, GetInputsParameters.GetStructMemory());

	FStructOnScope TryParameters(TryFunction);
	void* InputDestination = InputsProperty->ContainerPtrToValuePtr<void>(TryParameters.GetStructMemory());
	const void* InputSource = ReturnProperty->ContainerPtrToValuePtr<void>(GetInputsParameters.GetStructMemory());
	InputsProperty->Struct->CopyScriptStruct(InputDestination, InputSource);

	Component->ProcessEvent(TryFunction, TryParameters.GetStructMemory());
	bOutTraversalCheckFailed = CheckFailedProperty->GetPropertyValue_InContainer(TryParameters.GetStructMemory());
	bOutMontageSelectionFailed = MontageFailedProperty->GetPropertyValue_InContainer(TryParameters.GetStructMemory());
	return !bOutTraversalCheckFailed && !bOutMontageSelectionFailed;
}

bool AGameProjectSummer26ParkourCharacter::IsSampleTraversalActive() const
{
	const UActorComponent* Component = FindSampleTraversalComponent();
	const FBoolProperty* ActiveProperty = Component
		? FindFProperty<FBoolProperty>(Component->GetClass(), TEXT("DoingTraversalAction"))
		: nullptr;
	return Component && ActiveProperty && ActiveProperty->GetPropertyValue_InContainer(Component);
}

bool AGameProjectSummer26ParkourCharacter::TryStartLedgeHang()
{
	UGameProjectSummer26ParkourMovementComponent* Movement = GetParkourMovementComponent();
	if (!Movement || !Movement->IsFalling() || !ParkourTraversalComponent)
	{
		return false;
	}

	FGameProjectSummer26HangTarget HangTarget;
	return ParkourTraversalComponent->QueryLedgeGrab(HangTarget) &&
		Movement->StartHanging(HangTarget);
}

bool AGameProjectSummer26ParkourCharacter::IsHanging() const
{
	const UGameProjectSummer26ParkourMovementComponent* Movement =
		GetParkourMovementComponent();
	return Movement && Movement->IsHanging();
}

bool AGameProjectSummer26ParkourCharacter::TryClimbUpFromHang()
{
	UGameProjectSummer26ParkourMovementComponent* Movement = GetParkourMovementComponent();
	return Movement && Movement->TryClimbUp();
}

bool AGameProjectSummer26ParkourCharacter::JumpAwayFromHang()
{
	UGameProjectSummer26ParkourMovementComponent* Movement = GetParkourMovementComponent();
	return Movement && Movement->JumpAwayFromHang();
}

void AGameProjectSummer26ParkourCharacter::DropFromHang()
{
	if (UGameProjectSummer26ParkourMovementComponent* Movement = GetParkourMovementComponent())
	{
		Movement->DropFromHang();
	}
}

void AGameProjectSummer26ParkourCharacter::RefreshHangAnimation()
{
	UGameProjectSummer26ParkourMovementComponent* Movement = GetParkourMovementComponent();
	if (!Movement || !Movement->IsHanging())
	{
		StopHangAnimation();
		return;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	const UGameProjectSummer26ParkourSettings* Settings = Movement->GetParkourSettings();
	if (!AnimInstance || !Settings)
	{
		return;
	}
	ApplyHangCameraOverride();

	const EGameProjectSummer26HangAnimationState NewState = Movement->GetHangAnimationState();
	if (NewState == ActiveHangAnimationState && ActiveHangMontage)
	{
		UpdateHangHandAlignment();
		return;
	}

	const TSoftObjectPtr<UAnimSequenceBase>* AnimationAsset = nullptr;
	switch (NewState)
	{
	case EGameProjectSummer26HangAnimationState::Idle:
		AnimationAsset = &Settings->HangIdleAnimation;
		break;
	case EGameProjectSummer26HangAnimationState::ShimmyLeft:
		AnimationAsset = &Settings->HangShimmyLeftAnimation;
		break;
	case EGameProjectSummer26HangAnimationState::ShimmyRight:
		AnimationAsset = &Settings->HangShimmyRightAnimation;
		break;
	case EGameProjectSummer26HangAnimationState::ClimbUp:
		AnimationAsset = &Settings->HangClimbUpAnimation;
		break;
	default:
		StopHangAnimation();
		return;
	}

	UAnimSequenceBase* Sequence = AnimationAsset ? AnimationAsset->LoadSynchronous() : nullptr;
	if (!Sequence)
	{
		UE_LOG(LogGameProjectSummer26, Warning, TEXT("HANG_ANIM missing asset for state %d."), static_cast<int32>(NewState));
		return;
	}

	StopHangAnimation(Settings->HangAnimationBlendOutTime, false);
	if (!bHasHangRootMotionOverride)
	{
		PreviousRootMotionMode = static_cast<uint8>(AnimInstance->RootMotionMode);
		AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		bHasHangRootMotionOverride = true;
	}

	const bool bLoop = NewState != EGameProjectSummer26HangAnimationState::ClimbUp;
	const float StartTime = NewState == EGameProjectSummer26HangAnimationState::Idle
		? FMath::Clamp(Settings->HangIdleStartTime, 0.0f, Sequence->GetPlayLength())
		: 0.0f;
	ActiveHangMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
		Sequence,
		TEXT("DefaultSlot"),
		Settings->HangAnimationBlendInTime,
		Settings->HangAnimationBlendOutTime,
		Settings->HangAnimationPlayRate,
		bLoop ? 9999 : 1,
		-1.0f,
		StartTime);
	ActiveHangAnimationState = ActiveHangMontage
		? NewState
		: EGameProjectSummer26HangAnimationState::None;
	UE_LOG(
		LogGameProjectSummer26,
		Display,
		TEXT("HANG_ANIM %s (%s)."),
		ActiveHangMontage ? TEXT("PLAY") : TEXT("FAILED"),
		*GetNameSafe(Sequence));
	UpdateHangHandAlignment();
}

void AGameProjectSummer26ParkourCharacter::UpdateHangHandAlignment()
{
	UGameProjectSummer26ParkourMovementComponent* Movement = GetParkourMovementComponent();
	USkeletalMeshComponent* MeshComponent = GetMesh();
	const UGameProjectSummer26ParkourSettings* Settings =
		Movement ? Movement->GetParkourSettings() : nullptr;
	if (!Movement || !Movement->IsHanging() || !MeshComponent || !Settings ||
		!Settings->bAlignHangHandsToLedge)
	{
		return;
	}

	const int32 LeftHandIndex = MeshComponent->GetBoneIndex(Settings->HangLeftHandBone);
	const int32 RightHandIndex = MeshComponent->GetBoneIndex(Settings->HangRightHandBone);
	if (LeftHandIndex == INDEX_NONE || RightHandIndex == INDEX_NONE)
	{
		UE_LOG(
			LogGameProjectSummer26,
			Warning,
			TEXT("HANG_ANIM hand alignment skipped: missing bones '%s' or '%s'."),
			*Settings->HangLeftHandBone.ToString(),
			*Settings->HangRightHandBone.ToString());
		return;
	}

	if (!bHasSavedHangMeshRelativeLocation)
	{
		SavedHangMeshRelativeLocation = MeshComponent->GetRelativeLocation();
		bHasSavedHangMeshRelativeLocation = true;
	}

	const FGameProjectSummer26HangTarget HangTarget = Movement->GetHangTarget();
	const FVector CurrentHandsMidpoint =
		(MeshComponent->GetBoneLocation(Settings->HangLeftHandBone, EBoneSpaces::WorldSpace) +
			MeshComponent->GetBoneLocation(Settings->HangRightHandBone, EBoneSpaces::WorldSpace)) * 0.5f;
	const FVector DesiredHandsMidpoint = HangTarget.LedgeLocation +
		HangTarget.WallNormal.GetSafeNormal() * Settings->HangHandWallOffset +
		FVector::UpVector *
			(Settings->HangHandVerticalOffset - Settings->HangHandUndersideOffset);
	const FVector TargetMeshWorldLocation =
		MeshComponent->GetComponentLocation() + (DesiredHandsMidpoint - CurrentHandsMidpoint);
	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;
	const FVector NewMeshWorldLocation = Settings->HangHandAlignmentInterpSpeed > 0.0f
		? FMath::VInterpTo(
			MeshComponent->GetComponentLocation(),
			TargetMeshWorldLocation,
			DeltaSeconds,
			Settings->HangHandAlignmentInterpSpeed)
		: TargetMeshWorldLocation;
	MeshComponent->SetWorldLocation(NewMeshWorldLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void AGameProjectSummer26ParkourCharacter::StopHangAnimation(
	const float BlendOutTime,
	const bool bRestoreMeshAlignment)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	if (AnimInstance && ActiveHangMontage)
	{
		AnimInstance->Montage_Stop(BlendOutTime, ActiveHangMontage);
	}

	ActiveHangMontage = nullptr;
	ActiveHangAnimationState = EGameProjectSummer26HangAnimationState::None;
	if (AnimInstance && bHasHangRootMotionOverride)
	{
		AnimInstance->SetRootMotionMode(static_cast<ERootMotionMode::Type>(PreviousRootMotionMode));
	}
	bHasHangRootMotionOverride = false;
	if (bRestoreMeshAlignment)
	{
		ResetHangMeshAlignment();
		RestoreHangCameraOverride();
	}
}

void AGameProjectSummer26ParkourCharacter::ResetHangMeshAlignment()
{
	if (bHasSavedHangMeshRelativeLocation)
	{
		if (USkeletalMeshComponent* MeshComponent = GetMesh())
		{
			MeshComponent->SetRelativeLocation(
				SavedHangMeshRelativeLocation,
				false,
				nullptr,
				ETeleportType::TeleportPhysics);
		}
	}
	bHasSavedHangMeshRelativeLocation = false;
}

void AGameProjectSummer26ParkourCharacter::ApplyHangCameraOverride()
{
	if (!SavedHangSpringArms.IsEmpty())
	{
		return;
	}

	const UGameProjectSummer26ParkourSettings* Settings =
		GetParkourMovementComponent() ? GetParkourMovementComponent()->GetParkourSettings() : nullptr;
	if (!Settings || !Settings->bApplyHangSpringArmCameraOverride)
	{
		return;
	}

	TInlineComponentArray<USpringArmComponent*> SpringArms;
	GetComponents(SpringArms);
	for (USpringArmComponent* SpringArm : SpringArms)
	{
		if (!SpringArm)
		{
			continue;
		}

		FHangSpringArmState& SavedState = SavedHangSpringArms.AddDefaulted_GetRef();
		SavedState.Component = SpringArm;
		SavedState.TargetArmLength = SpringArm->TargetArmLength;
		SavedState.TargetOffset = SpringArm->TargetOffset;
		SavedState.bDoCollisionTest = SpringArm->bDoCollisionTest;
		SpringArm->TargetArmLength = Settings->HangCameraArmLength;
		SpringArm->TargetOffset.Z += Settings->HangCameraTargetOffsetZ;
		// A hang places the capsule close to a wall. Never turn collision off here:
		// that would allow a SpringArm-driven camera to pass through the ledge.
		SpringArm->bDoCollisionTest = true;
	}

	if (!SavedHangSpringArms.IsEmpty())
	{
		UE_LOG(LogGameProjectSummer26, Display, TEXT("HANG_CAMERA SpringArm override applied."));
	}
}

void AGameProjectSummer26ParkourCharacter::RestoreHangCameraOverride()
{
	for (const FHangSpringArmState& SavedState : SavedHangSpringArms)
	{
		if (USpringArmComponent* SpringArm = SavedState.Component.Get())
		{
			SpringArm->TargetArmLength = SavedState.TargetArmLength;
			SpringArm->TargetOffset = SavedState.TargetOffset;
			SpringArm->bDoCollisionTest = SavedState.bDoCollisionTest;
		}
	}
	SavedHangSpringArms.Reset();
}

void AGameProjectSummer26ParkourCharacter::ApplyParkourSettings()
{
	if (UGameProjectSummer26ParkourMovementComponent* Movement = GetParkourMovementComponent())
	{
		Movement->SetParkourSettings(ParkourSettings);
	}

	if (ParkourTraversalComponent)
	{
		ParkourTraversalComponent->SetParkourSettings(ParkourSettings);
	}
}

void AGameProjectSummer26ParkourCharacter::InitializeAbilitySystem()
{
	if (AGameProjectSummer26PlayerState* GamePlayerState =
		GetPlayerState<AGameProjectSummer26PlayerState>())
	{
		GamePlayerState->InitializeAbilitySystem(this);
	}
}
