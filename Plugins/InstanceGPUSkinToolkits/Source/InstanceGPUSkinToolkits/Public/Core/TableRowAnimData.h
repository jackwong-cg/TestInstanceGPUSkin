#pragma once


#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Classes/Engine/Texture2D.h"
#include "TableRowAnimData.generated.h"


/**
* 动画序列配置信息
*/
USTRUCT(BlueprintType)
struct FTableRowAnimData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// 动画数据存储在哪个纹理
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		TAssetPtr<UTexture2D> TargetTex = nullptr;

	// 动画纹理在数组中的索引值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int TexIndex = 0;

	// 动画纹理的路径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		FString AnimTexPath;

	// 动画名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		FString AnimName;

	// 动画起始帧在纹理的第几行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int StartFrameRow;

	// 动画结束帧在纹理的第几行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int EndFrameRow;

	// 动画总共有多少帧
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int FrameCount;

	// 一个动画帧占用几行像素数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int PixelRowsPerFrame;

	// 动画持续的时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		float AnimTimeLength;
};
