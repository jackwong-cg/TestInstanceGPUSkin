#pragma once


#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Classes/Engine/Texture2D.h"
#include "Engine/Classes/Engine/DataTable.h"
#include "TableRowAnimData.generated.h"


/**
* ���㶯������������Ϣ
*/
USTRUCT(BlueprintType)
struct FTableRowAnimData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// �������ݴ洢���ĸ�����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		TAssetPtr<UTexture2D> TargetTex = nullptr;

	// ���������������е�����ֵ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int TexIndex = 0;

	// ���������·��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		FString AnimTexPath;

	// ��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		FString AnimName;

	// ������ʼ֡������ĵڼ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int StartFrameRow;

	// ��������֡������ĵڼ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int EndFrameRow;

	// �����ܹ��ж���֡
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int FrameCount;

	// һ������֡ռ�ü�����������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		int PixelRowsPerFrame;

	// ����������ʱ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTableRowAnimData")
		float AnimTimeLength;
};
