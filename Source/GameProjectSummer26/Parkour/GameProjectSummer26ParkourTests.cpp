// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Parkour/GameProjectSummer26ParkourCharacter.h"
#include "AbilitySystem/GameProjectSummer26TraversalAbility.h"
#include "GameProjectSummer26GameplayTags.h"
#include "HAL/IConsoleManager.h"
#include "Parkour/GameProjectSummer26ParkourMovementComponent.h"
#include "Parkour/GameProjectSummer26ParkourSettings.h"
#include "Parkour/GameProjectSummer26ParkourTraversalComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameProjectSummer26ParkourFoundationTest,
	"GameProjectSummer26.Parkour.Foundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGameProjectSummer26ParkourFoundationTest::RunTest(const FString& Parameters)
{
	const AGameProjectSummer26ParkourCharacter* CharacterCDO =
		GetDefault<AGameProjectSummer26ParkourCharacter>();
	TestNotNull(TEXT("Parkour Character CDO exists"), CharacterCDO);
	if (!CharacterCDO)
	{
		return false;
	}

	const UGameProjectSummer26ParkourMovementComponent* Movement =
		CharacterCDO->GetParkourMovementComponent();
	TestNotNull(TEXT("Character uses the custom parkour movement component"), Movement);
	TestNotNull(TEXT("Character owns the traversal detection component"), CharacterCDO->GetParkourTraversalComponent());
	TestNotNull(TEXT("Default parkour settings are available"), Movement ? Movement->GetParkourSettings() : nullptr);

	if (Movement)
	{
		TestEqual(
			TEXT("The safe initial parkour movement mode is None"),
			Movement->GetParkourMovementMode(),
			EGameProjectSummer26ParkourMovementMode::None);
	}

	TestTrue(TEXT("Traversal input tag is registered"), GameProjectSummer26GameplayTags::Input_Ability_Traversal.GetTag().IsValid());
	TestTrue(TEXT("Traversal ability tag is registered"), GameProjectSummer26GameplayTags::Ability_Movement_Traversal.GetTag().IsValid());
	TestTrue(TEXT("Traversal state tag is registered"), GameProjectSummer26GameplayTags::State_Movement_Traversing.GetTag().IsValid());
	TestTrue(TEXT("Ledge grab ability tag is registered"), GameProjectSummer26GameplayTags::Ability_Movement_LedgeGrab.GetTag().IsValid());
	TestTrue(TEXT("Hanging state tag is registered"), GameProjectSummer26GameplayTags::State_Movement_Hanging.GetTag().IsValid());
	TestNotNull(
		TEXT("parkour.DebugHang console variable is registered"),
		IConsoleManager::Get().FindConsoleVariable(TEXT("parkour.DebugHang")));
	TestNotNull(TEXT("Traversal GAS ability CDO exists"), GetDefault<UGameProjectSummer26TraversalAbility>());
	TestNotNull(
		TEXT("Parkour Character exposes JumpAwayFromHang to Blueprint"),
		CharacterCDO->FindFunction(TEXT("JumpAwayFromHang")));
	TestNotNull(
		TEXT("Movement exposes the hang animation state to Blueprint"),
		Movement ? Movement->FindFunction(TEXT("GetHangAnimationState")) : nullptr);
	TestNotNull(
		TEXT("Movement exposes climb-up animation progress to Blueprint"),
		Movement ? Movement->FindFunction(TEXT("GetHangClimbUpProgress")) : nullptr);

	const UGameProjectSummer26ParkourSettings* Settings =
		Movement ? Movement->GetParkourSettings() : nullptr;
	if (Settings)
	{
		TestTrue(TEXT("Hang forward query distance is positive"), Settings->HangForwardDistance > 0.0f);
		TestTrue(TEXT("Hang shimmy speed is positive"), Settings->HangShimmySpeed > 0.0f);
		TestTrue(TEXT("Hang climb-up animation duration is positive"), Settings->HangClimbUpDuration > 0.0f);
		TestNotNull(TEXT("Hang idle animation loads"), Settings->HangIdleAnimation.LoadSynchronous());
		TestNotNull(TEXT("Hang shimmy-left animation loads"), Settings->HangShimmyLeftAnimation.LoadSynchronous());
		TestNotNull(TEXT("Hang shimmy-right animation loads"), Settings->HangShimmyRightAnimation.LoadSynchronous());
		TestNotNull(TEXT("Hang climb-up animation loads"), Settings->HangClimbUpAnimation.LoadSynchronous());
		TestTrue(TEXT("Hang wall jump outward speed is positive"), Settings->HangWallJumpOutwardSpeed > 0.0f);
		TestTrue(TEXT("Hang wall jump upward speed is positive"), Settings->HangWallJumpUpwardSpeed > 0.0f);
		TestTrue(
			TEXT("Hang vertical input threshold is normalized"),
			Settings->HangVerticalInputThreshold > 0.0f &&
			Settings->HangVerticalInputThreshold <= 1.0f);
		TestTrue(
			TEXT("Hang ledge height range is ordered"),
			Settings->MinimumHangLedgeOffsetZ < Settings->MaximumHangLedgeOffsetZ);
	}

	UClass* StagingCharacterClass = LoadClass<AGameProjectSummer26ParkourCharacter>(
		nullptr,
		TEXT("/Game/Blueprints/BP_PlayerCharacter_Parkour.BP_PlayerCharacter_Parkour_C"));
	TestNotNull(TEXT("Parkour staging Blueprint class loads"), StagingCharacterClass);
	const UFunction* GetInputsFunction = StagingCharacterClass
		? StagingCharacterClass->FindFunctionByName(TEXT("GetTraversalCheckInputs"))
		: nullptr;
	const FObjectPropertyBase* SampleComponentProperty = StagingCharacterClass
		? FindFProperty<FObjectPropertyBase>(StagingCharacterClass, TEXT("AC_TraversalLogic"))
		: nullptr;
	const UFunction* TryTraversalFunction = SampleComponentProperty && SampleComponentProperty->PropertyClass
		? SampleComponentProperty->PropertyClass->FindFunctionByName(TEXT("TryTraversalAction"))
		: nullptr;
	const FStructProperty* ReturnInputsProperty = GetInputsFunction
		? FindFProperty<FStructProperty>(GetInputsFunction, TEXT("ReturnValue"))
		: nullptr;
	const FStructProperty* TryInputsProperty = TryTraversalFunction
		? FindFProperty<FStructProperty>(TryTraversalFunction, TEXT("Inputs"))
		: nullptr;
	TestNotNull(TEXT("Staging Blueprint exposes GetTraversalCheckInputs"), GetInputsFunction);
	TestNotNull(TEXT("Staging Blueprint exposes its AC_TraversalLogic component property"), SampleComponentProperty);
	TestNotNull(TEXT("AC_TraversalLogic exposes TryTraversalAction"), TryTraversalFunction);
	TestTrue(
		TEXT("The character and sample component use the same traversal input struct"),
		ReturnInputsProperty && TryInputsProperty && ReturnInputsProperty->Struct == TryInputsProperty->Struct);

	return !HasAnyErrors();
}

#endif
