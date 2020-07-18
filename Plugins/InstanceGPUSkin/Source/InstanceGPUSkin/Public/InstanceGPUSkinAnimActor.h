#pragma once

#include "Engine.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Components/InstancedStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Engine/Classes/Engine/DataTable.h"
#include "TableRowGpuSkinAnimData.h"
#include "InstanceGPUSkinAnimActor.generated.h"


USTRUCT(BlueprintType)
struct FUCrowsInstDataGpuSkin
{
	GENERATED_USTRUCT_BODY()

public:
	void Tick(float DeltaTime);

public:
	// ʵ��������λ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstDataGpuSkin)
		FVector WorldLocation;
	// ʵ�������糯��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstDataGpuSkin)
		FVector WorldRotation;
	// ʵ��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstDataGpuSkin)
		FVector WorldScale;

	// ����ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstDataGpuSkin)
		int AnimSeq = 0;
	// ��ǰ���������ۻ���ʱ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstDataGpuSkin)
		float AnimTimeCounter = 0;
	// ������������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstDataGpuSkin)
		float AnimPlayRate = 1;

	// �������õ���������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstDataGpuSkin)
		int TexIndex = 0;

	// ��ǰ���������ڶ�������ĵڼ��п�ʼ��ȡ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstDataGpuSkin)
		int CurAnimRowStart = 0;

	UPROPERTY()
		AInstanceGPUSkinAnimActor* ParentActor;
};


UCLASS()
class AInstanceGPUSkinAnimActor : public AActor
{
	GENERATED_BODY()

public:
	AInstanceGPUSkinAnimActor();


public:
	UPROPERTY(VisibleDefaultsOnly, Category = InstanceGPUSkinAnimActor)
		USceneComponent* SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, Category = InstanceGPUSkinAnimActor)
		UInstancedStaticMeshComponent* instanceStaticMesh;

	// ʵ�����ɵķ�Χ
	UPROPERTY(VisibleDefaultsOnly, Category = InstanceGPUSkinAnimActor)
		UBoxComponent* boxRange;

	// �������ݱ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		UDataTable* AnimDataTable;

	// ������ʵ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		int GenerateCount;

	// ����Ķ����б���Ϣ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		TArray<FTableRowGpuSkinAnimData> AnimSeqDatas;

	// �ӵڼ�������ͼ��ʼ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		UTexture2D* TexVertBlendInfo;

	// �ӵڼ�������ͼ��ʼ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		int RandomAnimImgIndexStart = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		int MakeAllInstPlayAnimID = -1;

	// �����Ķ�̬����ʵ��
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		TArray<UMaterialInstanceDynamic*> DynamicMaterialInstances;

	// ��һ������洢ʵ������
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		UTexture2D* TexInstanceData;
	// ������������0
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		UTexture2D* TexAnimData0;
	// ������������1
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		UTexture2D* TexAnimData1;
	// ������������2
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinAnimActor)
		UTexture2D* TexAnimData2;

	UPROPERTY()
		TArray<FUCrowsInstDataGpuSkin> InstLst;

protected:
	TArray< TArray<FFloat16Color> > ParamDoubleBuffer;
	int		CurrentBufferIdx;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = InstanceGPUSkinAnimActor)
		FTransform GetInstTransform(int id);

	UFUNCTION(BlueprintCallable, Category = InstanceGPUSkinAnimActor)
		FUCrowsInstDataGpuSkin GetInst(int id);


#if WITH_EDITOR
	//ENGINE_API virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//ENGINE_API virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

};