// Copyright Epic Games, Inc. All Rights Reserved.

#include "RenderDocCapture.h"
#include "Runtime/Launch/Resources/Version.h"
#include "LogMacros.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#include "Internationalization/Internationalization.h"
#include "RendererInterface.h"
#include "RenderingThread.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif
#include "Async/Async.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/GameViewportClient.h"
#include "Misc/AutomationTest.h"

#if WITH_EDITOR
#include "UnrealClient.h"
#include "Editor/EditorEngine.h"
extern UNREALED_API UEditorEngine* GEditor;
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Editor.h"
#endif

#include "Runtime/Engine/Public/UnrealClient.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"

#if PLATFORM_ANDROID
#include "Runtime/Launch/Public/Android/AndroidJNI.h"
#include "Runtime/ApplicationCore/Public/Android/AndroidWindow.h"
#endif

#define LOCTEXT_NAMESPACE "FRenderDocCaptureModule"


FRenderDocCaptureModule* FRenderDocCaptureModule::GetModule()
{
	IModuleInterface* pModule = FModuleManager::Get().GetModule("RenderDocCapture");
	if (pModule == nullptr)
		return nullptr;
	return (FRenderDocCaptureModule*)(pModule);
}

void FRenderDocCaptureModule::StartupModule()
{
	RenderDocDLL = nullptr;
	RenderDocAPI = nullptr;

	//// check if enable renderdoc capturing
	//bool cfgEnableRenderDocCapture = false;
	//FString pluginConfigString = FPaths::ProjectConfigDir() + TEXT("Plugin.ini");
	//GConfig->GetBool(TEXT("Plugin"), TEXT("EnableRenderDocCapture"), cfgEnableRenderDocCapture, pluginConfigString);
	//if (cfgEnableRenderDocCapture == false)
	//{
	//	return;
	//}
	
	FString LibraryPath = TEXT("");

#if PLATFORM_ANDROID
	LibraryPath = TEXT("libVkLayer_GLES_RenderDoc.so");

#elif PLATFORM_WINDOWS

#if WITH_EDITOR
	//FString BaseDir = IPluginManager::Get().FindPlugin("RenderDocCapture")->GetBaseDir();
	//LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/RenderDoc_1_8/Win64/renderdoc.dll"));
#else
	FString BaseDir = IPluginManager::Get().FindPlugin("RenderDocCapture")->GetBaseDir();
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/RenderDoc_1_8/Win64/renderdoc.dll"));
#endif

#else
	UE_LOG(LogTemp, Warning, TEXT("RENDERDOC - not integrate renderdoc to current platform!"));
	return;

#endif	

	// try to load renderdoc dynamic lib
	if (RenderDocDLL == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("RENDERDOC - Try load lib at : %s"), *LibraryPath);
		RenderDocDLL = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

		if (RenderDocDLL)
		{
			UE_LOG(LogTemp, Log, TEXT("RENDERDOC - Load %s success!"), *LibraryPath);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("RENDERDOC - Load %s failed!"), *LibraryPath);
		}
	}

	// try to get renderdoc api interface
	if (RenderDocDLL)
	{
		// Get interface
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)FPlatformProcess::GetDllExport(RenderDocDLL, TEXT("RENDERDOC_GetAPI"));
		if (RENDERDOC_GetAPI == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("RENDERDOC - unable to obtain 'RENDERDOC_GetAPI' function from '%s'. You are likely using an incompatible version of RenderDoc."), *LibraryPath);
			FPlatformProcess::FreeDllHandle(RenderDocDLL);
			RenderDocDLL = nullptr;
			return;
		}

		// Version checking and reporting
		if (0 == RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_0_0, (void**)&RenderDocAPI))
		{
			UE_LOG(LogTemp, Warning, TEXT("RENDERDOC - unable to initialize RenderDoc library due to API incompatibility (plugin requires eRENDERDOC_API_Version_1_4_1)."), *LibraryPath);
			FPlatformProcess::FreeDllHandle(RenderDocDLL);
			RenderDocDLL = nullptr;
			return;
		}
	}

	// initialize render doc
	if(RenderDocAPI)
	{
		int MajorVersion(0), MinorVersion(0), PatchVersion(0);
		RenderDocAPI->GetAPIVersion(&MajorVersion, &MinorVersion, &PatchVersion);
		UE_LOG(LogTemp, Log, TEXT("RENDERDOC - RenderDoc library has been loaded (Vision - v%i.%i.%i)."), MajorVersion, MinorVersion, PatchVersion);

		RenderDocAPI->UnloadCrashHandler();

		// make sure the capture folder has been created		
		FString RenderDocCapturePath = GetCaptureFolderPath();
		if (!IFileManager::Get().DirectoryExists(*RenderDocCapturePath))
		{
			IFileManager::Get().MakeDirectory(*RenderDocCapturePath, true);
		}
		FString CapturePath = FPaths::Combine(*RenderDocCapturePath, *FDateTime::Now().ToString());
		CapturePath = FPaths::ConvertRelativePathToFull(CapturePath);
		FPaths::NormalizeDirectoryName(CapturePath);
		RenderDocAPI->SetLogFilePathTemplate(TCHAR_TO_ANSI(*CapturePath));

		// Setup RenderDoc settings
		RenderDocAPI->SetFocusToggleKeys(nullptr, 0);
		RenderDocAPI->SetCaptureKeys(nullptr, 0);
		RenderDocAPI->SetCaptureOptionU32(eRENDERDOC_Option_CaptureCallstacks, 1);
		RenderDocAPI->SetCaptureOptionU32(eRENDERDOC_Option_RefAllResources, 1);
		RenderDocAPI->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1);
		RenderDocAPI->MaskOverlayBits(eRENDERDOC_Overlay_Default, eRENDERDOC_Overlay_Default);

		// register console command for renderdoc capture frame 
		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("rdc"),
			TEXT("Trigger RenderDoc Capture."),
			FConsoleCommandDelegate::CreateLambda([]()
			{
				FRenderDocCaptureModule::GetModule()->CaptureFrame();
			}),
			ECVF_Default
		);
	}
}

void FRenderDocCaptureModule::ShutdownModule()
{
	if (RenderDocAPI)
	{
		RenderDocAPI->Shutdown();
		RenderDocAPI = nullptr;
	}

	if (RenderDocDLL != nullptr)
	{
		FPlatformProcess::FreeDllHandle(RenderDocDLL);
		RenderDocDLL = nullptr;
	}
}

void FRenderDocCaptureModule::CaptureFrame(FString captureFileName)
{
	if (RenderDocAPI != nullptr)
	{
		int capCount = RenderDocAPI->GetNumCaptures();
		FText ftex = FText::Format(LOCTEXT("CapFrameName", "{0}_{1}_{2}"), FText::AsNumber(capCount), FText::FromString(FDateTime::Now().ToString()), FText::FromString(captureFileName));
		FString realFileName = ftex.ToString();
		FString CapturePath = FPaths::Combine(*GetCaptureFolderPath(), *realFileName);
		CapturePath = FPaths::ConvertRelativePathToFull(CapturePath);
		FPaths::NormalizeDirectoryName(CapturePath);
		RenderDocAPI->SetLogFilePathTemplate(TCHAR_TO_ANSI(*CapturePath));
		UE_LOG(LogTemp, Log, TEXT("RENDERDOC - try to capture and save to file : %s"), *CapturePath);

		if (RenderDocAPI->IsFrameCapturing() == false)
		{
			UE_LOG(LogTemp, Log, TEXT("RENDERDOC - RenderDocAPI->TriggerCapture()"));
			RenderDocAPI->TriggerCapture();
		}
	}
}

FString FRenderDocCaptureModule::GetCaptureFolderPath()
{
	FString RenderDocCapturePath = FPaths::ProjectSavedDir() / TEXT("RenderDocCaptures");

#if PLATFORM_ANDROID
	RenderDocCapturePath = AndroidWrapper::SExternalPath / TEXT("RenderDocCaptures");
#endif

	return RenderDocCapturePath;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRenderDocCaptureModule, RenderDocCapture)
