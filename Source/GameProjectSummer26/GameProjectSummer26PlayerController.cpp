// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameProjectSummer26PlayerController.h"
#include "AbilitySystem/GameProjectSummer26AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "GameProjectSummer26.h"
#include "UI/GameProjectSummer26StatusWidget.h"
#include "Widgets/Input/SVirtualJoystick.h"

AGameProjectSummer26PlayerController::AGameProjectSummer26PlayerController()
{
	StatusWidgetClass = UGameProjectSummer26StatusWidget::StaticClass();
}

void AGameProjectSummer26PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController() && StatusWidgetClass && !StatusWidget)
	{
		StatusWidget = CreateWidget<UGameProjectSummer26StatusWidget>(this, StatusWidgetClass);
		if (StatusWidget)
		{
			StatusWidget->AddToPlayerScreen(10);
		}
	}

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogGameProjectSummer26, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AGameProjectSummer26PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AGameProjectSummer26PlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetPawn()))
	{
		if (UGameProjectSummer26AbilitySystemComponent* GameASC = Cast<UGameProjectSummer26AbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent()))
		{
			GameASC->ProcessAbilityInput(DeltaTime, bGamePaused);
		}
	}
}

bool AGameProjectSummer26PlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
