// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26AbilitySystemComponent.h"

#include "AbilitySystem/GameProjectSummer26GameplayAbility.h"
#include "GameplayAbilitySpec.h"

void UGameProjectSummer26AbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
		}
	}
}

void UGameProjectSummer26AbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.Remove(AbilitySpec.Handle);
		}
	}
}

void UGameProjectSummer26AbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (bGamePaused)
	{
		ClearAbilityInput();
		return;
	}

	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reserve(InputHeldSpecHandles.Num() + InputPressedSpecHandles.Num());

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			const UGameProjectSummer26GameplayAbility* AbilityCDO = Cast<UGameProjectSummer26GameplayAbility>(AbilitySpec->Ability);
			if (AbilityCDO && AbilityCDO->GetActivationPolicy() == EGameProjectSummer26AbilityActivationPolicy::WhileInputActive && !AbilitySpec->IsActive())
			{
				AbilitiesToActivate.AddUnique(SpecHandle);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			AbilitySpecInputPressed(*AbilitySpec);

			if (!AbilitySpec->IsActive())
			{
				const UGameProjectSummer26GameplayAbility* AbilityCDO = Cast<UGameProjectSummer26GameplayAbility>(AbilitySpec->Ability);
				if (AbilityCDO && AbilityCDO->GetActivationPolicy() == EGameProjectSummer26AbilityActivationPolicy::OnInputTriggered)
				{
					AbilitiesToActivate.AddUnique(SpecHandle);
				}
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(SpecHandle);
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			AbilitySpecInputReleased(*AbilitySpec);
		}
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UGameProjectSummer26AbilitySystemComponent::ClearAbilityInput()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			AbilitySpecInputReleased(*AbilitySpec);
		}
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}
