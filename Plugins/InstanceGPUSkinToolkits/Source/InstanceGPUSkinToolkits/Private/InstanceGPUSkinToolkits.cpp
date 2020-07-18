// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#if WITH_EDITOR


#include "InstanceGPUSkinToolkits.h"
#include "InstanceGPUSkinToolkitsStyle.h"
#include "InstanceGPUSkinToolkitsCommands.h"
#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "LevelEditor.h"

#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SExportAnimWidget.h"


static const FName InstanceGPUSkinToolkitsTabName("InstanceGPUSkinToolkits");

#define LOCTEXT_NAMESPACE "FInstanceGPUSkinToolkitsModule"

void FInstanceGPUSkinToolkitsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FInstanceGPUSkinToolkitsStyle::Initialize();
	FInstanceGPUSkinToolkitsStyle::ReloadTextures();

	FInstanceGPUSkinToolkitsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FInstanceGPUSkinToolkitsCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FInstanceGPUSkinToolkitsModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FInstanceGPUSkinToolkitsModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FInstanceGPUSkinToolkitsModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}


	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(InstanceGPUSkinToolkitsTabName, FOnSpawnTab::CreateRaw(this, &FInstanceGPUSkinToolkitsModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FInstanceSkinMeshToolTabTitle", "GPUAnimExpoter"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FInstanceGPUSkinToolkitsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FInstanceGPUSkinToolkitsStyle::Shutdown();

	FInstanceGPUSkinToolkitsCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(InstanceGPUSkinToolkitsTabName);
}

void FInstanceGPUSkinToolkitsModule::PluginButtonClicked()
{
	//// Put your "OnButtonClicked" stuff here
	//FText DialogText = FText::Format(
	//						LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
	//						FText::FromString(TEXT("FInstanceGPUSkinToolkitsModule::PluginButtonClicked()")),
	//						FText::FromString(TEXT("InstanceGPUSkinToolkits.cpp"))
	//				   );
	//FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	FGlobalTabmanager::Get()->InvokeTab(InstanceGPUSkinToolkitsTabName);
}

void FInstanceGPUSkinToolkitsModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FInstanceGPUSkinToolkitsCommands::Get().PluginAction);
}

void FInstanceGPUSkinToolkitsModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FInstanceGPUSkinToolkitsCommands::Get().PluginAction);
}


TSharedRef<SDockTab> FInstanceGPUSkinToolkitsModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FInstanceSkinMeshToolModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("InstanceGPUSkinToolkits.cpp"))
	);

	TSharedRef<SDockTab> tab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SExportAnimWidget)
		];
	tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FInstanceGPUSkinToolkitsModule::OnTabClose));

	return tab;
}

void FInstanceGPUSkinToolkitsModule::OnTabClose(TSharedRef<class SDockTab> tab)
{
	SExportAnimWidget::DestroyInstance();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInstanceGPUSkinToolkitsModule, InstanceGPUSkinToolkits)



#endif