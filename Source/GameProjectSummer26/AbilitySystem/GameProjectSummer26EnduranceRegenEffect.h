// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffect.h"
#include "GameProjectSummer26EnduranceRegenEffect.generated.h"

/** Adds two Endurance every 0.2 seconds: 10 points per second. */
UCLASS()
class GAMEPROJECTSUMMER26_API UGameProjectSummer26EnduranceRegenEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameProjectSummer26EnduranceRegenEffect();
};
