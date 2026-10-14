// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Parkour/GameProjectSummer26ParkourTypes.h"
#include "GameProjectSummer26ParkourTraversalComponent.generated.h"

class UGameProjectSummer26ParkourSettings;

/** Performs read-only traversal queries. It never moves the character or plays animation. */
UCLASS(ClassGroup = (Parkour), meta = (BlueprintSpawnableComponent))
class GAMEPROJECTSUMMER26_API UGameProjectSummer26ParkourTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameProjectSummer26ParkourTraversalComponent();

	UFUNCTION(BlueprintCallable, Category = "Parkour|Detection")
	bool QueryGroundTraversal(FGameProjectSummer26ParkourQueryResult& OutResult) const;

	UFUNCTION(BlueprintCallable, Category = "Parkour|Detection")
	bool QueryWallRun(bool bRightSide, FGameProjectSummer26ParkourQueryResult& OutResult) const;

	/** Finds a static ledge in front of the owning character. */
	UFUNCTION(BlueprintCallable, Category = "Parkour|Detection")
	bool QueryLedgeGrab(FGameProjectSummer26HangTarget& OutTarget) const;

	/** Revalidates a ledge from an arbitrary capsule location while shimmying. */
	bool QueryLedgeGrabFromLocation(
		const FVector& ProbeLocation,
		const FVector& WallFacingDirection,
		FGameProjectSummer26HangTarget& OutTarget) const;

	/** Returns a clear capsule location on top of a validated ledge. */
	bool QueryClimbUpLocation(
		const FGameProjectSummer26HangTarget& HangTarget,
		FVector& OutCapsuleLocation) const;

	void SetParkourSettings(UGameProjectSummer26ParkourSettings* InSettings);

	UFUNCTION(BlueprintPure, Category = "Parkour|Detection")
	const UGameProjectSummer26ParkourSettings* GetParkourSettings() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Detection")
	TObjectPtr<UGameProjectSummer26ParkourSettings> ParkourSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Debug")
	bool bDrawDebug = false;

private:
	bool ShouldDrawDebug() const;
	bool QueryLedgeGrabInternal(
		const FVector& ProbeLocation,
		const FVector& WallFacingDirection,
		FGameProjectSummer26HangTarget& OutTarget,
		bool bReportFailure) const;
};
