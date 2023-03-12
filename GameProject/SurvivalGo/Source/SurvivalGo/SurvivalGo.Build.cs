// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SurvivalGo : ModuleRules
{
	public SurvivalGo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(new string[] 
		{
			"SurvivalGo/Gameplay",
			"SurvivalGo/MapElements",
			"SurvivalGo/UI",
			"SurvivalGo/Utils",
		});

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"Json",
			"JsonUtilities",
            "HeadMountedDisplay",
            "EnhancedInput",
        });

		PrivateDependencyModuleNames.AddRange(new string[] 
		{  
			"Slate",
			"SlateCore",
			"UMG",
			"GameplayTags",
			"AIModule",
			"ChessManipulator",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
