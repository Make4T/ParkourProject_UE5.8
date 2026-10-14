// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimSequenceBase.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "GameProjectSummer26ParkourSettings.generated.h"

/** Designer-facing parkour tuning values shared by detection and movement. */
UCLASS(BlueprintType)
class GAMEPROJECTSUMMER26_API UGameProjectSummer26ParkourSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Existing Game Animation Sample Traversable trace channel. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection")
	TEnumAsByte<ECollisionChannel> TraversalTraceChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Ground", meta = (ClampMin = "1.0"))
	float GroundTraceForwardDistance = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Ground", meta = (ClampMin = "1.0"))
	float GroundTraceRadius = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Ground", meta = (ClampMin = "1.0"))
	float MinimumObstacleHeight = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Ground", meta = (ClampMin = "1.0"))
	float MaximumVaultHeight = 105.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Ground", meta = (ClampMin = "1.0"))
	float MaximumLowMantleHeight = 160.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Ground", meta = (ClampMin = "1.0"))
	float MaximumMantleHeight = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Ground", meta = (ClampMin = "1.0"))
	float TopSurfaceForwardOffset = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Ground", meta = (ClampMin = "1.0"))
	float LandingForwardOffset = 55.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Ground", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimumTopSurfaceNormalZ = 0.65f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Wall", meta = (ClampMin = "1.0"))
	float WallTraceDistance = 85.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Wall", meta = (ClampMin = "1.0"))
	float WallTraceRadius = 24.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Wall", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaximumRunnableWallNormalZ = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Hang", meta = (ClampMin = "1.0"))
	float HangForwardDistance = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Hang", meta = (ClampMin = "1.0"))
	float HangWallTraceRadius = 18.0f;

	/** Height of the wall probe relative to the character capsule centre. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Hang")
	float HangWallProbeHeight = 45.0f;

	/** Allowed ledge height range relative to the character capsule centre. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Hang")
	float MinimumHangLedgeOffsetZ = -20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Hang")
	float MaximumHangLedgeOffsetZ = 160.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Hang", meta = (ClampMin = "1.0"))
	float HangTopSurfaceForwardOffset = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Hang", meta = (ClampMin = "0.0"))
	float HangWallClearance = 8.0f;

	/** Vertical distance from the ledge to the capsule centre while hanging. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Hang", meta = (ClampMin = "1.0"))
	float HangVerticalOffset = 65.0f;

	/** Extra distance below the detected top surface, for gripping an underside rather than its top. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection|Hang", meta = (ClampMin = "0.0"))
	float HangUndersideOffset = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Hang", meta = (ClampMin = "0.0"))
	float HangShimmySpeed = 130.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Hang", meta = (ClampMin = "0.0"))
	float HangShimmyAcceleration = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Hang", meta = (ClampMin = "1.0"))
	float ClimbUpForwardOffset = 55.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Hang", meta = (ClampMin = "0.0"))
	float ClimbUpSurfaceClearance = 2.0f;

	/** Time reserved for the hang-to-ledge animation before the capsule is placed on top. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang", meta = (ClampMin = "0.05"))
	float HangClimbUpDuration = 1.25f;

	/** Retargeted UEFN mannequin assets played by the native hang animation controller. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang")
	TSoftObjectPtr<UAnimSequenceBase> HangIdleAnimation =
		TSoftObjectPtr<UAnimSequenceBase>(FSoftObjectPath(
			TEXT("/Game/Animations/Parkour/Hang/Retargeted_Animations/Hanging_Idle.Hanging_Idle")));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang")
	TSoftObjectPtr<UAnimSequenceBase> HangShimmyLeftAnimation =
		TSoftObjectPtr<UAnimSequenceBase>(FSoftObjectPath(
			TEXT("/Game/Animations/Parkour/Hang/Retargeted_Animations/Braced_Hang_Shimmy_Left.Braced_Hang_Shimmy_Left")));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang")
	TSoftObjectPtr<UAnimSequenceBase> HangShimmyRightAnimation =
		TSoftObjectPtr<UAnimSequenceBase>(FSoftObjectPath(
			TEXT("/Game/Animations/Parkour/Hang/Retargeted_Animations/Braced_Hang_Shimmy_Right.Braced_Hang_Shimmy_Right")));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang")
	TSoftObjectPtr<UAnimSequenceBase> HangClimbUpAnimation =
		TSoftObjectPtr<UAnimSequenceBase>(FSoftObjectPath(
			TEXT("/Game/Animations/Parkour/Hang/Retargeted_Animations/Braced_Hang_Hop_Up.Braced_Hang_Hop_Up")));

	/** Start time is useful when an imported idle has a short setup before its stable loop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang", meta = (ClampMin = "0.0"))
	float HangIdleStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang", meta = (ClampMin = "0.01"))
	float HangAnimationPlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang", meta = (ClampMin = "0.0"))
	float HangAnimationBlendInTime = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang", meta = (ClampMin = "0.0"))
	float HangAnimationBlendOutTime = 0.12f;

	/** Align the midpoint of the selected hand bones to the detected ledge each hang frame. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang|Hand Alignment")
	bool bAlignHangHandsToLedge = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang|Hand Alignment")
	FName HangLeftHandBone = TEXT("hand_l");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang|Hand Alignment")
	FName HangRightHandBone = TEXT("hand_r");

	/** Positive values move the hand target away from the wall, along its normal. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang|Hand Alignment")
	float HangHandWallOffset = 3.0f;

	/** Positive values move the target above the detected top edge. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang|Hand Alignment")
	float HangHandVerticalOffset = 0.0f;

	/** Downward distance from the detected top edge to the underside where hands grip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang|Hand Alignment", meta = (ClampMin = "0.0"))
	float HangHandUndersideOffset = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Hang|Hand Alignment", meta = (ClampMin = "0.0"))
	float HangHandAlignmentInterpSpeed = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Hang")
	bool bApplyHangSpringArmCameraOverride = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Hang", meta = (ClampMin = "0.0"))
	float HangCameraArmLength = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Hang")
	float HangCameraTargetOffsetZ = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Hang")
	// Keep camera obstruction handling enabled while hanging. Disabling this can
	// put a third-person camera inside the ledge the character is holding.
	bool bDisableHangCameraCollision = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Hang", meta = (ClampMin = "0.0"))
	float HangDropOutwardSpeed = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Hang", meta = (ClampMin = "0.0"))
	float HangDropDownwardSpeed = 80.0f;

	/** Space while hanging launches away from the wall with these velocities. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Hang", meta = (ClampMin = "0.0"))
	float HangWallJumpOutwardSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Hang", meta = (ClampMin = "0.0"))
	float HangWallJumpUpwardSpeed = 250.0f;

	/** Required W/S input amount before climb or drop is triggered. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Hang", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float HangVerticalInputThreshold = 0.55f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Slide", meta = (ClampMin = "0.0"))
	float MinimumSlideStartSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Slide", meta = (ClampMin = "0.0"))
	float SlideInitialBoost = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Slide", meta = (ClampMin = "0.0"))
	float MinimumSlideInitialSpeed = 1150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Slide", meta = (ClampMin = "0.0"))
	float SlideDeceleration = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Slide", meta = (ClampMin = "0.0"))
	float SlideEndSpeed = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Slide", meta = (ClampMin = "0.0"))
	float SlideSlopeAcceleration = 900.0f;
};
