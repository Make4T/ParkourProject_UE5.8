// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/GameProjectSummer26GameplayAbility.h"
#include "TimerManager.h"
#include "GameProjectSummer26SprintAbility.generated.h"

struct FOnAttributeChangeData;

/** Hold-to-sprint ability. Authority drains one Stamina point every DrainInterval. */
UCLASS()
class GAMEPROJECTSUMMER26_API UGameProjectSummer26SprintAbility : public UGameProjectSummer26GameplayAbility
{
	GENERATED_BODY()

public:
	UGameProjectSummer26SprintAbility();

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

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	float SprintSpeed = 900.0f;

	/** One Stamina is consumed each interval; 0.1 means ten Stamina per second. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint", meta = (ClampMin = "0.05"))
	float DrainInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	TSubclassOf<UGameplayEffect> SprintCostEffectClass;

private:
	void DrainStamina();
	void HandleStaminaChanged(const FOnAttributeChangeData& ChangeData);
	void EndSprint(bool bWasCancelled);

	FTimerHandle StaminaDrainTimer;
	FDelegateHandle StaminaChangedDelegateHandle;
	float PreviousWalkSpeed = 0.0f;
};
