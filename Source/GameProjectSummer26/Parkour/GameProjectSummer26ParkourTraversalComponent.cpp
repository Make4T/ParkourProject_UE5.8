// Copyright Epic Games, Inc. All Rights Reserved.

#include "Parkour/GameProjectSummer26ParkourTraversalComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameProjectSummer26.h"
#include "GameFramework/Character.h"
#include "HAL/IConsoleManager.h"
#include "Parkour/GameProjectSummer26ParkourSettings.h"

namespace
{
	TAutoConsoleVariable<int32> CVarParkourDrawTraces(
		TEXT("parkour.DrawTraces"),
		0,
		TEXT("Draw GameProjectSummer26 parkour queries. 0=off, 1=results, 2=all traces."),
		ECVF_Cheat);

	TAutoConsoleVariable<int32> CVarParkourDebugHang(
		TEXT("parkour.DebugHang"),
		0,
		TEXT("Ledge hang diagnostics. 0=off, 1=state changes, 2=query success/failure reasons."),
		ECVF_Cheat);

	constexpr float DebugDuration = 1.5f;

	void DisplayHangDebug(
		const UObject* Context,
		const FString& Message,
		const FColor Color,
		const int32 RequiredLevel)
	{
		if (CVarParkourDebugHang.GetValueOnGameThread() < RequiredLevel)
		{
			return;
		}

		UE_LOG(
			LogGameProjectSummer26,
			Display,
			TEXT("HANG_DEBUG [%s] %s"),
			*GetNameSafe(Context),
			*Message);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 4.0f, Color, FString::Printf(TEXT("HANG: %s"), *Message));
		}
	}
}

UGameProjectSummer26ParkourTraversalComponent::UGameProjectSummer26ParkourTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGameProjectSummer26ParkourTraversalComponent::SetParkourSettings(
	UGameProjectSummer26ParkourSettings* InSettings)
{
	ParkourSettings = InSettings;
}

const UGameProjectSummer26ParkourSettings*
UGameProjectSummer26ParkourTraversalComponent::GetParkourSettings() const
{
	return ParkourSettings ? ParkourSettings.Get() : GetDefault<UGameProjectSummer26ParkourSettings>();
}

bool UGameProjectSummer26ParkourTraversalComponent::ShouldDrawDebug() const
{
	return bDrawDebug || CVarParkourDrawTraces.GetValueOnGameThread() > 0;
}

bool UGameProjectSummer26ParkourTraversalComponent::QueryGroundTraversal(
	FGameProjectSummer26ParkourQueryResult& OutResult) const
{
	OutResult.Reset();

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	const UCapsuleComponent* Capsule = Character ? Character->GetCapsuleComponent() : nullptr;
	UWorld* World = GetWorld();
	if (!Character || !Settings || !Capsule || !World)
	{
		return false;
	}

	const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
	const FVector ActorLocation = Character->GetActorLocation();
	const float CharacterHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float CharacterRadius = Capsule->GetScaledCapsuleRadius();
	const float CapsuleBottomZ = ActorLocation.Z - CharacterHalfHeight;
	const float TraceHeightRange = Settings->MaximumMantleHeight - Settings->MinimumObstacleHeight;
	const float TraceHalfHeight = FMath::Max(Settings->GroundTraceRadius, TraceHeightRange * 0.5f);
	const float TraceCenterZ = CapsuleBottomZ + Settings->MinimumObstacleHeight + TraceHalfHeight;
	const FVector FrontTraceStart(ActorLocation.X, ActorLocation.Y, TraceCenterZ);
	const FVector FrontTraceEnd = FrontTraceStart + Forward * Settings->GroundTraceForwardDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ParkourGroundTraversal), false, Character);
	FHitResult FrontHit;
	const bool bHitFront = World->SweepSingleByChannel(
		FrontHit,
		FrontTraceStart,
		FrontTraceEnd,
		FQuat::Identity,
		Settings->TraversalTraceChannel,
		FCollisionShape::MakeCapsule(Settings->GroundTraceRadius, TraceHalfHeight),
		QueryParams);

	const bool bDebug = ShouldDrawDebug();
	if (bDebug)
	{
		DrawDebugCapsule(
			World,
			bHitFront ? FrontHit.Location : FrontTraceEnd,
			TraceHalfHeight,
			Settings->GroundTraceRadius,
			FQuat::Identity,
			bHitFront ? FColor::Yellow : FColor::Red,
			false,
			DebugDuration);
		DrawDebugLine(World, FrontTraceStart, FrontTraceEnd, bHitFront ? FColor::Yellow : FColor::Red, false, DebugDuration);
	}

	if (!bHitFront || !FrontHit.GetComponent())
	{
		return false;
	}

	const FVector TopTraceXY = FrontHit.ImpactPoint + Forward * Settings->TopSurfaceForwardOffset;
	const FVector TopTraceStart(TopTraceXY.X, TopTraceXY.Y, CapsuleBottomZ + Settings->MaximumMantleHeight + CharacterRadius);
	const FVector TopTraceEnd(TopTraceXY.X, TopTraceXY.Y, CapsuleBottomZ + Settings->MinimumObstacleHeight);
	FHitResult TopHit;
	const bool bHitTop = World->LineTraceSingleByChannel(
		TopHit,
		TopTraceStart,
		TopTraceEnd,
		Settings->TraversalTraceChannel,
		QueryParams);

	if (bDebug)
	{
		DrawDebugLine(World, TopTraceStart, TopTraceEnd, bHitTop ? FColor::Green : FColor::Red, false, DebugDuration);
	}

	if (!bHitTop || TopHit.ImpactNormal.Z < Settings->MinimumTopSurfaceNormalZ)
	{
		return false;
	}

	const float ObstacleHeight = TopHit.ImpactPoint.Z - CapsuleBottomZ;
	if (ObstacleHeight < Settings->MinimumObstacleHeight || ObstacleHeight > Settings->MaximumMantleHeight)
	{
		return false;
	}

	const FVector BackTraceXY = TopHit.ImpactPoint + Forward * Settings->LandingForwardOffset;
	const FVector BackTraceStart(BackTraceXY.X, BackTraceXY.Y, TopTraceStart.Z);
	const FVector BackTraceEnd(BackTraceXY.X, BackTraceXY.Y, TopTraceEnd.Z);
	FHitResult BackFloorHit;
	const bool bHitBackFloor = World->LineTraceSingleByChannel(
		BackFloorHit,
		BackTraceStart,
		BackTraceEnd,
		Settings->TraversalTraceChannel,
		QueryParams) &&
		BackFloorHit.ImpactNormal.Z >= Settings->MinimumTopSurfaceNormalZ;

	const FVector LandingFloorLocation = bHitBackFloor ? BackFloorHit.ImpactPoint : TopHit.ImpactPoint;
	const FVector LandingCapsuleLocation = LandingFloorLocation + FVector::UpVector * (CharacterHalfHeight + 2.0f);
	const bool bLandingBlocked = World->OverlapBlockingTestByChannel(
		LandingCapsuleLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeCapsule(CharacterRadius, CharacterHalfHeight),
		QueryParams);

	if (bDebug)
	{
		DrawDebugLine(World, BackTraceStart, BackTraceEnd, bHitBackFloor ? FColor::Cyan : FColor::Orange, false, DebugDuration);
		DrawDebugCapsule(
			World,
			LandingCapsuleLocation,
			CharacterHalfHeight,
			CharacterRadius,
			FQuat::Identity,
			bLandingBlocked ? FColor::Red : FColor::Green,
			false,
			DebugDuration);
	}

	if (bLandingBlocked)
	{
		return false;
	}

	EGameProjectSummer26ParkourAction Action = EGameProjectSummer26ParkourAction::MantleHigh;
	if (ObstacleHeight <= Settings->MaximumVaultHeight && bHitBackFloor)
	{
		Action = EGameProjectSummer26ParkourAction::Vault;
	}
	else if (ObstacleHeight <= Settings->MaximumLowMantleHeight)
	{
		Action = EGameProjectSummer26ParkourAction::MantleLow;
	}

	const FVector FacingDirection = (-FrontHit.ImpactNormal).GetSafeNormal2D();
	OutResult.Action = Action;
	OutResult.ObstacleLocation = FrontHit.ImpactPoint;
	OutResult.LedgeLocation = TopHit.ImpactPoint;
	OutResult.LandingLocation = LandingCapsuleLocation;
	OutResult.WallNormal = FrontHit.ImpactNormal.GetSafeNormal();
	OutResult.WallDirection = FVector::ZeroVector;
	OutResult.WarpTarget = FTransform(FacingDirection.Rotation(), TopHit.ImpactPoint);
	OutResult.TargetComponent = FrontHit.GetComponent();
	OutResult.ObstacleHeight = ObstacleHeight;
	OutResult.ObstacleDepth = bHitBackFloor
		? FMath::Max(0.0f, FVector::DotProduct(BackFloorHit.ImpactPoint - TopHit.ImpactPoint, Forward))
		: 0.0f;
	OutResult.bTargetIsMoving = FrontHit.GetComponent()->Mobility == EComponentMobility::Movable;
	return OutResult.IsValid();
}

bool UGameProjectSummer26ParkourTraversalComponent::QueryWallRun(
	bool bRightSide,
	FGameProjectSummer26ParkourQueryResult& OutResult) const
{
	OutResult.Reset();

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	UWorld* World = GetWorld();
	if (!Character || !Settings || !World)
	{
		return false;
	}

	const FVector SideDirection = (bRightSide ? Character->GetActorRightVector() : -Character->GetActorRightVector()).GetSafeNormal2D();
	const FVector TraceStart = Character->GetActorLocation();
	const FVector TraceEnd = TraceStart + SideDirection * Settings->WallTraceDistance;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ParkourWallRun), false, Character);
	FHitResult WallHit;
	const bool bHitWall = World->SweepSingleByChannel(
		WallHit,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		Settings->TraversalTraceChannel,
		FCollisionShape::MakeSphere(Settings->WallTraceRadius),
		QueryParams);

	if (ShouldDrawDebug())
	{
		DrawDebugLine(World, TraceStart, TraceEnd, bHitWall ? FColor::Green : FColor::Red, false, DebugDuration);
		DrawDebugSphere(World, bHitWall ? WallHit.ImpactPoint : TraceEnd, Settings->WallTraceRadius, 12, bHitWall ? FColor::Green : FColor::Red, false, DebugDuration);
	}

	if (!bHitWall || !WallHit.GetComponent() || FMath::Abs(WallHit.ImpactNormal.Z) > Settings->MaximumRunnableWallNormalZ)
	{
		return false;
	}

	FVector WallDirection = FVector::CrossProduct(FVector::UpVector, WallHit.ImpactNormal).GetSafeNormal2D();
	if (FVector::DotProduct(WallDirection, Character->GetActorForwardVector()) < 0.0f)
	{
		WallDirection *= -1.0f;
	}

	OutResult.Action = bRightSide
		? EGameProjectSummer26ParkourAction::WallRunRight
		: EGameProjectSummer26ParkourAction::WallRunLeft;
	OutResult.ObstacleLocation = WallHit.ImpactPoint;
	OutResult.WallNormal = WallHit.ImpactNormal.GetSafeNormal();
	OutResult.WallDirection = WallDirection;
	OutResult.WarpTarget = FTransform(WallDirection.Rotation(), WallHit.ImpactPoint);
	OutResult.TargetComponent = WallHit.GetComponent();
	OutResult.bTargetIsMoving = WallHit.GetComponent()->Mobility == EComponentMobility::Movable;
	return OutResult.IsValid();
}

bool UGameProjectSummer26ParkourTraversalComponent::QueryLedgeGrab(
	FGameProjectSummer26HangTarget& OutTarget) const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		OutTarget.Reset();
		return false;
	}

	return QueryLedgeGrabInternal(
		Character->GetActorLocation(),
		Character->GetActorForwardVector(),
		OutTarget,
		true);
}

bool UGameProjectSummer26ParkourTraversalComponent::QueryLedgeGrabFromLocation(
	const FVector& ProbeLocation,
	const FVector& WallFacingDirection,
	FGameProjectSummer26HangTarget& OutTarget) const
{
	return QueryLedgeGrabInternal(
		ProbeLocation,
		WallFacingDirection,
		OutTarget,
		false);
}

bool UGameProjectSummer26ParkourTraversalComponent::QueryLedgeGrabInternal(
	const FVector& ProbeLocation,
	const FVector& WallFacingDirection,
	FGameProjectSummer26HangTarget& OutTarget,
	const bool bReportFailure) const
{
	OutTarget.Reset();
	const auto FailQuery = [this, bReportFailure](const FString& Reason)
	{
		if (bReportFailure)
		{
			DisplayHangDebug(this, FString::Printf(TEXT("QUERY FAIL - %s"), *Reason), FColor::Red, 2);
		}
		return false;
	};

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	const UCapsuleComponent* Capsule = Character ? Character->GetCapsuleComponent() : nullptr;
	UWorld* World = GetWorld();
	const FVector Forward = WallFacingDirection.GetSafeNormal2D();
	if (!Character || !Settings || !Capsule || !World || Forward.IsNearlyZero())
	{
		return FailQuery(TEXT("missing Character, settings, capsule, world, or facing direction."));
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ParkourLedgeGrab), false, Character);
	const FVector WallTraceStart = ProbeLocation + FVector::UpVector * Settings->HangWallProbeHeight;
	const FVector WallTraceEnd = WallTraceStart + Forward * Settings->HangForwardDistance;
	FHitResult WallHit;
	const bool bHitWall = World->SweepSingleByChannel(
		WallHit,
		WallTraceStart,
		WallTraceEnd,
		FQuat::Identity,
		Settings->TraversalTraceChannel,
		FCollisionShape::MakeSphere(Settings->HangWallTraceRadius),
		QueryParams);

	const bool bDebug = ShouldDrawDebug();
	if (bDebug)
	{
		DrawDebugLine(World, WallTraceStart, WallTraceEnd, bHitWall ? FColor::Yellow : FColor::Red, false, DebugDuration);
		DrawDebugSphere(
			World,
			bHitWall ? WallHit.ImpactPoint : WallTraceEnd,
			Settings->HangWallTraceRadius,
			12,
			bHitWall ? FColor::Yellow : FColor::Red,
			false,
			DebugDuration);
	}

	if (!bHitWall || !WallHit.GetComponent())
	{
		return FailQuery(FString::Printf(
			TEXT("front trace found no wall on Traversable channel %d within %.0f cm."),
			static_cast<int32>(Settings->TraversalTraceChannel),
			Settings->HangForwardDistance));
	}

	if (FMath::Abs(WallHit.ImpactNormal.Z) > Settings->MaximumRunnableWallNormalZ)
	{
		return FailQuery(FString::Printf(
			TEXT("wall is not vertical enough (NormalZ %.2f, allowed %.2f)."),
			WallHit.ImpactNormal.Z,
			Settings->MaximumRunnableWallNormalZ));
	}

	if (WallHit.GetComponent()->Mobility == EComponentMobility::Movable)
	{
		return FailQuery(TEXT("target is Movable; the first hanging version accepts Static ledges only."));
	}

	const FVector WallNormal = WallHit.ImpactNormal.GetSafeNormal2D();
	const FVector TopTraceXY = WallHit.ImpactPoint - WallNormal * Settings->HangTopSurfaceForwardOffset;
	const FVector TopTraceStart(
		TopTraceXY.X,
		TopTraceXY.Y,
		ProbeLocation.Z + Settings->MaximumHangLedgeOffsetZ + Settings->HangWallTraceRadius);
	const FVector TopTraceEnd(
		TopTraceXY.X,
		TopTraceXY.Y,
		ProbeLocation.Z + Settings->MinimumHangLedgeOffsetZ);
	FHitResult TopHit;
	const bool bHitTop = World->LineTraceSingleByChannel(
		TopHit,
		TopTraceStart,
		TopTraceEnd,
		Settings->TraversalTraceChannel,
		QueryParams);

	if (bDebug)
	{
		DrawDebugLine(World, TopTraceStart, TopTraceEnd, bHitTop ? FColor::Green : FColor::Red, false, DebugDuration);
	}

	if (!bHitTop)
	{
		return FailQuery(TEXT("downward trace did not find a top surface behind the wall face."));
	}

	if (TopHit.ImpactNormal.Z < Settings->MinimumTopSurfaceNormalZ)
	{
		return FailQuery(FString::Printf(
			TEXT("top surface is too steep (NormalZ %.2f, minimum %.2f)."),
			TopHit.ImpactNormal.Z,
			Settings->MinimumTopSurfaceNormalZ));
	}

	const float LedgeOffsetZ = TopHit.ImpactPoint.Z - ProbeLocation.Z;
	if (LedgeOffsetZ < Settings->MinimumHangLedgeOffsetZ ||
		LedgeOffsetZ > Settings->MaximumHangLedgeOffsetZ)
	{
		return FailQuery(FString::Printf(
			TEXT("ledge offset Z %.1f is outside %.1f..%.1f."),
			LedgeOffsetZ,
			Settings->MinimumHangLedgeOffsetZ,
			Settings->MaximumHangLedgeOffsetZ));
	}

	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const FVector LedgeLocation(
		WallHit.ImpactPoint.X,
		WallHit.ImpactPoint.Y,
		TopHit.ImpactPoint.Z);
	FVector HangLocation = LedgeLocation +
		WallNormal * (CapsuleRadius + Settings->HangWallClearance);
	HangLocation.Z = LedgeLocation.Z -
		(Settings->HangVerticalOffset + Settings->HangUndersideOffset);

	const bool bHangLocationBlocked = World->OverlapBlockingTestByChannel(
		HangLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
		QueryParams);
	if (bDebug)
	{
		DrawDebugCapsule(
			World,
			HangLocation,
			CapsuleHalfHeight,
			CapsuleRadius,
			FQuat::Identity,
			bHangLocationBlocked ? FColor::Red : FColor::Cyan,
			false,
			DebugDuration);
	}

	if (bHangLocationBlocked)
	{
		return FailQuery(FString::Printf(
			TEXT("hanging capsule location is blocked at %s; increase HangWallClearance or check Pawn collision."),
			*HangLocation.ToCompactString()));
	}

	OutTarget.LedgeLocation = LedgeLocation;
	OutTarget.HangLocation = HangLocation;
	OutTarget.WallNormal = WallNormal;
	OutTarget.LedgeDirection = FVector::CrossProduct(FVector::UpVector, WallNormal).GetSafeNormal2D();
	OutTarget.TargetComponent = WallHit.GetComponent();
	OutTarget.bTargetIsMoving = false;
	const bool bValid = OutTarget.IsValid();
	if (bValid && bReportFailure)
	{
		DisplayHangDebug(
			this,
			FString::Printf(
				TEXT("QUERY PASS - ledge %s, offset Z %.1f, target %s."),
				*LedgeLocation.ToCompactString(),
				LedgeOffsetZ,
				*GetNameSafe(OutTarget.TargetComponent)),
			FColor::Green,
			2);
	}
	return bValid;
}

bool UGameProjectSummer26ParkourTraversalComponent::QueryClimbUpLocation(
	const FGameProjectSummer26HangTarget& HangTarget,
	FVector& OutCapsuleLocation) const
{
	OutCapsuleLocation = FVector::ZeroVector;
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	const UCapsuleComponent* Capsule = Character ? Character->GetCapsuleComponent() : nullptr;
	UWorld* World = GetWorld();
	if (!Character || !Settings || !Capsule || !World || !HangTarget.IsValid())
	{
		return false;
	}

	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	OutCapsuleLocation = HangTarget.LedgeLocation -
		HangTarget.WallNormal * Settings->ClimbUpForwardOffset +
		FVector::UpVector * (CapsuleHalfHeight + Settings->ClimbUpSurfaceClearance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ParkourClimbUpClearance), false, Character);
	const bool bBlocked = World->OverlapBlockingTestByChannel(
		OutCapsuleLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
		QueryParams);

	if (ShouldDrawDebug())
	{
		DrawDebugCapsule(
			World,
			OutCapsuleLocation,
			CapsuleHalfHeight,
			CapsuleRadius,
			FQuat::Identity,
			bBlocked ? FColor::Red : FColor::Green,
			false,
			DebugDuration);
	}

	return !bBlocked;
}
