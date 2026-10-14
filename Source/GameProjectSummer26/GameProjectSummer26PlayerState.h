// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "GameProjectSummer26PlayerState.generated.h"

class UGameProjectSummer26AbilitySystemComponent;
class UGameProjectSummer26AttributeSet;
class UGameProjectSummer26StatusWidget;
class UGameplayAbility;
class UGameplayEffect;
class ACharacter;

/** One ability class and the optional input tag stored on its granted ability spec. */
USTRUCT(BlueprintType)
struct GAMEPROJECTSUMMER26_API FGameProjectSummer26StartupAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (Categories = "Input.Ability"))
	FGameplayTag InputTag;
};

/**
 * Persistent GAS owner for a player. The possessed Character is used as the avatar.
 * Startup effects and abilities are applied only by authority and only once.
 */
UCLASS()
class GAMEPROJECTSUMMER26_API AGameProjectSummer26PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGameProjectSummer26PlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Abilities")
	UGameProjectSummer26AbilitySystemComponent* GetGameAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category = "Abilities")
	const UGameProjectSummer26AttributeSet* GetCoreAttributeSet() const { return AttributeSet; }

	/** Refreshes Owner/Avatar actor info and performs authority-only startup once. */
	void InitializeAbilitySystem(AActor* AvatarActor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UGameProjectSummer26AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UGameProjectSummer26AttributeSet> AttributeSet;

	/** Override in a PlayerState Blueprint when a different initialization effect is desired. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Startup")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

	/** Abilities granted once by the server. This native class intentionally starts with an empty list. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Startup")
	TArray<FGameProjectSummer26StartupAbility> StartupAbilities;

	/** Authority applies this pulse while the player is not sprinting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Regeneration")
	TSubclassOf<UGameplayEffect> StaminaRegenerationEffect;

	/** Authority restores two Endurance every 0.2 seconds, or 10 per second. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities|Regeneration")
	TSubclassOf<UGameplayEffect> EnduranceRegenerationEffect;

	/** Fallback HUD used by pawns that do not inherit the project's native Character. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGameProjectSummer26StatusWidget> FallbackStatusWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UGameProjectSummer26StatusWidget> FallbackStatusWidget;

private:
	void ApplyDefaultAttributes();
	void GrantStartupAbilities();
	void RegenerateResources();
	void UpdateGenericPawnBridge(float DeltaSeconds);
	void SetFallbackSprintInput(bool bPressed);
	void SetFallbackDodgeInput(bool bPressed);
	void SetFallbackControlInput(bool bPressed);
	bool StartFallbackSlide(ACharacter* Character);
	void UpdateFallbackSlide(ACharacter* Character, float DeltaSeconds);
	void EndFallbackSlide(ACharacter* Character);
	void HandleFallbackTurnAround();

#if !UE_BUILD_SHIPPING
	void RunDodgeSmokeTest();
	void FinishDodgeSmokeTest();
	void RunSlideSmokeTest();
	void FinishSlideSmokeTest();
	void RunCrouchSmokeTest();
#endif

	bool bDefaultAttributesApplied = false;
	bool bStartupAbilitiesGranted = false;
	bool bFallbackSprintPressed = false;
	bool bFallbackDodgePressed = false;
	bool bFallbackControlPressed = false;
	bool bFallbackControlInputOverriddenForTest = false;
	bool bFallbackSliding = false;
	bool bFallbackTurnAroundPressed = false;
	FVector FallbackSlideDirection = FVector::ForwardVector;
	float FallbackSlideSpeed = 0.0f;
	float SavedGroundFriction = 0.0f;
	float SavedBrakingDecelerationWalking = 0.0f;
	float SavedBrakingFrictionFactor = 0.0f;
	float SavedMaxAcceleration = 0.0f;
	float SavedMaxWalkSpeed = 0.0f;
	float SavedMaxWalkSpeedCrouched = 0.0f;
	bool bSavedUseSeparateBrakingFriction = false;
	TWeakObjectPtr<APawn> LastInitializedPawn;
	FTimerHandle ResourceRegenerationTimer;

#if !UE_BUILD_SHIPPING
	bool bDodgeSmokeTestScheduled = false;
	bool bSlideSmokeTestScheduled = false;
	float DodgeSmokeEnduranceAfterActivation = 0.0f;
	FVector DodgeSmokeStartLocation = FVector::ZeroVector;
	FVector DodgeSmokeStartForward = FVector::ForwardVector;
	FTimerHandle DodgeSmokeTestTimer;
	FTimerHandle SlideSmokeTestTimer;
#endif
};
