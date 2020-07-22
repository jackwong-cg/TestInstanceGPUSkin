//
//  RenderDocCaptureModule.h
//
//  Created by junjiehuang on 2020/6/30.
//

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "../ThirdParty/RenderDoc_1_8/api/app/renderdoc_app.h"


class FRenderDocCaptureModule : public IModuleInterface
{
public:
	static FRenderDocCaptureModule* GetModule();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** capture one frame and save to file*/
	void CaptureFrame(FString captureDescription = TEXT(""));
	FString GetCaptureFolderPath();

public:
	typedef RENDERDOC_API_1_0_0 RENDERDOC_API_CONTEXT;

private:

	/** Handle to the dynamic lib we will load */
	void* RenderDocDLL;

	/** Handle to the render doc api interfaces */
	RENDERDOC_API_CONTEXT* RenderDocAPI;	
};
