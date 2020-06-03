#pragma once

#include "Engine.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Components/InstancedStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Engine/Classes/Engine/DataTable.h"
#include "TableRowAnimData.h"
#include "InstanceGPUSkinActor.generated.h"


USTRUCT(BlueprintType)
struct FUCrowsInstData
{
	GENERATED_USTRUCT_BODY()

public:
	void Tick(float DeltaTime);

public:
	// ʵ��������λ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		FVector WorldLocation;
	// ʵ�������糯��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		FVector WorldRotation;
	// ʵ��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		FVector WorldScale;

	// ����ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		int AnimSeq = 0;
	// ��ǰ���������ۻ���ʱ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		float AnimTimeCounter = 0;
	// ������������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		float AnimPlayRate = 1;

	// �������õ���������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		int TexIndex = 0;

	// ��ǰ���������ڶ�������ĵڼ��п�ʼ��ȡ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		int CurAnimRowStart = 0;

	UPROPERTY()
		AInstanceGPUSkinActor* ParentActor;
};


UCLASS()
class AInstanceGPUSkinActor : public AActor
{
	GENERATED_BODY()

public:
	AInstanceGPUSkinActor();


public:
	UPROPERTY(VisibleDefaultsOnly, Category = InstanceGPUSkinActor)
		USceneComponent* SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, Category = InstanceGPUSkinActor)
		UInstancedStaticMeshComponent* instanceStaticMesh;

	// ʵ�����ɵķ�Χ
	UPROPERTY(VisibleDefaultsOnly, Category = InstanceGPUSkinActor)
		UBoxComponent* boxRange;
	
	// �������ݱ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		UDataTable* AnimDataTable;

	// ������ʵ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		int GenerateCount;

	// ����Ķ����б���Ϣ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		TArray<FTableRowAnimData> AnimSeqDatas;

	// �ӵڼ�������ͼ��ʼ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		int RandomAnimImgIndexStart = -1;

	// �����Ķ�̬����ʵ��
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		TArray<UMaterialInstanceDynamic*> DynamicMaterialInstances;
	   
	// ��һ������洢ʵ������
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		UTexture2D* TexInstanceData;
	// ������������0
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		UTexture2D* TexAnimData0;
	// ������������1
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		UTexture2D* TexAnimData1;
	// ������������2
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		UTexture2D* TexAnimData2;

	UPROPERTY()
		TArray<FUCrowsInstData> InstLst;

protected:
	TArray< TArray<FFloat16Color> > ParamDoubleBuffer;
	int		CurrentBufferIdx;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = InstanceGPUSkinActor)
		FTransform GetInstTransform(int id);

	UFUNCTION(BlueprintCallable, Category = InstanceGPUSkinActor)
		FUCrowsInstData GetInst(int id);


#if WITH_EDITOR
	//ENGINE_API virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//ENGINE_API virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

};