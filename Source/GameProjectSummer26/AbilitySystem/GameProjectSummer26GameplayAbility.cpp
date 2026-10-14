// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GameProjectSummer26GameplayAbility.h"

UGameProjectSummer26GameplayAbility::UGameProjectSummer26GameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
