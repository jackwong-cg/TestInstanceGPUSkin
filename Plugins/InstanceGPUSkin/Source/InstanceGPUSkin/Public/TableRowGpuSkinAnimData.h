#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Classes/Engine/Texture2D.h"
#if !PLATFORM_WINDOWS
#include "Engine/Classes/Engine/DataTable.h"
#endif
#include "TableRowGpuSkinAnimData.generated.h"


/**
* GpuSkin动画序列配置信息
*/
USTRUCT(BlueprintType)
struct FTableRowGpuSkinAnimData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// 动画数据存储在哪个纹理
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		TAssetPtr<UTexture2D> TargetTex = nullptr;

	// 动画纹理在数组中的索引值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int TexIndex = 0;

	// 动画纹理的路径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		FString AnimTexPath;

	// 动画名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		FString AnimName;

	// 动画起始帧在纹理的第几行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int StartFrameRow;

	// 动画结束帧在纹理的第几行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int EndFrameRow;

	// 动画总共有多少帧
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int FrameCount;

	// 一个动画帧占用几行像素数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int PixelRowsPerFrame;

	// 动画持续的时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		float AnimTimeLength;
};
