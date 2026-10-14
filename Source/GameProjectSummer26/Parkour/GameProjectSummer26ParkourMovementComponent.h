// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Parkour/GameProjectSummer26ParkourTypes.h"
#include "GameProjectSummer26ParkourMovementComponent.generated.h"

class UGameProjectSummer26ParkourSettings;

/** Character Movement owner for continuous, network-predicted parkour physics. */
UCLASS(ClassGroup = (Parkour))
class GAMEPROJECTSUMMER26_API UGameProjectSummer26ParkourMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UGameProjectSummer26ParkourMovementComponent();

	void SetParkourSettings(UGameProjectSummer26ParkourSettings* InSettings);

	UFUNCTION(BlueprintPure, Category = "Parkour|Movement")
	const UGameProjectSummer26ParkourSettings* GetParkourSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Parkour|Movement")
	void SetWantsToSlide(bool bNewWantsToSlide);

	UFUNCTION(BlueprintPure, Category = "Parkour|Movement")
	bool WantsToSlide() const { return bWantsToSlide; }

	UFUNCTION(BlueprintPure, Category = "Parkour|Movement")
	bool IsSliding() const;

	UFUNCTION(BlueprintPure, Category = "Parkour|Hang")
	bool IsHanging() const;

	/** Snaps to a validated ledge and enters the Hanging custom movement mode. */
	UFUNCTION(BlueprintCallable, Category = "Parkour|Hang")
	bool StartHanging(const FGameProjectSummer26HangTarget& NewHangTarget);

	/** Places the capsule on the top surface when the destination is clear. */
	UFUNCTION(BlueprintCallable, Category = "Parkour|Hang")
	bool TryClimbUp();

	/** Leaves the ledge and applies a small outward/downward velocity. */
	UFUNCTION(BlueprintCallable, Category = "Parkour|Hang")
	void DropFromHang();

	/** Launches away from the wall and leaves Hanging. */
	UFUNCTION(BlueprintCallable, Category = "Parkour|Hang")
	bool JumpAwayFromHang();

	UFUNCTION(BlueprintPure, Category = "Parkour|Hang")
	FGameProjectSummer26HangTarget GetHangTarget() const { return CurrentHangTarget; }

	/** -1..1 shimmy value for the Animation Blueprint. */
	UFUNCTION(BlueprintPure, Category = "Parkour|Hang")
	float GetHangMovementAlpha() const;

	/** Stable state selector for the Animation Blueprint. */
	UFUNCTION(BlueprintPure, Category = "Parkour|Hang|Animation")
	EGameProjectSummer26HangAnimationState GetHangAnimationState() const;

	UFUNCTION(BlueprintPure, Category = "Parkour|Hang|Animation")
	bool IsClimbingUpFromHang() const { return bIsClimbingUpFromHang; }

	/** Normalized 0..1 time for synchronizing a climb-up animation. */
	UFUNCTION(BlueprintPure, Category = "Parkour|Hang|Animation")
	float GetHangClimbUpProgress() const;

	/** Uses the selected climb animation length when the asset is available. */
	float GetHangClimbUpDuration() const;

	UFUNCTION(BlueprintPure, Category = "Parkour|Movement")
	EGameProjectSummer26ParkourMovementMode GetParkourMovementMode() const;

	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour")
	TObjectPtr<UGameProjectSummer26ParkourSettings> ParkourSettings;

private:
	bool CanStartSlide() const;
	void StartSlide();
	void StopSlide();
	void PhysSliding(float DeltaTime, int32 Iterations);
	void PhysHanging(float DeltaTime, int32 Iterations);
	void CompleteClimbUp();

	uint8 bWantsToSlide : 1;

	FGameProjectSummer26HangTarget CurrentHangTarget;
	float CurrentHangSpeed = 0.0f;
	FVector PendingClimbUpLocation = FVector::ZeroVector;
	float HangClimbUpElapsedTime = 0.0f;
	bool bIsClimbingUpFromHang = false;
	bool bHangEndReported = false;
	/** W/S must return to neutral once after grabbing before an exit action is accepted. */
	bool bHangVerticalInputArmed = false;
	bool bHangVerticalInputLatched = false;
};

/** Saved input state replayed during client movement correction. */
class FGameProjectSummer26SavedMove_Parkour : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual void SetMoveFor(
		ACharacter* Character,
		float InDeltaTime,
		const FVector& NewAcceleration,
		FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* Character) override;

private:
	uint8 bSavedWantsToSlide : 1;
};

/** Allocates the project's saved move type. */
class FGameProjectSummer26NetworkPredictionData_Client_Parkour : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	explicit FGameProjectSummer26NetworkPredictionData_Client_Parkour(
		const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};
