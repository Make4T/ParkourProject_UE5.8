// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26DefaultAttributesEffect.h"

#include "AbilitySystem/GameProjectSummer26AttributeSet.h"

UGameProjectSummer26DefaultAttributesEffect::UGameProjectSummer26DefaultAttributesEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	auto AddOverrideModifier = [this](const FGameplayAttribute& Attribute, float Value)
	{
		FGameplayModifierInfo& Modifier = Modifiers.AddDefaulted_GetRef();
		Modifier.Attribute = Attribute;
		Modifier.ModifierOp = EGameplayModOp::Override;
		Modifier.ModifierMagnitude = FScalableFloat(Value);
	};

	AddOverrideModifier(UGameProjectSummer26AttributeSet::GetMaxHealthAttribute(), 100.0f);
	AddOverrideModifier(UGameProjectSummer26AttributeSet::GetHealthAttribute(), 100.0f);
	AddOverrideModifier(UGameProjectSummer26AttributeSet::GetMaxStaminaAttribute(), 100.0f);
	AddOverrideModifier(UGameProjectSummer26AttributeSet::GetStaminaAttribute(), 100.0f);
	AddOverrideModifier(UGameProjectSummer26AttributeSet::GetMaxManaAttribute(), 100.0f);
	AddOverrideModifier(UGameProjectSummer26AttributeSet::GetManaAttribute(), 100.0f);
	AddOverrideModifier(UGameProjectSummer26AttributeSet::GetMaxEnduranceAttribute(), 100.0f);
	AddOverrideModifier(UGameProjectSummer26AttributeSet::GetEnduranceAttribute(), 100.0f);
}
