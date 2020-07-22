//
//  RenderDocFunctionLibrary.h
//
//  Created by junjiehuang on 2020/6/30.
//

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RenderDocFunctionLibrary.generated.h"

UCLASS()
class URenderDocFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** interface for capture renderdoc frame */
	UFUNCTION(BlueprintCallable, Category = "RenderDocCapture")
		static void TriggerRenderDocCapture(FString captureDescription);

	/** DrawCall count of current frame */
	UFUNCTION(BlueprintCallable, Category = "RenderDocCapture")
		static int32 GetNumDrawCalls();

	/** DrawCall count of current frame */
	UFUNCTION(BlueprintCallable, Category = "RenderDocCapture")
		static int32 GetNumPrimitivesDrawn();

	UFUNCTION(BlueprintCallable, Category = "RenderDocCapture")
		static int32 GetFPS();

	UFUNCTION(BlueprintCallable, Category = "RenderDocCapture")
		static int32 GetFrameIntervalMillisecond();

	UFUNCTION(BlueprintCallable, Category = "RenderDocCapture")
		static int32 GetGameThreadTime();

	UFUNCTION(BlueprintCallable, Category = "RenderDocCapture")
		static int32 GetRenderThreadTime();

	UFUNCTION(BlueprintCallable, Category = "RenderDocCapture")
		static int32 GetRHIThreadTime();

	UFUNCTION(BlueprintCallable, Category = "RenderDocCapture")
		static int32 GetGPUTime();
};