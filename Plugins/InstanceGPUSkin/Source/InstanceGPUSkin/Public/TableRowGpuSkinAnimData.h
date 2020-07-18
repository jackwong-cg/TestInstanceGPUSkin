#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Classes/Engine/Texture2D.h"
#if !PLATFORM_WINDOWS
#include "Engine/Classes/Engine/DataTable.h"
#endif
#include "TableRowGpuSkinAnimData.generated.h"


/**
* GpuSkin��������������Ϣ
*/
USTRUCT(BlueprintType)
struct FTableRowGpuSkinAnimData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// �������ݴ洢���ĸ�����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		TAssetPtr<UTexture2D> TargetTex = nullptr;

	// ���������������е�����ֵ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int TexIndex = 0;

	// ���������·��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		FString AnimTexPath;

	// ��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		FString AnimName;

	// ������ʼ֡������ĵڼ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int StartFrameRow;

	// ��������֡������ĵڼ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int EndFrameRow;

	// �����ܹ��ж���֡
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int FrameCount;

	// һ������֡ռ�ü�����������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		int PixelRowsPerFrame;

	// ����������ʱ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TableRowGpuSkinAnimData")
		float AnimTimeLength;
};
