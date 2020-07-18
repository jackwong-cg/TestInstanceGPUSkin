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
#include "TableRowGpuSkinAnimData.h"
#include "InstanceData.h"
#include "InstanceSkeletelMeshActor.generated.h"


USTRUCT(BlueprintType)
struct FUBakeVertexAnimationInfo
{
	GENERATED_USTRUCT_BODY()

	// �������ݱ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
	UDataTable* AnimDataTableBakeVertex;

	// ����Ķ����б���Ϣ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BakeVertexAnimationInfo)
	TArray<FTableRowAnimData> AnimSeqDatas;

};

USTRUCT(BlueprintType)
struct FUGpuSkinAnimationInfo
{
	GENERATED_USTRUCT_BODY()

	// �������ݱ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
	UDataTable* AnimDataTableGpuSkin;

	// ����Ķ����б���Ϣ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GpuSkinAnimationInfo)
	TArray<FTableRowGpuSkinAnimData> AnimSeqDatas;

	// ����������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GpuSkinAnimationInfo)
	UTexture2D* TexVertBlendInfo;
};

UENUM(BlueprintType)
enum class EAnimType : uint8
{
	// ���������ݸ�������ֵ�������ӻ��߼���
	EBakeVertexAnimation,	// UMETA(DisplayName = "BakeVertexAnimation"),
	// �Ѽ���������Ϊָ����ֵ
	EGpuSkinAnimation,		// UMETA(DisplayName = "GpuSkinAnimation"),
};


UCLASS()
class AInstanceSkeletelMeshActor : public AActor
{
	GENERATED_BODY()

public:
	AInstanceSkeletelMeshActor();

public:
	UPROPERTY(VisibleDefaultsOnly, Category = InstanceSkeletelMeshActor)
		USceneComponent* SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, Category = InstanceSkeletelMeshActor)
		UInstancedStaticMeshComponent* instanceStaticMesh;

	// ʵ�����ɵķ�Χ
	UPROPERTY(VisibleDefaultsOnly, Category = InstanceSkeletelMeshActor)
		UBoxComponent* boxRange;

	// ����Ķ��㶯������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		FUBakeVertexAnimationInfo AnimInfoBakeVertex;

	// GPU��Ƥ��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		FUGpuSkinAnimationInfo AnimInfoGpuSkin;

	// ��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		EAnimType AnimType = EAnimType::EBakeVertexAnimation;

	// ������ʵ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		int GenerateCount;

	// �ӵڼ�������ͼ��ʼ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		int RandomAnimImgIndexStart = -1;

	// >= 0,��ʾ����ʵ������ָ��ID�Ķ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		int MakeAllInstPlaySpecAnimID = -1;

	// �����Ķ�̬����ʵ��
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		TArray<UMaterialInstanceDynamic*> DynamicMaterialInstances;

	// ��һ������洢ʵ������
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		UTexture2D* TexInstanceData;
	// ������������0
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		UTexture2D* TexAnimData0;
	// ������������1
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		UTexture2D* TexAnimData1;
	// ������������2
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		UTexture2D* TexAnimData2;

	UPROPERTY()
		TArray<FUInstanceData> InstLst;

protected:
	TArray< TArray<FFloat16Color> > ParamDoubleBuffer;
	int		CurrentBufferIdx;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = InstanceSkeletelMeshActor)
		FTransform GetInstTransform(int id);

	UFUNCTION(BlueprintCallable, Category = InstanceSkeletelMeshActor)
		FUInstanceData GetInst(int id);


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};