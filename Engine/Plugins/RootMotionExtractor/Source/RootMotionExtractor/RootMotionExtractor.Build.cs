// Copyright2017 Yaki Studios, Inc. All Rights Reserved.

#if WITH_FORWARDED_MODULE_RULES_CTOR
#define UE_4_16_OR_LATER
# endif

using UnrealBuildTool;

#if UE_4_20_OR_LATER
using System.IO;
#endif

public class RootMotionExtractor : ModuleRules
{
	public RootMotionExtractor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;


#if UE_4_20_OR_LATER
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
#else
        PublicIncludePaths.AddRange(
          new string[] {
                "RootMotionExtractor/Public"
          });

        PrivateIncludePaths.AddRange(
            new string[] {
                "RootMotionExtractor/Private",
				// ... add other private include paths required here ...
			});
#endif

        PublicDependencyModuleNames.AddRange(
         new string[]
         {
                "Core",
          });

        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Projects",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "PropertyEditor",
#if UE_4_20_OR_LATER
                  "AnimationModifiers",
#endif
                "Persona"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
