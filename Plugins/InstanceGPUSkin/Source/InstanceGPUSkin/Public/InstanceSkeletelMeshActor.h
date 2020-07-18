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

	// 动画数据表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
	UDataTable* AnimDataTableBakeVertex;

	// 缓存的动画列表信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BakeVertexAnimationInfo)
	TArray<FTableRowAnimData> AnimSeqDatas;

};

USTRUCT(BlueprintType)
struct FUGpuSkinAnimationInfo
{
	GENERATED_USTRUCT_BODY()

	// 动画数据表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
	UDataTable* AnimDataTableGpuSkin;

	// 缓存的动画列表信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GpuSkinAnimationInfo)
	TArray<FTableRowGpuSkinAnimData> AnimSeqDatas;

	// 顶点混合纹理
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GpuSkinAnimationInfo)
	UTexture2D* TexVertBlendInfo;
};

UENUM(BlueprintType)
enum class EAnimType : uint8
{
	// 计数器根据给定的数值进行增加或者减少
	EBakeVertexAnimation,	// UMETA(DisplayName = "BakeVertexAnimation"),
	// 把计数器设置为指定数值
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

	// 实例生成的范围
	UPROPERTY(VisibleDefaultsOnly, Category = InstanceSkeletelMeshActor)
		UBoxComponent* boxRange;

	// 烘培的顶点动画数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		FUBakeVertexAnimationInfo AnimInfoBakeVertex;

	// GPU蒙皮动画数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		FUGpuSkinAnimationInfo AnimInfoGpuSkin;

	// 动画类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		EAnimType AnimType = EAnimType::EBakeVertexAnimation;

	// 创建的实例数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		int GenerateCount;

	// 从第几个纹理图象开始
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		int RandomAnimImgIndexStart = -1;

	// >= 0,表示所有实例播放指定ID的动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		int MakeAllInstPlaySpecAnimID = -1;

	// 创建的动态材质实例
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		TArray<UMaterialInstanceDynamic*> DynamicMaterialInstances;

	// 用一张纹理存储实例数据
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		UTexture2D* TexInstanceData;
	// 动画数据纹理0
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		UTexture2D* TexAnimData0;
	// 动画数据纹理1
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceSkeletelMeshActor)
		UTexture2D* TexAnimData1;
	// 动画数据纹理2
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