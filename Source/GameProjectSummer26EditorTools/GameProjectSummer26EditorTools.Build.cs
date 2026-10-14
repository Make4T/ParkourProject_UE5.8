// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameProjectSummer26EditorTools : ModuleRules
{
	public GameProjectSummer26EditorTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"AssetTools",
			"Chooser",
			"KismetCompiler",
			"UnrealEd"
		});
	}
}
