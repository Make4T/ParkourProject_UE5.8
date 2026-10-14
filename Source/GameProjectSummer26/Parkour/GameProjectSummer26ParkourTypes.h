// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameProjectSummer26ParkourTypes.generated.h"

class UPrimitiveComponent;

/** Stable ledge data used by hanging movement. */
USTRUCT(BlueprintType)
struct GAMEPROJECTSUMMER26_API FGameProjectSummer26HangTarget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Parkour|Hang")
	FVector LedgeLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour|Hang")
	FVector HangLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour|Hang")
	FVector WallNormal = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour|Hang")
	FVector LedgeDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour|Hang")
	TObjectPtr<UPrimitiveComponent> TargetComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour|Hang")
	bool bTargetIsMoving = false;

	bool IsValid() const
	{
		return TargetComponent != nullptr &&
			!WallNormal.IsNearlyZero() &&
			!LedgeDirection.IsNearlyZero();
	}

	void Reset()
	{
		*this = FGameProjectSummer26HangTarget();
	}
};

/** High-level action selected by traversal detection and owned by a Gameplay Ability. */
UENUM(BlueprintType)
enum class EGameProjectSummer26ParkourAction : uint8
{
	None,
	Vault,
	MantleLow,
	MantleHigh,
	LedgeGrab,
	Hanging,
	ClimbUp,
	Drop,
	Slide,
	WallRunLeft,
	WallRunRight,
	WallJump,
	WallClimb
};

/** Values stored in UCharacterMovementComponent::CustomMovementMode. */
UENUM(BlueprintType)
enum class EGameProjectSummer26ParkourMovementMode : uint8
{
	None = 0,
	Hanging = 1,
	Sliding = 2,
	WallRunning = 3,
	WallClimbing = 4
};

/** Animation-facing hanging state. This is intentionally separate from CharacterMovement modes. */
UENUM(BlueprintType)
enum class EGameProjectSummer26HangAnimationState : uint8
{
	None,
	Idle,
	ShimmyLeft,
	ShimmyRight,
	ClimbUp
};

/**
 * Local result of a traversal query.
 * Gameplay Abilities may use this as input, but the server must repeat and validate the query.
 */
USTRUCT(BlueprintType)
struct GAMEPROJECTSUMMER26_API FGameProjectSummer26ParkourQueryResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	EGameProjectSummer26ParkourAction Action = EGameProjectSummer26ParkourAction::None;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	FVector ObstacleLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	FVector LedgeLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	FVector LandingLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	FVector WallNormal = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	FVector WallDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	FTransform WarpTarget = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	TObjectPtr<UPrimitiveComponent> TargetComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	float ObstacleHeight = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	float ObstacleDepth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Parkour")
	bool bTargetIsMoving = false;

	bool IsValid() const
	{
		return Action != EGameProjectSummer26ParkourAction::None && TargetComponent != nullptr;
	}

	void Reset()
	{
		*this = FGameProjectSummer26ParkourQueryResult();
	}
};
