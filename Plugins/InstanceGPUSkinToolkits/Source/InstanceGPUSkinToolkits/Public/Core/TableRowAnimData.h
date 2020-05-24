#pragma once


#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Classes/Engine/Texture2D.h"
#include "TableRowAnimData.generated.h"


/**
* ��������������Ϣ
*/
USTRUCT(BlueprintType)
struct FTableRowAnimData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// �������ݴ洢���ĸ�����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		TAssetPtr<UTexture2D> TargetTex = nullptr;

	// ���������������е�����ֵ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int TexIndex = 0;

	// ���������·��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		FString AnimTexPath;

	// ��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		FString AnimName;

	// ������ʼ֡������ĵڼ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int StartFrameRow;

	// ��������֡������ĵڼ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int EndFrameRow;

	// �����ܹ��ж���֡
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int FrameCount;

	// һ������֡ռ�ü�����������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		int PixelRowsPerFrame;

	// ����������ʱ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FAnimInfo")
		float AnimTimeLength;
};
