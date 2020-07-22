#include "RenderDocFunctionLibrary.h"
#include "RenderDocCapture.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Runtime/RHI/Public/RHI.h"
#include "Runtime/RenderCore/Public/RenderCore.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

#if PLATFORM_ANDROID
#include "Runtime/Core/Public/Android/AndroidPlatformTime.h"
#elif PLATFORM_IOS
#include "Runtime/Core/Public/IOS/IOSPlatformTime.h"
#elif PLATFORM_LINUX
#include "Runtime/Core/Public/Linux/LinuxPlatformTime.h"
#elif PLATFORM_WINDOWS
#include "Runtime/Core/Public/Windows/WindowsPlatformTime.h"
#endif

void URenderDocFunctionLibrary::TriggerRenderDocCapture(FString captureDescription)
{
	FRenderDocCaptureModule* pMod = FRenderDocCaptureModule::GetModule();
	if (pMod != nullptr)
	{
		pMod->CaptureFrame(captureDescription);
	}
}

int32 URenderDocFunctionLibrary::GetNumDrawCalls()
{
	return GNumDrawCallsRHI;
}

int32 URenderDocFunctionLibrary::GetNumPrimitivesDrawn()
{
	return GNumPrimitivesDrawnRHI;
}

int32 URenderDocFunctionLibrary::GetFPS()
{
	int32 MaxThreadTime = UKismetMathLibrary::Max(FMath::Max3(
		FPlatformTime::ToMilliseconds(GGameThreadTime),
		FPlatformTime::ToMilliseconds(GRenderThreadTime),
		FPlatformTime::ToMilliseconds(GRHIThreadTime)),
		FPlatformTime::ToMilliseconds(GSwapBufferTime));
	return (int32)(1000.0f / (float)(MaxThreadTime));
}

int32 URenderDocFunctionLibrary::GetFrameIntervalMillisecond()
{
	int32 MaxThreadTime = UKismetMathLibrary::Max(FMath::Max3(
		FPlatformTime::ToMilliseconds(GGameThreadTime),
		FPlatformTime::ToMilliseconds(GRenderThreadTime),
		FPlatformTime::ToMilliseconds(GRHIThreadTime)),
		FPlatformTime::ToMilliseconds(GSwapBufferTime));
	return MaxThreadTime;
}

int32 URenderDocFunctionLibrary::GetGameThreadTime()
{
	return FPlatformTime::ToMilliseconds(GGameThreadTime);
}

int32 URenderDocFunctionLibrary::GetRenderThreadTime()
{
	return FPlatformTime::ToMilliseconds(GRenderThreadTime);
}

int32 URenderDocFunctionLibrary::GetRHIThreadTime()
{
	return FPlatformTime::ToMilliseconds(GRHIThreadTime);
}

int32 URenderDocFunctionLibrary::GetGPUTime()
{
	return FPlatformTime::ToMilliseconds(GSwapBufferTime);
}