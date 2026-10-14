// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameProjectSummer26PlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UGameProjectSummer26StatusWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AGameProjectSummer26PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGameProjectSummer26PlayerController();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Native status widget shown for every local player. Can be replaced by a Widget Blueprint subclass. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGameProjectSummer26StatusWidget> StatusWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UGameProjectSummer26StatusWidget> StatusWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Processes tag-based ability input after Enhanced Input has updated this frame. */
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

};
