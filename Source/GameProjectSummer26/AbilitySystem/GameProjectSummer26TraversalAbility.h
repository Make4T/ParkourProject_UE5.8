// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/GameProjectSummer26GameplayAbility.h"
#include "TimerManager.h"
#include "GameProjectSummer26TraversalAbility.generated.h"

/**
 * GAS owner for Game Animation Sample traversal activation.
 * The sample component continues to own Chooser, montage and Motion Warping execution.
 */
UCLASS()
class GAMEPROJECTSUMMER26_API UGameProjectSummer26TraversalAbility : public UGameProjectSummer26GameplayAbility
{
	GENERATED_BODY()

public:
	UGameProjectSummer26TraversalAbility();

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal", meta = (ClampMin = "0.01"))
	float CompletionPollInterval = 0.05f;

private:
	void MonitorTraversalCompletion();

	FTimerHandle CompletionPollTimer;
	bool bOwnsHangingTag = false;
};
