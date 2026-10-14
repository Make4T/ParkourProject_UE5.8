// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffect.h"
#include "GameProjectSummer26SprintCostEffect.generated.h"

/** Instant one-point Stamina cost used by the sprint ability at a fixed interval. */
UCLASS()
class GAMEPROJECTSUMMER26_API UGameProjectSummer26SprintCostEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameProjectSummer26SprintCostEffect();
};
