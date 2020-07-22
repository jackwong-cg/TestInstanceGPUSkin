// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RenderDocCapture : ModuleRules
{
	public RenderDocCapture(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects",
                "CoreUObject",
                "Engine",
                "RHI",
                "RenderCore",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);


        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
            string pluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            string aplPath = pluginPath + "/RenderDocCapture_APL.xml";
            //Receipt.AdditionalProperties.Add(new ReceiptProperty("AndroidPlugin", aplPath));
            AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", aplPath));
            System.Console.WriteLine("RenderDocCapture -------------- PluginPath = " + pluginPath);
            System.Console.WriteLine("RenderDocCapture -------------- aplPath = " + aplPath);
        }
        else if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string pluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            RuntimeDependencies.Add(pluginPath + "/../../Binaries/ThirdParty/RenderDoc_1_8/Win64/renderdoc.dll");
            System.Console.WriteLine("RenderDocCapture -------------- ModuleDirectory = " + ModuleDirectory);
            System.Console.WriteLine("RenderDocCapture -------------- pluginPath = " + pluginPath);
        }
    }
}
