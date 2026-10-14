// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "GameProjectSummer26StatusWidget.generated.h"

class SProgressBar;
class STextBlock;

/** Native status HUD for Health, Stamina, Mana and two Endurance dodge segments. */
UCLASS()
class GAMEPROJECTSUMMER26_API UGameProjectSummer26StatusWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void UpdateResource(
		const TSharedPtr<SProgressBar>& Bar,
		const TSharedPtr<STextBlock>& Text,
		const FText& Label,
		float Value,
		float Maximum) const;
	void UpdateDodge(float Value, float Maximum) const;

	TSharedPtr<SProgressBar> HealthBar;
	TSharedPtr<STextBlock> HealthText;
	TSharedPtr<SProgressBar> StaminaBar;
	TSharedPtr<STextBlock> StaminaText;
	TSharedPtr<SProgressBar> ManaBar;
	TSharedPtr<STextBlock> ManaText;
	TSharedPtr<SProgressBar> DodgeSegmentOneBar;
	TSharedPtr<SProgressBar> DodgeSegmentTwoBar;
	TSharedPtr<STextBlock> DodgeText;
};
