// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/GameProjectSummer26GameplayAbility.h"
#include "TimerManager.h"
#include "GameProjectSummer26DodgeAbility.generated.h"

/** Directional ground dodge that consumes one 50-point Endurance segment. */
UCLASS()
class GAMEPROJECTSUMMER26_API UGameProjectSummer26DodgeAbility : public UGameProjectSummer26GameplayAbility
{
	GENERATED_BODY()

public:
	UGameProjectSummer26DodgeAbility();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float DodgeStrength = 1050.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float DodgeUpwardVelocity = 80.0f;

	/** A very short full-speed impulse before rapid braking begins. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float DodgeCoastDuration = 0.16f;

	/** Horizontal speed removed per second after the initial impulse. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float DodgeDeceleration = 2400.0f;

private:
	void UpdateDodgeVelocity();

	FTimerHandle DodgeVelocityTimer;
	FVector ActiveDodgeDirection = FVector::ForwardVector;
	float ActiveDodgeSpeed = 0.0f;
	float ActiveDodgeElapsed = 0.0f;
	static constexpr float DodgeUpdateInterval = 0.025f;
};
