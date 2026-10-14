// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"

#include "ParkourChooserRepairCommandlet.generated.h"

/** Repairs the Parkour-owned Chooser integration without changing GASP source assets. */
UCLASS()
class GAMEPROJECTSUMMER26EDITORTOOLS_API UGameProjectSummer26ParkourChooserRepairCommandlet
	: public UCommandlet
{
	GENERATED_BODY()

public:
	UGameProjectSummer26ParkourChooserRepairCommandlet();

	virtual int32 Main(const FString& Params) override;
};
