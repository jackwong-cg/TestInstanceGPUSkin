#pragma once


#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Classes/Engine/Texture2D.h"
#include "Engine/Classes/Engine/DataTable.h"
#include "TableRowAnimData.generated.h"


/**
* 顶点动画序列配置信息
*/
USTRUCT(BlueprintType)
struct FTableRowAnimData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// 动画数据存储在哪个纹理
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		TAssetPtr<UTexture2D> TargetTex = nullptr;

	// 动画纹理在数组中的索引值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int TexIndex = 0;

	// 动画纹理的路径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		FString AnimTexPath;

	// 动画名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		FString AnimName;

	// 动画起始帧在纹理的第几行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int StartFrameRow;

	// 动画结束帧在纹理的第几行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int EndFrameRow;

	// 动画总共有多少帧
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int FrameCount;

	// 一个动画帧占用几行像素数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int PixelRowsPerFrame;

	// 动画持续的时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		float AnimTimeLength;
};
