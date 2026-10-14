// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkourChooserRepairCommandlet.h"

#include "Animation/AnimBlueprint.h"
#include "AssetToolsModule.h"
#include "Chooser.h"
#include "ChooserPropertyAccess.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "Serialization/ArchiveReplaceObjectRef.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogParkourChooserRepair, Log, All);

UGameProjectSummer26ParkourChooserRepairCommandlet::
	UGameProjectSummer26ParkourChooserRepairCommandlet()
{
	IsClient = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UGameProjectSummer26ParkourChooserRepairCommandlet::Main(const FString& Params)
{
		constexpr TCHAR ParkourAnimBlueprintPath[] =
			TEXT("/Game/Animations/Parkour/Hang/ABP_ParkourCharacter.ABP_ParkourCharacter");
		constexpr TCHAR DestinationFolder[] = TEXT("/Game/Portfolio/Parkour/Animation/Choosers");

		struct FChooserRepairSpec
		{
			const TCHAR* SourcePath;
			const TCHAR* DestinationName;
		};

		const FChooserRepairSpec RepairSpecs[] = {
			{
				TEXT("/Game/Characters/UEFN_Mannequin/Animations/ExperimentalStateMachineData/CHT_CMCCharacterAnimations.CHT_CMCCharacterAnimations"),
				TEXT("CHT_Parkour_CMCCharacterAnimations")
			},
			{
				TEXT("/Game/Characters/UEFN_Mannequin/Animations/MotionMatchingData/CHT_PoseSearchDatabases.CHT_PoseSearchDatabases"),
				TEXT("CHT_Parkour_PoseSearchDatabases")
			},
			{
				TEXT("/Game/Characters/UEFN_Mannequin/Animations/MotionMatchingData/CHT_PoseSearchDatabases_Dense.CHT_PoseSearchDatabases_Dense"),
				TEXT("CHT_Parkour_PoseSearchDatabases_Dense")
			},
		};

		if (!FParse::Param(*Params, TEXT("Apply")))
		{
			UE_LOG(LogParkourChooserRepair, Display, TEXT("Dry run complete. Re-run with -Apply to save the repair."));
			return 0;
		}

		UAnimBlueprint* ParkourAnimBlueprint = LoadObject<UAnimBlueprint>(nullptr, ParkourAnimBlueprintPath);
		if (!ParkourAnimBlueprint || !ParkourAnimBlueprint->GeneratedClass)
		{
			UE_LOG(LogParkourChooserRepair, Error, TEXT("Could not load the Parkour animation blueprint or its generated class."));
			return 1;
		}

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		TMap<UObject*, UObject*> ReplacementMap;
		TArray<UObject*> AssetsToSave;

		for (const FChooserRepairSpec& Spec : RepairSpecs)
		{
			UChooserTable* SourceChooser = LoadObject<UChooserTable>(nullptr, Spec.SourcePath);
			if (!SourceChooser)
			{
				UE_LOG(LogParkourChooserRepair, Error, TEXT("Could not load source Chooser %s."), Spec.SourcePath);
				return 1;
			}

			const FString DestinationPath = FString::Printf(
				TEXT("%s/%s.%s"), DestinationFolder, Spec.DestinationName, Spec.DestinationName);
			UChooserTable* ParkourChooser = LoadObject<UChooserTable>(nullptr, *DestinationPath);
			if (!ParkourChooser)
			{
				ParkourChooser = Cast<UChooserTable>(AssetToolsModule.Get().DuplicateAsset(
					Spec.DestinationName, DestinationFolder, SourceChooser));
			}

			if (!ParkourChooser)
			{
				UE_LOG(LogParkourChooserRepair, Error, TEXT("Could not create Parkour Chooser copy %s."), *DestinationPath);
				return 1;
			}

			bool bFoundClassContext = false;
			ParkourChooser->Modify();
			for (FInstancedStruct& ContextData : ParkourChooser->ContextData)
			{
				if (FContextObjectTypeClass* ClassContext =
					ContextData.GetMutablePtr<FContextObjectTypeClass>())
				{
					ClassContext->Class = ParkourAnimBlueprint->GeneratedClass;
					bFoundClassContext = true;
				}
			}

			if (!bFoundClassContext)
			{
				UE_LOG(LogParkourChooserRepair, Error, TEXT("Chooser %s did not contain a class context."), *DestinationPath);
				return 1;
			}

			ParkourChooser->Compile(true);
			ParkourChooser->MarkPackageDirty();
			ReplacementMap.Add(SourceChooser, ParkourChooser);
			AssetsToSave.Add(ParkourChooser);
		}

		TArray<UObject*> BlueprintObjects;
		GetObjectsWithOuter(
			ParkourAnimBlueprint,
			BlueprintObjects,
			EGetObjectsFlags::IncludeNestedObjects);
		BlueprintObjects.Add(ParkourAnimBlueprint);
		for (UObject* BlueprintObject : BlueprintObjects)
		{
			FArchiveReplaceObjectRef<UObject> ReplaceReferences(BlueprintObject, ReplacementMap);
		}

		ParkourAnimBlueprint->Modify();
		FBlueprintEditorUtils::RefreshAllNodes(ParkourAnimBlueprint);
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(ParkourAnimBlueprint);
		FKismetEditorUtilities::CompileBlueprint(ParkourAnimBlueprint);
		if (ParkourAnimBlueprint->Status == BS_Error)
		{
			UE_LOG(LogParkourChooserRepair, Error, TEXT("Parkour animation Blueprint compilation failed; no packages were saved."));
			return 1;
		}

		ParkourAnimBlueprint->MarkPackageDirty();
		AssetsToSave.Add(ParkourAnimBlueprint);
		for (UObject* Asset : AssetsToSave)
		{
			UPackage* Package = Asset->GetOutermost();
			const FString Filename = FPackageName::LongPackageNameToFilename(
				Package->GetName(), FPackageName::GetAssetPackageExtension());
			FSavePackageArgs SaveArguments;
			SaveArguments.TopLevelFlags = RF_Public | RF_Standalone;
			if (!UPackage::SavePackage(Package, Asset, *Filename, SaveArguments))
			{
				UE_LOG(LogParkourChooserRepair, Error, TEXT("Failed to save %s."), *Package->GetName());
				return 1;
			}
		}

		UE_LOG(
			LogParkourChooserRepair,
			Display,
			TEXT("Parkour Chooser repair completed. Shared GASP Choosers remain unchanged."));
		return 0;
}
