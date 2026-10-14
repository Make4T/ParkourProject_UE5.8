// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameProjectSummer26 : ModuleRules
{
	public GameProjectSummer26(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"GameProjectSummer26",
			"GameProjectSummer26/Variant_Platforming",
			"GameProjectSummer26/Variant_Platforming/Animation",
			"GameProjectSummer26/Variant_Combat",
			"GameProjectSummer26/Variant_Combat/AI",
			"GameProjectSummer26/Variant_Combat/Animation",
			"GameProjectSummer26/Variant_Combat/Gameplay",
			"GameProjectSummer26/Variant_Combat/Interfaces",
			"GameProjectSummer26/Variant_Combat/UI",
			"GameProjectSummer26/Variant_SideScrolling",
			"GameProjectSummer26/Variant_SideScrolling/AI",
			"GameProjectSummer26/Variant_SideScrolling/Gameplay",
			"GameProjectSummer26/Variant_SideScrolling/Interfaces",
			"GameProjectSummer26/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
