// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26SprintCostEffect.h"

#include "AbilitySystem/GameProjectSummer26AttributeSet.h"

UGameProjectSummer26SprintCostEffect::UGameProjectSummer26SprintCostEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo& StaminaCost = Modifiers.AddDefaulted_GetRef();
	StaminaCost.Attribute = UGameProjectSummer26AttributeSet::GetStaminaAttribute();
	StaminaCost.ModifierOp = EGameplayModOp::Additive;
	StaminaCost.ModifierMagnitude = FScalableFloat(-1.0f);
}
