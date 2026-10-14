// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Parkour/GameProjectSummer26ParkourTypes.h"
#include "GameProjectSummer26ParkourCharacter.generated.h"

class UGameProjectSummer26AbilitySystemComponent;
class UActorComponent;
class UAnimMontage;
class UGameProjectSummer26ParkourMovementComponent;
class UGameProjectSummer26ParkourSettings;
class UGameProjectSummer26ParkourTraversalComponent;
class USpringArmComponent;

/**
 * Minimal native base for the Game Animation Sample CMC character.
 * Camera, mesh, AnimBP and sample presentation components remain owned by the derived Blueprint.
 */
UCLASS(Blueprintable)
class GAMEPROJECTSUMMER26_API AGameProjectSummer26ParkourCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	explicit AGameProjectSummer26ParkourCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Parkour")
	UGameProjectSummer26ParkourMovementComponent* GetParkourMovementComponent() const;

	UFUNCTION(BlueprintPure, Category = "Parkour")
	UGameProjectSummer26ParkourTraversalComponent* GetParkourTraversalComponent() const
	{
		return ParkourTraversalComponent;
	}

	/** Transitional GAS accessor while sample input is still routed by PlayerState. */
	UFUNCTION(BlueprintPure, Category = "Abilities")
	UGameProjectSummer26AbilitySystemComponent* GetGameAbilitySystemComponent() const;

	/** Sends one traversal input pulse through GAS. The ability performs a normal Jump on traversal failure. */
	UFUNCTION(BlueprintCallable, Category = "Parkour|Input")
	void HandleTraversalInputPressed();

	/** True when the copied Game Animation Sample component and its required Blueprint functions are available. */
	UFUNCTION(BlueprintPure, Category = "Parkour|Traversal")
	bool HasSampleTraversalSupport() const;

	/** Starts the sample's Chooser/montage/Motion Warping path through its Blueprint API. */
	bool TryStartSampleTraversal(bool& bOutTraversalCheckFailed, bool& bOutMontageSelectionFailed);

	/** Reads the sample component's DoingTraversalAction state. */
	UFUNCTION(BlueprintPure, Category = "Parkour|Traversal")
	bool IsSampleTraversalActive() const;

	/** Attempts the native ledge grab. Valid only while falling. */
	UFUNCTION(BlueprintCallable, Category = "Parkour|Hang")
	bool TryStartLedgeHang();

	UFUNCTION(BlueprintPure, Category = "Parkour|Hang")
	bool IsHanging() const;

	UFUNCTION(BlueprintCallable, Category = "Parkour|Hang")
	bool TryClimbUpFromHang();

	UFUNCTION(BlueprintCallable, Category = "Parkour|Hang")
	bool JumpAwayFromHang();

	UFUNCTION(BlueprintCallable, Category = "Parkour|Hang")
	void DropFromHang();

	/** Native animation bridge for the retargeted hang sequences. Called by movement state changes. */
	void RefreshHangAnimation();

	/** Aligns the visible hand midpoint to the native ledge target without moving the capsule. */
	void UpdateHangHandAlignment();

protected:
	virtual void PostInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour")
	TObjectPtr<UGameProjectSummer26ParkourTraversalComponent> ParkourTraversalComponent;

	/** Optional asset override. Class defaults are used when this is empty. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parkour")
	TObjectPtr<UGameProjectSummer26ParkourSettings> ParkourSettings;

private:
	void ApplyParkourSettings();
	void InitializeAbilitySystem();
	UActorComponent* FindSampleTraversalComponent() const;
	void StopHangAnimation(float BlendOutTime = 0.12f, bool bRestoreMeshAlignment = true);
	void ResetHangMeshAlignment();
	void ApplyHangCameraOverride();
	void RestoreHangCameraOverride();

	TObjectPtr<UAnimMontage> ActiveHangMontage;
	EGameProjectSummer26HangAnimationState ActiveHangAnimationState =
		EGameProjectSummer26HangAnimationState::None;
	uint8 PreviousRootMotionMode = 0;
	bool bHasHangRootMotionOverride = false;
	FVector SavedHangMeshRelativeLocation = FVector::ZeroVector;
	bool bHasSavedHangMeshRelativeLocation = false;

	struct FHangSpringArmState
	{
		TWeakObjectPtr<USpringArmComponent> Component;
		float TargetArmLength = 0.0f;
		FVector TargetOffset = FVector::ZeroVector;
		bool bDoCollisionTest = true;
	};
	TArray<FHangSpringArmState> SavedHangSpringArms;
};
