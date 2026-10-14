// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26SprintAbility.h"

#include "AbilitySystem/GameProjectSummer26AttributeSet.h"
#include "AbilitySystem/GameProjectSummer26SprintCostEffect.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameProjectSummer26GameplayTags.h"

UGameProjectSummer26SprintAbility::UGameProjectSummer26SprintAbility()
{
	ActivationPolicy = EGameProjectSummer26AbilityActivationPolicy::OnInputTriggered;
	SprintCostEffectClass = UGameProjectSummer26SprintCostEffect::StaticClass();

	FGameplayTagContainer AssetTagContainer;
	AssetTagContainer.AddTag(GameProjectSummer26GameplayTags::Ability_Movement_Sprint);
	SetAssetTags(AssetTagContainer);
	ActivationOwnedTags.AddTag(GameProjectSummer26GameplayTags::State_Movement_Sprinting);
	ActivationBlockedTags.AddTag(GameProjectSummer26GameplayTags::State_Movement_Traversing);
}

bool UGameProjectSummer26SprintAbility::CanActivateAbility(
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
	return ASC && Character && Character->GetCharacterMovement() &&
		ASC->GetNumericAttribute(UGameProjectSummer26AttributeSet::GetStaminaAttribute()) > 0.0f;
}

void UGameProjectSummer26SprintAbility::ActivateAbility(
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
	UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!MovementComponent || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PreviousWalkSpeed = MovementComponent->MaxWalkSpeed;
	MovementComponent->MaxWalkSpeed = SprintSpeed;

	StaminaChangedDelegateHandle = ASC
		->GetGameplayAttributeValueChangeDelegate(UGameProjectSummer26AttributeSet::GetStaminaAttribute())
		.AddUObject(this, &UGameProjectSummer26SprintAbility::HandleStaminaChanged);

	if (ActorInfo->IsNetAuthority())
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				StaminaDrainTimer,
				this,
				&UGameProjectSummer26SprintAbility::DrainStamina,
				DrainInterval,
				true,
				DrainInterval);
		}
	}
}

void UGameProjectSummer26SprintAbility::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGameProjectSummer26SprintAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaDrainTimer);
	}

	if (ActorInfo)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			if (StaminaChangedDelegateHandle.IsValid())
			{
				ASC->GetGameplayAttributeValueChangeDelegate(UGameProjectSummer26AttributeSet::GetStaminaAttribute())
					.Remove(StaminaChangedDelegateHandle);
				StaminaChangedDelegateHandle.Reset();
			}
		}

		if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
			{
				MovementComponent->MaxWalkSpeed = PreviousWalkSpeed;
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameProjectSummer26SprintAbility::DrainStamina()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !SprintCostEffectClass)
	{
		EndSprint(true);
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SprintCostEffectClass, GetAbilityLevel(), EffectContext);
	if (!SpecHandle.IsValid())
	{
		EndSprint(true);
		return;
	}

	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	if (ASC->GetNumericAttribute(UGameProjectSummer26AttributeSet::GetStaminaAttribute()) <= 0.0f)
	{
		EndSprint(false);
	}
}

void UGameProjectSummer26SprintAbility::HandleStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
	if (ChangeData.NewValue <= 0.0f)
	{
		EndSprint(false);
	}
}

void UGameProjectSummer26SprintAbility::EndSprint(bool bWasCancelled)
{
	if (IsActive())
	{
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActorInfo(),
			GetCurrentActivationInfo(),
			true,
			bWasCancelled);
	}
}
