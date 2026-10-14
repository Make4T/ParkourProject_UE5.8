// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26StaminaRegenEffect.h"

#include "AbilitySystem/GameProjectSummer26AttributeSet.h"

UGameProjectSummer26StaminaRegenEffect::UGameProjectSummer26StaminaRegenEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo& StaminaRegen = Modifiers.AddDefaulted_GetRef();
	StaminaRegen.Attribute = UGameProjectSummer26AttributeSet::GetStaminaAttribute();
	StaminaRegen.ModifierOp = EGameplayModOp::Additive;
	StaminaRegen.ModifierMagnitude = FScalableFloat(1.0f);
}
