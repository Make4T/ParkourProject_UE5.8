// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26TraversalAbility.h"

#include "AbilitySystemComponent.h"
#include "GameProjectSummer26.h"
#include "GameProjectSummer26GameplayTags.h"
#include "Parkour/GameProjectSummer26ParkourCharacter.h"

UGameProjectSummer26TraversalAbility::UGameProjectSummer26TraversalAbility()
{
	ActivationPolicy = EGameProjectSummer26AbilityActivationPolicy::OnInputTriggered;

	FGameplayTagContainer AssetTagContainer;
	AssetTagContainer.AddTag(GameProjectSummer26GameplayTags::Ability_Movement_Traversal);
	AssetTagContainer.AddTag(GameProjectSummer26GameplayTags::Ability_Movement_LedgeGrab);
	SetAssetTags(AssetTagContainer);

	ActivationOwnedTags.AddTag(GameProjectSummer26GameplayTags::State_Movement_Traversing);
	ActivationBlockedTags.AddTag(GameProjectSummer26GameplayTags::State_Movement_Traversing);
	ActivationBlockedTags.AddTag(GameProjectSummer26GameplayTags::State_Movement_Dodging);
	ActivationBlockedTags.AddTag(GameProjectSummer26GameplayTags::State_Movement_Sliding);
}

bool UGameProjectSummer26TraversalAbility::CanActivateAbility(
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

	const AGameProjectSummer26ParkourCharacter* Character =
		ActorInfo ? Cast<AGameProjectSummer26ParkourCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	return Character && !Character->IsSampleTraversalActive();
}

void UGameProjectSummer26TraversalAbility::ActivateAbility(
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

	AGameProjectSummer26ParkourCharacter* Character =
		ActorInfo ? Cast<AGameProjectSummer26ParkourCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!Character || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayTagContainer SprintAbilityTags;
	SprintAbilityTags.AddTag(GameProjectSummer26GameplayTags::Ability_Movement_Sprint);
	ASC->CancelAbilities(&SprintAbilityTags, nullptr, this);

	bOwnsHangingTag = false;
	bool bTraversalCheckFailed = true;
	bool bMontageSelectionFailed = true;
	bool bTraversalStarted = Character->TryStartLedgeHang();
	if (bTraversalStarted)
	{
		ASC->AddLooseGameplayTag(GameProjectSummer26GameplayTags::State_Movement_Hanging);
		bOwnsHangingTag = true;
		UE_LOG(LogGameProjectSummer26, Verbose, TEXT("Traversal GAS entered native ledge hang."));
	}
	else
	{
		bTraversalStarted = Character->TryStartSampleTraversal(
			bTraversalCheckFailed,
			bMontageSelectionFailed);
	}

	if (!bTraversalStarted)
	{
		UE_LOG(
			LogGameProjectSummer26,
			Verbose,
			TEXT("Traversal GAS fallback to Jump. CheckFailed=%s MontageFailed=%s"),
			bTraversalCheckFailed ? TEXT("true") : TEXT("false"),
			bMontageSelectionFailed ? TEXT("true") : TEXT("false"));
		Character->Jump();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			CompletionPollTimer,
			this,
			&UGameProjectSummer26TraversalAbility::MonitorTraversalCompletion,
			CompletionPollInterval,
			true,
			CompletionPollInterval);
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGameProjectSummer26TraversalAbility::MonitorTraversalCompletion()
{
	const AGameProjectSummer26ParkourCharacter* Character =
		Cast<AGameProjectSummer26ParkourCharacter>(GetAvatarActorFromActorInfo());
	if (!Character || (!Character->IsSampleTraversalActive() && !Character->IsHanging()))
	{
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActorInfo(),
			GetCurrentActivationInfo(),
			true,
			Character == nullptr);
	}
}

void UGameProjectSummer26TraversalAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CompletionPollTimer);
	}

	if (bOwnsHangingTag)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo
			? ActorInfo->AbilitySystemComponent.Get()
			: nullptr)
		{
			ASC->RemoveLooseGameplayTag(GameProjectSummer26GameplayTags::State_Movement_Hanging);
		}
		bOwnsHangingTag = false;
	}

	if (bWasCancelled)
	{
		if (AGameProjectSummer26ParkourCharacter* Character = ActorInfo
			? Cast<AGameProjectSummer26ParkourCharacter>(ActorInfo->AvatarActor.Get())
			: nullptr)
		{
			Character->DropFromHang();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
