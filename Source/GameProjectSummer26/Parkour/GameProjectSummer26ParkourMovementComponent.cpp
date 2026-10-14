// Copyright Epic Games, Inc. All Rights Reserved.

#include "Parkour/GameProjectSummer26ParkourMovementComponent.h"

#include "Engine/Engine.h"
#include "GameProjectSummer26.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "InputCoreTypes.h"
#include "Parkour/GameProjectSummer26ParkourCharacter.h"
#include "Parkour/GameProjectSummer26ParkourSettings.h"
#include "Parkour/GameProjectSummer26ParkourTraversalComponent.h"

namespace
{
	int32 GetHangDebugLevel()
	{
		const IConsoleVariable* DebugVariable =
			IConsoleManager::Get().FindConsoleVariable(TEXT("parkour.DebugHang"));
		return DebugVariable ? DebugVariable->GetInt() : 0;
	}

	void DisplayHangState(
		const UObject* Context,
		const FString& Message,
		const FColor Color,
		const int32 RequiredLevel = 1)
	{
		if (GetHangDebugLevel() < RequiredLevel)
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

UGameProjectSummer26ParkourMovementComponent::UGameProjectSummer26ParkourMovementComponent()
	: bWantsToSlide(false)
{
}

void UGameProjectSummer26ParkourMovementComponent::SetParkourSettings(
	UGameProjectSummer26ParkourSettings* InSettings)
{
	ParkourSettings = InSettings;
}

const UGameProjectSummer26ParkourSettings*
UGameProjectSummer26ParkourMovementComponent::GetParkourSettings() const
{
	return ParkourSettings ? ParkourSettings.Get() : GetDefault<UGameProjectSummer26ParkourSettings>();
}

void UGameProjectSummer26ParkourMovementComponent::SetWantsToSlide(bool bNewWantsToSlide)
{
	bWantsToSlide = bNewWantsToSlide;
}

bool UGameProjectSummer26ParkourMovementComponent::IsSliding() const
{
	return MovementMode == MOVE_Custom &&
		CustomMovementMode == static_cast<uint8>(EGameProjectSummer26ParkourMovementMode::Sliding);
}

bool UGameProjectSummer26ParkourMovementComponent::IsHanging() const
{
	return MovementMode == MOVE_Custom &&
		CustomMovementMode == static_cast<uint8>(EGameProjectSummer26ParkourMovementMode::Hanging);
}

bool UGameProjectSummer26ParkourMovementComponent::StartHanging(
	const FGameProjectSummer26HangTarget& NewHangTarget)
{
	if (!NewHangTarget.IsValid() || NewHangTarget.bTargetIsMoving ||
		!CharacterOwner || !UpdatedComponent)
	{
		DisplayHangState(this, TEXT("START FAIL - invalid/moving target or missing movement owner."), FColor::Red, 2);
		return false;
	}

	CurrentHangTarget = NewHangTarget;
	CurrentHangSpeed = 0.0f;
	PendingClimbUpLocation = FVector::ZeroVector;
	HangClimbUpElapsedTime = 0.0f;
	bIsClimbingUpFromHang = false;
	bHangEndReported = false;
	bHangVerticalInputArmed = false;
	bHangVerticalInputLatched = false;
	StopMovementImmediately();

	const FQuat FacingRotation = (-CurrentHangTarget.WallNormal).Rotation().Quaternion();
	UpdatedComponent->SetWorldLocationAndRotation(
		CurrentHangTarget.HangLocation,
		FacingRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	SetMovementMode(
		MOVE_Custom,
		static_cast<uint8>(EGameProjectSummer26ParkourMovementMode::Hanging));
	if (AGameProjectSummer26ParkourCharacter* ParkourCharacter =
		Cast<AGameProjectSummer26ParkourCharacter>(CharacterOwner))
	{
		ParkourCharacter->RefreshHangAnimation();
	}
	DisplayHangState(
		this,
		FString::Printf(
			TEXT("START PASS - Hanging at %s. Release W/S before the next climb or drop command."),
			*CurrentHangTarget.HangLocation.ToCompactString()),
		FColor::Green);
	return IsHanging();
}

bool UGameProjectSummer26ParkourMovementComponent::TryClimbUp()
{
	if (!IsHanging() || !CharacterOwner || !UpdatedComponent)
	{
		DisplayHangState(this, TEXT("CLIMB FAIL - character is not hanging."), FColor::Red);
		return false;
	}

	if (bIsClimbingUpFromHang)
	{
		return true;
	}

	const AGameProjectSummer26ParkourCharacter* ParkourCharacter =
		Cast<AGameProjectSummer26ParkourCharacter>(CharacterOwner);
	const UGameProjectSummer26ParkourTraversalComponent* Traversal =
		ParkourCharacter ? ParkourCharacter->GetParkourTraversalComponent() : nullptr;
	FVector ClimbLocation;
	if (!Traversal || !Traversal->QueryClimbUpLocation(CurrentHangTarget, ClimbLocation))
	{
		DisplayHangState(this, TEXT("CLIMB FAIL - destination capsule is blocked or traversal component is missing."), FColor::Red);
		return false;
	}

	StopMovementImmediately();
	PendingClimbUpLocation = ClimbLocation;
	HangClimbUpElapsedTime = 0.0f;
	bIsClimbingUpFromHang = true;
	if (AGameProjectSummer26ParkourCharacter* MutableParkourCharacter =
		Cast<AGameProjectSummer26ParkourCharacter>(CharacterOwner))
	{
		MutableParkourCharacter->RefreshHangAnimation();
	}
	DisplayHangState(
		this,
		FString::Printf(TEXT("CLIMB START PASS - animation target %s."), *ClimbLocation.ToCompactString()),
		FColor::Green);
	return true;
}

void UGameProjectSummer26ParkourMovementComponent::CompleteClimbUp()
{
	if (!bIsClimbingUpFromHang || !CharacterOwner || !UpdatedComponent)
	{
		return;
	}

	const FVector ClimbLocation = PendingClimbUpLocation;
	const FQuat FacingRotation = (-CurrentHangTarget.WallNormal).Rotation().Quaternion();
	bIsClimbingUpFromHang = false;
	HangClimbUpElapsedTime = 0.0f;
	StopMovementImmediately();
	UpdatedComponent->SetWorldLocationAndRotation(
		ClimbLocation,
		FacingRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	FindFloor(ClimbLocation, CurrentFloor, false);
	DisplayHangState(
		this,
		FString::Printf(TEXT("CLIMB COMPLETE - moved to %s."), *ClimbLocation.ToCompactString()),
		FColor::Green);
	SetMovementMode(CurrentFloor.IsWalkableFloor() ? MOVE_Walking : MOVE_Falling);
}

void UGameProjectSummer26ParkourMovementComponent::DropFromHang()
{
	if (!IsHanging() || bIsClimbingUpFromHang)
	{
		return;
	}

	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	const FVector DropNormal = CurrentHangTarget.WallNormal.GetSafeNormal2D();
	const float OutwardSpeed = Settings ? Settings->HangDropOutwardSpeed : 120.0f;
	const float DownwardSpeed = Settings ? Settings->HangDropDownwardSpeed : 80.0f;
	DisplayHangState(this, TEXT("DROP - released the ledge and entered Falling."), FColor::Yellow);
	SetMovementMode(MOVE_Falling);
	Velocity = DropNormal * OutwardSpeed - FVector::UpVector * DownwardSpeed;
}

bool UGameProjectSummer26ParkourMovementComponent::JumpAwayFromHang()
{
	if (!IsHanging() || bIsClimbingUpFromHang)
	{
		DisplayHangState(this, TEXT("WALL JUMP FAIL - character is not hanging."), FColor::Red);
		return false;
	}

	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	const FVector WallNormal = CurrentHangTarget.WallNormal.GetSafeNormal2D();
	// Clamp legacy Data Assets too: older assets may still contain the former 550/450 defaults.
	const float OutwardSpeed = FMath::Min(
		Settings ? Settings->HangWallJumpOutwardSpeed : 300.0f,
		300.0f);
	const float UpwardSpeed = FMath::Min(
		Settings ? Settings->HangWallJumpUpwardSpeed : 250.0f,
		250.0f);
	DisplayHangState(
		this,
		FString::Printf(
			TEXT("WALL JUMP - outward %.0f, upward %.0f."),
			OutwardSpeed,
			UpwardSpeed),
		FColor::Cyan);
	SetMovementMode(MOVE_Falling);
	Velocity = WallNormal * OutwardSpeed + FVector::UpVector * UpwardSpeed;
	return true;
}

float UGameProjectSummer26ParkourMovementComponent::GetHangMovementAlpha() const
{
	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	return Settings && Settings->HangShimmySpeed > KINDA_SMALL_NUMBER
		? FMath::Clamp(CurrentHangSpeed / Settings->HangShimmySpeed, -1.0f, 1.0f)
		: 0.0f;
}

EGameProjectSummer26HangAnimationState
UGameProjectSummer26ParkourMovementComponent::GetHangAnimationState() const
{
	if (!IsHanging())
	{
		return EGameProjectSummer26HangAnimationState::None;
	}

	if (bIsClimbingUpFromHang)
	{
		return EGameProjectSummer26HangAnimationState::ClimbUp;
	}

	const float MovementAlpha = GetHangMovementAlpha();
	if (MovementAlpha < -0.05f)
	{
		return EGameProjectSummer26HangAnimationState::ShimmyLeft;
	}
	if (MovementAlpha > 0.05f)
	{
		return EGameProjectSummer26HangAnimationState::ShimmyRight;
	}

	return EGameProjectSummer26HangAnimationState::Idle;
}

float UGameProjectSummer26ParkourMovementComponent::GetHangClimbUpProgress() const
{
	const float Duration = GetHangClimbUpDuration();
	return bIsClimbingUpFromHang
		? FMath::Clamp(HangClimbUpElapsedTime / Duration, 0.0f, 1.0f)
		: 0.0f;
}

float UGameProjectSummer26ParkourMovementComponent::GetHangClimbUpDuration() const
{
	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	if (Settings)
	{
		if (UAnimSequenceBase* ClimbAnimation = Settings->HangClimbUpAnimation.LoadSynchronous())
		{
			return FMath::Max(ClimbAnimation->GetPlayLength(), 0.05f);
		}
		return FMath::Max(Settings->HangClimbUpDuration, 0.05f);
	}
	return 1.25f;
}

EGameProjectSummer26ParkourMovementMode
UGameProjectSummer26ParkourMovementComponent::GetParkourMovementMode() const
{
	if (MovementMode != MOVE_Custom)
	{
		return EGameProjectSummer26ParkourMovementMode::None;
	}

	return static_cast<EGameProjectSummer26ParkourMovementMode>(CustomMovementMode);
}

float UGameProjectSummer26ParkourMovementComponent::GetMaxSpeed() const
{
	if (IsHanging())
	{
		const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
		return Settings ? Settings->HangShimmySpeed : 0.0f;
	}

	if (IsSliding())
	{
		const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
		return Settings ? FMath::Max(Settings->MinimumSlideInitialSpeed, Velocity.Size2D()) : Super::GetMaxSpeed();
	}

	return Super::GetMaxSpeed();
}

bool UGameProjectSummer26ParkourMovementComponent::CanStartSlide() const
{
	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	return Settings && IsMovingOnGround() && Velocity.Size2D() >= Settings->MinimumSlideStartSpeed;
}

void UGameProjectSummer26ParkourMovementComponent::StartSlide()
{
	if (!CanStartSlide())
	{
		return;
	}

	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	FVector SlideDirection = Velocity.GetSafeNormal2D();
	if (SlideDirection.IsNearlyZero() && CharacterOwner)
	{
		SlideDirection = CharacterOwner->GetActorForwardVector().GetSafeNormal2D();
	}

	const float InitialSpeed = FMath::Max(
		Velocity.Size2D() + Settings->SlideInitialBoost,
		Settings->MinimumSlideInitialSpeed);
	Velocity = SlideDirection * InitialSpeed;
	SetMovementMode(MOVE_Custom, static_cast<uint8>(EGameProjectSummer26ParkourMovementMode::Sliding));
}

void UGameProjectSummer26ParkourMovementComponent::StopSlide()
{
	if (!IsSliding())
	{
		return;
	}

	SetMovementMode(CurrentFloor.IsWalkableFloor() ? MOVE_Walking : MOVE_Falling);
}

void UGameProjectSummer26ParkourMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (bWantsToSlide && !IsSliding())
	{
		StartSlide();
	}
	else if (!bWantsToSlide && IsSliding())
	{
		StopSlide();
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UGameProjectSummer26ParkourMovementComponent::OnMovementModeChanged(
	EMovementMode PreviousMovementMode,
	uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Custom &&
		PreviousCustomMode == static_cast<uint8>(EGameProjectSummer26ParkourMovementMode::Hanging) &&
		!IsHanging())
	{
		CurrentHangTarget.Reset();
		CurrentHangSpeed = 0.0f;
		PendingClimbUpLocation = FVector::ZeroVector;
		HangClimbUpElapsedTime = 0.0f;
		bIsClimbingUpFromHang = false;
		bHangEndReported = false;
		bHangVerticalInputArmed = false;
		bHangVerticalInputLatched = false;
	}

	if (AGameProjectSummer26ParkourCharacter* ParkourCharacter =
		Cast<AGameProjectSummer26ParkourCharacter>(CharacterOwner))
	{
		ParkourCharacter->RefreshHangAnimation();
	}

	if (!CharacterOwner)
	{
		return;
	}

	if (IsSliding())
	{
		CharacterOwner->Crouch();
	}
	else if (PreviousMovementMode == MOVE_Custom &&
		PreviousCustomMode == static_cast<uint8>(EGameProjectSummer26ParkourMovementMode::Sliding) &&
		!bWantsToCrouch)
	{
		CharacterOwner->UnCrouch();
	}
}

void UGameProjectSummer26ParkourMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	switch (GetParkourMovementMode())
	{
	case EGameProjectSummer26ParkourMovementMode::Sliding:
		PhysSliding(DeltaTime, Iterations);
		return;

	case EGameProjectSummer26ParkourMovementMode::Hanging:
		PhysHanging(DeltaTime, Iterations);
		return;

	case EGameProjectSummer26ParkourMovementMode::WallRunning:
	case EGameProjectSummer26ParkourMovementMode::WallClimbing:
		// These modes are enabled only when their dedicated physics is implemented.
		// Falling is the safe recovery path, preventing a character from remaining stuck in MOVE_Custom.
		SetMovementMode(MOVE_Falling);
		return;

	default:
		Super::PhysCustom(DeltaTime, Iterations);
		return;
	}
}

void UGameProjectSummer26ParkourMovementComponent::PhysHanging(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME || !CharacterOwner || !UpdatedComponent ||
		!CurrentHangTarget.IsValid())
	{
		DisplayHangState(this, TEXT("RECOVERY - invalid hanging state, switching to Falling."), FColor::Red);
		SetMovementMode(MOVE_Falling);
		return;
	}

	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	const AGameProjectSummer26ParkourCharacter* ParkourCharacter =
		Cast<AGameProjectSummer26ParkourCharacter>(CharacterOwner);
	const UGameProjectSummer26ParkourTraversalComponent* Traversal =
		ParkourCharacter ? ParkourCharacter->GetParkourTraversalComponent() : nullptr;
	if (!Settings || !Traversal)
	{
		DisplayHangState(this, TEXT("RECOVERY - missing settings/traversal component, switching to Falling."), FColor::Red);
		SetMovementMode(MOVE_Falling);
		return;
	}

	if (bIsClimbingUpFromHang)
	{
		StopMovementImmediately();
		HangClimbUpElapsedTime += DeltaTime;
		if (HangClimbUpElapsedTime >= FMath::Max(Settings->HangClimbUpDuration, 0.05f))
		{
			CompleteClimbUp();
		}
		return;
	}

	// LedgeDirection is authored as Up x WallNormal, which points to the
	// character left while facing the wall. Derive an explicit right tangent so
	// positive movement input always means D/right and negative means A/left.
	const FVector HangRightDirection = -CurrentHangTarget.LedgeDirection.GetSafeNormal2D();
	const FVector InputDirection = Acceleration.GetSafeNormal2D();
	const FVector TowardWall = -CurrentHangTarget.WallNormal.GetSafeNormal2D();
	float SideInput = InputDirection.IsNearlyZero()
		? 0.0f
		: FMath::Clamp(FVector::DotProduct(InputDirection, HangRightDirection), -1.0f, 1.0f);
	float VerticalInput = InputDirection.IsNearlyZero()
		? 0.0f
		: FMath::Clamp(FVector::DotProduct(InputDirection, TowardWall), -1.0f, 1.0f);
	// The sample converts IA_Move into a camera-relative world direction. During a hang,
	// that is unsafe: at an oblique camera angle A/D can project toward/away from the wall
	// and be misread as W/S. Treat the keyboard axes as explicit hang commands instead.
	if (const APlayerController* PlayerController =
		Cast<APlayerController>(CharacterOwner->GetController()))
	{
		if (PlayerController->IsLocalController())
		{
			const bool bWPressed = PlayerController->IsInputKeyDown(EKeys::W);
			const bool bSPressed = PlayerController->IsInputKeyDown(EKeys::S);
			const bool bAPressed = PlayerController->IsInputKeyDown(EKeys::A);
			const bool bDPressed = PlayerController->IsInputKeyDown(EKeys::D);
			if (bWPressed || bSPressed || bAPressed || bDPressed)
			{
				VerticalInput = static_cast<float>(bWPressed) - static_cast<float>(bSPressed);
				SideInput = static_cast<float>(bDPressed) - static_cast<float>(bAPressed);
			}
		}
	}
	const float VerticalThreshold = Settings->HangVerticalInputThreshold;
	const float VerticalReleaseThreshold = VerticalThreshold * 0.5f;
	const bool bVerticalInputIsNeutral = FMath::Abs(VerticalInput) <= VerticalReleaseThreshold;
	const bool bVerticalInputIsDominant =
		FMath::Abs(VerticalInput) >= FMath::Abs(SideInput) + 0.15f;
	if (bVerticalInputIsNeutral)
	{
		if (!bHangVerticalInputArmed)
		{
			DisplayHangState(this, TEXT("INPUT ARMED - W climbs, S drops, A/D shimmy."), FColor::Cyan, 2);
		}
		bHangVerticalInputArmed = true;
		bHangVerticalInputLatched = false;
	}
	else if (bHangVerticalInputArmed && !bHangVerticalInputLatched &&
		bVerticalInputIsDominant && VerticalInput >= VerticalThreshold)
	{
		bHangVerticalInputLatched = true;
		DisplayHangState(this, TEXT("W INPUT - trying Climb Up."), FColor::Cyan);
		if (TryClimbUp())
		{
			return;
		}
	}
	else if (bHangVerticalInputArmed && !bHangVerticalInputLatched &&
		bVerticalInputIsDominant && VerticalInput <= -VerticalThreshold)
	{
		bHangVerticalInputLatched = true;
		DisplayHangState(this, TEXT("S INPUT - dropping from the ledge."), FColor::Yellow);
		DropFromHang();
		return;
	}

	const float TargetSpeed = SideInput * Settings->HangShimmySpeed;
	CurrentHangSpeed = FMath::FInterpConstantTo(
		CurrentHangSpeed,
		TargetSpeed,
		DeltaTime,
		Settings->HangShimmyAcceleration);
	Velocity = HangRightDirection * CurrentHangSpeed;

	if (FMath::IsNearlyZero(CurrentHangSpeed, 0.5f))
	{
		CurrentHangSpeed = 0.0f;
		Velocity = FVector::ZeroVector;
		bHangEndReported = false;
		const_cast<AGameProjectSummer26ParkourCharacter*>(ParkourCharacter)->RefreshHangAnimation();
		return;
	}

	const FVector CandidateProbeLocation =
		UpdatedComponent->GetComponentLocation() + Velocity * DeltaTime;
	FGameProjectSummer26HangTarget CandidateTarget;
	const bool bHasCandidate = Traversal->QueryLedgeGrabFromLocation(
		CandidateProbeLocation,
		-CurrentHangTarget.WallNormal,
		CandidateTarget);
	const bool bSameStraightLedge =
		bHasCandidate &&
		FVector::DotProduct(CandidateTarget.WallNormal, CurrentHangTarget.WallNormal) >= 0.95f &&
		FMath::Abs(CandidateTarget.LedgeLocation.Z - CurrentHangTarget.LedgeLocation.Z) <= 20.0f;
	if (!bSameStraightLedge)
	{
		if (!bHangEndReported)
		{
			DisplayHangState(this, TEXT("SHIMMY STOP - no compatible straight ledge in that direction."), FColor::Yellow);
			bHangEndReported = true;
		}
		CurrentHangSpeed = 0.0f;
		Velocity = FVector::ZeroVector;
		const_cast<AGameProjectSummer26ParkourCharacter*>(ParkourCharacter)->RefreshHangAnimation();
		return;
	}

	const FVector Delta = CandidateTarget.HangLocation - UpdatedComponent->GetComponentLocation();
	const FQuat FacingRotation = (-CandidateTarget.WallNormal).Rotation().Quaternion();
	FHitResult Hit(1.0f);
	SafeMoveUpdatedComponent(Delta, FacingRotation, true, Hit);
	if (Hit.IsValidBlockingHit())
	{
		if (!bHangEndReported)
		{
			DisplayHangState(this, TEXT("SHIMMY STOP - capsule movement hit blocking collision."), FColor::Yellow);
			bHangEndReported = true;
		}
		CurrentHangSpeed = 0.0f;
		Velocity = FVector::ZeroVector;
		const_cast<AGameProjectSummer26ParkourCharacter*>(ParkourCharacter)->RefreshHangAnimation();
		return;
	}

	CurrentHangTarget = CandidateTarget;
	bHangEndReported = false;
	const_cast<AGameProjectSummer26ParkourCharacter*>(ParkourCharacter)->RefreshHangAnimation();
}

void UGameProjectSummer26ParkourMovementComponent::PhysSliding(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME || !CharacterOwner || !UpdatedComponent)
	{
		return;
	}

	const UGameProjectSummer26ParkourSettings* Settings = GetParkourSettings();
	if (!Settings)
	{
		bWantsToSlide = false;
		StopSlide();
		return;
	}

	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, false);
	if (!CurrentFloor.IsWalkableFloor())
	{
		bWantsToSlide = false;
		SetMovementMode(MOVE_Falling);
		return;
	}

	const FVector FloorNormal = CurrentFloor.HitResult.ImpactNormal.GetSafeNormal();
	FVector SlideDirection = Velocity.GetSafeNormal2D();
	if (SlideDirection.IsNearlyZero())
	{
		SlideDirection = CharacterOwner->GetActorForwardVector().GetSafeNormal2D();
	}

	const FVector DownSlope = FVector::VectorPlaneProject(FVector::DownVector, FloorNormal).GetSafeNormal();
	const float SlopeInfluence = FMath::Clamp(FVector::DotProduct(DownSlope, SlideDirection), -1.0f, 1.0f);
	float SlideSpeed = Velocity.Size2D();
	SlideSpeed += Settings->SlideSlopeAcceleration * SlopeInfluence * DeltaTime;
	SlideSpeed = FMath::Max(0.0f, SlideSpeed - Settings->SlideDeceleration * DeltaTime);

	if (SlideSpeed <= Settings->SlideEndSpeed)
	{
		Velocity.X = 0.0f;
		Velocity.Y = 0.0f;
		bWantsToSlide = false;
		StopSlide();
		return;
	}

	Velocity = FVector::VectorPlaneProject(SlideDirection * SlideSpeed, FloorNormal);
	FHitResult Hit(1.0f);
	const FVector Delta = Velocity * DeltaTime;
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
	if (Hit.IsValidBlockingHit())
	{
		HandleImpact(Hit, DeltaTime, Delta);
		SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit, true);
	}
}

void UGameProjectSummer26ParkourMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToSlide = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* UGameProjectSummer26ParkourMovementComponent::GetPredictionData_Client() const
{
	check(CharacterOwner != nullptr);

	if (!ClientPredictionData)
	{
		UGameProjectSummer26ParkourMovementComponent* MutableThis =
			const_cast<UGameProjectSummer26ParkourMovementComponent*>(this);
		MutableThis->ClientPredictionData =
			new FGameProjectSummer26NetworkPredictionData_Client_Parkour(*this);
	}

	return ClientPredictionData;
}

void FGameProjectSummer26SavedMove_Parkour::Clear()
{
	Super::Clear();
	bSavedWantsToSlide = false;
}

uint8 FGameProjectSummer26SavedMove_Parkour::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bSavedWantsToSlide)
	{
		Result |= FLAG_Custom_0;
	}
	return Result;
}

bool FGameProjectSummer26SavedMove_Parkour::CanCombineWith(
	const FSavedMovePtr& NewMove,
	ACharacter* Character,
	float MaxDelta) const
{
	const FGameProjectSummer26SavedMove_Parkour* NewParkourMove =
		static_cast<const FGameProjectSummer26SavedMove_Parkour*>(NewMove.Get());
	if (bSavedWantsToSlide != NewParkourMove->bSavedWantsToSlide)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FGameProjectSummer26SavedMove_Parkour::SetMoveFor(
	ACharacter* Character,
	float InDeltaTime,
	const FVector& NewAcceleration,
	FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAcceleration, ClientData);
	const UGameProjectSummer26ParkourMovementComponent* Movement = Character
		? Cast<UGameProjectSummer26ParkourMovementComponent>(Character->GetCharacterMovement())
		: nullptr;
	bSavedWantsToSlide = Movement && Movement->WantsToSlide();
}

void FGameProjectSummer26SavedMove_Parkour::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	if (UGameProjectSummer26ParkourMovementComponent* Movement = Character
		? Cast<UGameProjectSummer26ParkourMovementComponent>(Character->GetCharacterMovement())
		: nullptr)
	{
		Movement->SetWantsToSlide(bSavedWantsToSlide);
	}
}

FGameProjectSummer26NetworkPredictionData_Client_Parkour::
FGameProjectSummer26NetworkPredictionData_Client_Parkour(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FGameProjectSummer26NetworkPredictionData_Client_Parkour::AllocateNewMove()
{
	return FSavedMovePtr(new FGameProjectSummer26SavedMove_Parkour());
}
