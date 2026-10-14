// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26DodgeAbility.h"

#include "AbilitySystem/GameProjectSummer26AttributeSet.h"
#include "AbilitySystem/GameProjectSummer26DodgeCostEffect.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameProjectSummer26.h"
#include "GameProjectSummer26GameplayTags.h"

UGameProjectSummer26DodgeAbility::UGameProjectSummer26DodgeAbility()
{
	ActivationPolicy = EGameProjectSummer26AbilityActivationPolicy::OnInputTriggered;
	CostGameplayEffectClass = UGameProjectSummer26DodgeCostEffect::StaticClass();
	DodgeCoastDuration = 0.01f;

	FGameplayTagContainer AssetTagContainer;
	AssetTagContainer.AddTag(GameProjectSummer26GameplayTags::Ability_Movement_Dodge);
	SetAssetTags(AssetTagContainer);
	ActivationOwnedTags.AddTag(GameProjectSummer26GameplayTags::State_Movement_Dodging);
	ActivationBlockedTags.AddTag(GameProjectSummer26GameplayTags::State_Movement_Traversing);
}

bool UGameProjectSummer26DodgeAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;

	return ASC && Movement && Movement->IsMovingOnGround() &&
		ASC->GetNumericAttribute(UGameProjectSummer26AttributeSet::GetEnduranceAttribute()) >= 50.0f;
}

void UGameProjectSummer26DodgeAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Character || !Movement)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FVector DodgeDirection = Character->GetLastMovementInputVector();
	DodgeDirection.Z = 0.0f;
	if (!DodgeDirection.Normalize())
	{
		// A dodge without movement input is always a backward dodge.
		DodgeDirection = -Character->GetActorForwardVector().GetSafeNormal2D();
	}

	const FVector LaunchVelocity = DodgeDirection * DodgeStrength + FVector::UpVector * DodgeUpwardVelocity;
	ActiveDodgeDirection = DodgeDirection;
	ActiveDodgeSpeed = DodgeStrength;
	ActiveDodgeElapsed = 0.0f;
	Character->LaunchCharacter(LaunchVelocity, true, false);
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			DodgeVelocityTimer,
			this,
			&UGameProjectSummer26DodgeAbility::UpdateDodgeVelocity,
			DodgeUpdateInterval,
			true,
			DodgeUpdateInterval);
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGameProjectSummer26DodgeAbility::UpdateDodgeVelocity()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Character || !Movement)
	{
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActorInfo(),
			GetCurrentActivationInfo(),
			true,
			true);
		return;
	}

	const float UpdateDeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : DodgeUpdateInterval;
	ActiveDodgeElapsed += UpdateDeltaSeconds;
	const float BrakingElapsed = FMath::Max(0.0f, ActiveDodgeElapsed - DodgeCoastDuration);
	ActiveDodgeSpeed = FMath::Max(0.0f, DodgeStrength - DodgeDeceleration * BrakingElapsed);

	const FVector HorizontalVelocity = ActiveDodgeDirection * ActiveDodgeSpeed;
	Movement->Velocity.X = HorizontalVelocity.X;
	Movement->Velocity.Y = HorizontalVelocity.Y;

	if (ActiveDodgeSpeed <= KINDA_SMALL_NUMBER)
	{
		Movement->Velocity.X = 0.0f;
		Movement->Velocity.Y = 0.0f;
		Character->ConsumeMovementInputVector();
		UE_LOG(LogGameProjectSummer26, Display, TEXT("DODGE_SPEED_ADAPT PASS: dodge ended at horizontal speed 0.0."));
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActorInfo(),
			GetCurrentActivationInfo(),
			true,
			false);
	}
}

void UGameProjectSummer26DodgeAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DodgeVelocityTimer);
	}
	ActiveDodgeSpeed = 0.0f;
	ActiveDodgeElapsed = 0.0f;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
