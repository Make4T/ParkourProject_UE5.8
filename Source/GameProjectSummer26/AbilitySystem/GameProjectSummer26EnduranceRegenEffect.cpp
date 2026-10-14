// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26EnduranceRegenEffect.h"

#include "AbilitySystem/GameProjectSummer26AttributeSet.h"

UGameProjectSummer26EnduranceRegenEffect::UGameProjectSummer26EnduranceRegenEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo& Regeneration = Modifiers.AddDefaulted_GetRef();
	Regeneration.Attribute = UGameProjectSummer26AttributeSet::GetEnduranceAttribute();
	Regeneration.ModifierOp = EGameplayModOp::Additive;
	Regeneration.ModifierMagnitude = FScalableFloat(2.0f);
}
