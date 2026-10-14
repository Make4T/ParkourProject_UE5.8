// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/GameProjectSummer26StatusWidget.h"

#include "AbilitySystem/GameProjectSummer26AttributeSet.h"
#include "GameProjectSummer26PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "GameProjectSummer26StatusWidget"

TSharedRef<SWidget> UGameProjectSummer26StatusWidget::RebuildWidget()
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(24.0f))
		[
			SNew(SBorder)
			.Padding(FMargin(14.0f))
			.BorderBackgroundColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.88f))
			[
				SNew(SBox)
				.WidthOverride(360.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 10.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("StatusTitle", "PLAYER STATUS"))
						.ColorAndOpacity(FLinearColor(0.9f, 0.95f, 1.0f, 1.0f))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 3.0f)
					[
						SNew(SBox)
						.HeightOverride(30.0f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SAssignNew(HealthBar, SProgressBar)
								.Percent(1.0f)
								.FillColorAndOpacity(FLinearColor(0.85f, 0.08f, 0.08f, 1.0f))
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SAssignNew(HealthText, STextBlock)
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 3.0f)
					[
						SNew(SBox)
						.HeightOverride(30.0f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SAssignNew(StaminaBar, SProgressBar)
								.Percent(1.0f)
								.FillColorAndOpacity(FLinearColor(0.12f, 0.8f, 0.22f, 1.0f))
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SAssignNew(StaminaText, STextBlock)
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 3.0f)
					[
						SNew(SBox)
						.HeightOverride(30.0f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SAssignNew(ManaBar, SProgressBar)
								.Percent(1.0f)
								.FillColorAndOpacity(FLinearColor(0.08f, 0.35f, 0.95f, 1.0f))
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SAssignNew(ManaText, STextBlock)
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 9.0f, 0.0f, 3.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 4.0f)
						[
							SAssignNew(DodgeText, STextBlock)
							.ColorAndOpacity(FLinearColor(0.9f, 0.82f, 0.2f, 1.0f))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 3.0f, 0.0f)
							[
								SNew(SBox)
								.HeightOverride(22.0f)
								[
									SAssignNew(DodgeSegmentOneBar, SProgressBar)
									.Percent(1.0f)
									.FillColorAndOpacity(FLinearColor(0.95f, 0.68f, 0.08f, 1.0f))
								]
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(3.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SBox)
								.HeightOverride(22.0f)
								[
									SAssignNew(DodgeSegmentTwoBar, SProgressBar)
									.Percent(1.0f)
									.FillColorAndOpacity(FLinearColor(0.95f, 0.68f, 0.08f, 1.0f))
								]
							]
						]
					]
				]
			]
		];
}

void UGameProjectSummer26StatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const APlayerController* PlayerController = GetOwningPlayer();
	const AGameProjectSummer26PlayerState* GamePlayerState =
		PlayerController ? PlayerController->GetPlayerState<AGameProjectSummer26PlayerState>() : nullptr;
	const UGameProjectSummer26AttributeSet* Attributes =
		GamePlayerState ? GamePlayerState->GetCoreAttributeSet() : nullptr;

	if (!Attributes)
	{
		return;
	}

	UpdateResource(HealthBar, HealthText, LOCTEXT("HealthLabel", "HP"), Attributes->GetHealth(), Attributes->GetMaxHealth());
	UpdateResource(StaminaBar, StaminaText, LOCTEXT("StaminaLabel", "STAMINA"), Attributes->GetStamina(), Attributes->GetMaxStamina());
	UpdateResource(ManaBar, ManaText, LOCTEXT("ManaLabel", "MANA"), Attributes->GetMana(), Attributes->GetMaxMana());
	UpdateDodge(Attributes->GetEndurance(), Attributes->GetMaxEndurance());
}

void UGameProjectSummer26StatusWidget::UpdateResource(
	const TSharedPtr<SProgressBar>& Bar,
	const TSharedPtr<STextBlock>& Text,
	const FText& Label,
	float Value,
	float Maximum) const
{
	const float SafeMaximum = FMath::Max(0.0f, Maximum);
	const float Percent = SafeMaximum > 0.0f ? FMath::Clamp(Value / SafeMaximum, 0.0f, 1.0f) : 0.0f;

	if (Bar.IsValid())
	{
		Bar->SetPercent(TOptional<float>(Percent));
	}

	if (Text.IsValid())
	{
		Text->SetText(FText::Format(
			LOCTEXT("ResourceFormat", "{0}  {1} / {2}"),
			Label,
			FText::AsNumber(FMath::RoundToInt(Value)),
			FText::AsNumber(FMath::RoundToInt(SafeMaximum))));
	}
}

void UGameProjectSummer26StatusWidget::UpdateDodge(float Value, float Maximum) const
{
	constexpr float SegmentSize = 50.0f;
	const float SafeMaximum = FMath::Max(0.0f, Maximum);
	const float SafeValue = FMath::Clamp(Value, 0.0f, SafeMaximum);
	const float SegmentOnePercent = FMath::Clamp(SafeValue / SegmentSize, 0.0f, 1.0f);
	const float SegmentTwoPercent = FMath::Clamp((SafeValue - SegmentSize) / SegmentSize, 0.0f, 1.0f);

	if (DodgeSegmentOneBar.IsValid())
	{
		DodgeSegmentOneBar->SetPercent(TOptional<float>(SegmentOnePercent));
	}
	if (DodgeSegmentTwoBar.IsValid())
	{
		DodgeSegmentTwoBar->SetPercent(TOptional<float>(SegmentTwoPercent));
	}
	if (DodgeText.IsValid())
	{
		const int32 ReadyCharges = FMath::Clamp(FMath::FloorToInt(SafeValue / SegmentSize), 0, 2);
		DodgeText->SetText(FText::Format(
			LOCTEXT("DodgeFormat", "DODGE  {0} / 2    ({1} / {2})"),
			FText::AsNumber(ReadyCharges),
			FText::AsNumber(FMath::RoundToInt(SafeValue)),
			FText::AsNumber(FMath::RoundToInt(SafeMaximum))));
	}
}

#undef LOCTEXT_NAMESPACE
