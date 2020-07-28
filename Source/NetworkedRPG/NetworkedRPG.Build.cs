// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetworkedRPG : ModuleRules
{
	public NetworkedRPG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule" });

		PrivateDependencyModuleNames.AddRange(new string[]
        {
			"Slate",
			"SlateCore",
			"OnlineSubsystem",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
        });
	}
}
