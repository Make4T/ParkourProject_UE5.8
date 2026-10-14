// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "GameProjectSummer26GameplayAbility.generated.h"

UENUM(BlueprintType)
enum class EGameProjectSummer26AbilityActivationPolicy : uint8
{
	OnInputTriggered,
	WhileInputActive
};

/** Base class for gameplay abilities used by this project. */
UCLASS(Abstract, Blueprintable)
class GAMEPROJECTSUMMER26_API UGameProjectSummer26GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameProjectSummer26GameplayAbility();

	EGameProjectSummer26AbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

protected:
	/** Controls whether input activates once on press or remains eligible while held. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Input")
	EGameProjectSummer26AbilityActivationPolicy ActivationPolicy = EGameProjectSummer26AbilityActivationPolicy::OnInputTriggered;
};
