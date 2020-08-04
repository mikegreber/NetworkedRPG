// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetworkedMenu : ModuleRules
{
	public NetworkedMenu(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add("NetworkedMenu/Public");
		
		PrivateIncludePaths.Add("NetworkedMenu/Private");
		
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Slate",
				"SlateCore",
				"OnlineSubsystem",
			}
		);
	}
}
