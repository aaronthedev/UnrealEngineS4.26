// Copyright 2017 Yaki Studios, Inc. All Rights Reserved.

#include "RootMotionExtractor.h"
#include "RootMotionExtractorStyle.h"
#include "RootMotionExtractorCommands.h"
#include "LevelEditor.h"
#include "Editor.h"
#include "Engine.h"


#define LOCTEXT_NAMESPACE "FRootMotionExtractorModule"

/// <summary>
/// Startup the plugin and proceed with initialization and attaching.
/// </summary>
void FRootMotionExtractorModule::StartupModule()
{
	TickDelegate = FTickerDelegate::CreateRaw(this, &FRootMotionExtractorModule::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
	FRootMotionExtractorModule::Initialize(); //initialize plugin
	FRootMotionExtractorModule::StickPlugin(); // stick plugin to editor toolboar and menu
}

/// <summary>
/// Initialize and generate plugin window
/// </summary>
void FRootMotionExtractorModule::Initialize() {
	FRootMotionExtractorStyle::Initialize();
	FRootMotionExtractorStyle::ReloadTextures();
	FRootMotionExtractorCommands::Register();
	TSharedPtr <FCaptureType> Bulk = MakeShareable(new FCaptureType);
	TSharedPtr <FCaptureType> Single = MakeShareable(new FCaptureType);
	Bulk.Get()->CaptureType = ECaptureType::CT_Bulk;
	Bulk.Get()->CaptureLabel = FString("Capture Bulk");
	Single.Get()->CaptureType = ECaptureType::CT_Single;
	Single.Get()->CaptureLabel = FString("Capture Single");
	CaptureTypes.Add(Bulk);
	CaptureTypes.Add(Single);
}

/// <summary>
/// /Attach Plugin to toolbar and menu/
/// </summary>
void FRootMotionExtractorModule::StickPlugin() {
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FRootMotionExtractorCommands::Get().OpenPluginWindow, FExecuteAction::CreateRaw(this, &FRootMotionExtractorModule::PluginButtonClicked), FCanExecuteAction());
	    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FRootMotionExtractorModule::AddMenuExtension));
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FRootMotionExtractorModule::AddToolbarExtension));
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}
/// <summary>
/// Plugin is shutdown. Execute some cleanup.
/// </summary>
void FRootMotionExtractorModule::ShutdownModule()
{
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	FRootMotionExtractorStyle::Shutdown();
	FRootMotionExtractorCommands::Unregister();
	FRootMotionExtractorGUI::WindowContainer.Reset();
	FRootMotionExtractorGUI::AddBoneWindowContainer.Reset();
}
/// <summary>
/// Add plugin to menu
/// </summary>
void FRootMotionExtractorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FRootMotionExtractorCommands::Get().OpenPluginWindow);
}
/// <summary>
/// Add plugin to toolbar
/// </summary>
void FRootMotionExtractorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FRootMotionExtractorCommands::Get().OpenPluginWindow);
}

/// <summary>
/// Tick and check if Extraction is running and finished.
/// </summary>
/// <param name="DeltaTime">Delta Tick</param>
/// <returns>Tick</returns>
bool FRootMotionExtractorModule::Tick(float DeltaTime)
{
	if (RMEEngine::ExtractJobs.IsEmpty() && RMEEngine::bOperationStarted && !RMEEngine::IsRunning()) {
		RMEEngine::bOperationStarted = false;
		RMEEngine::RunningTasks.Empty();
		FRootMotionExtractorModule::DisplayMessage(FString::Printf(TEXT("Extraction succesfull for %s file(s)"), *FString::FromInt(RMEEngine::CompletedFiles)));
		UE_LOG(RMELog, Warning, TEXT("Extraction succesfull for %s file(s)"), *FString::FromInt(RMEEngine::CompletedFiles));
	}

	if (RMEEngine::bOperationStarted) {
		for (int i = 0; i < RMEEngine::ExtractionsLaunchedPerFrame; i++)
		{
			RMEEngine::FRMEExtractor::TryStartJob();
		}
	}
	return true;
}

/// /// <summary>
/// Plugin button was clicked. Initialize Window and show it.
/// </summary>
void FRootMotionExtractorModule::PluginButtonClicked()
{
	FRootMotionExtractorGUI::ShowMainWindow();
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRootMotionExtractorModule, RootMotionExtractor)