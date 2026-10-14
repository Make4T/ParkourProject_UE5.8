// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26DodgeCostEffect.h"

#include "AbilitySystem/GameProjectSummer26AttributeSet.h"

UGameProjectSummer26DodgeCostEffect::UGameProjectSummer26DodgeCostEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo& Cost = Modifiers.AddDefaulted_GetRef();
	Cost.Attribute = UGameProjectSummer26AttributeSet::GetEnduranceAttribute();
	Cost.ModifierOp = EGameplayModOp::Additive;
	Cost.ModifierMagnitude = FScalableFloat(-50.0f);
}
