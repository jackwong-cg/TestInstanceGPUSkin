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
	// 实例的世界位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		FVector WorldLocation;
	// 实例的世界朝向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		FVector WorldRotation;
	// 实例的缩放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		FVector WorldScale;

	// 动画ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		int AnimSeq = 0;
	// 当前动画播放累积的时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		float AnimTimeCounter = 0;
	// 动画播放速率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		float AnimPlayRate = 1;

	// 动画引用的纹理索引
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		int TexIndex = 0;

	// 当前动画数据在动画纹理的第几行开始读取
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

	// 实例生成的范围
	UPROPERTY(VisibleDefaultsOnly, Category = InstanceGPUSkinActor)
		UBoxComponent* boxRange;
	
	// 动画数据表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		UDataTable* AnimDataTable;

	// 创建的实例数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		int GenerateCount;

	// 缓存的动画列表信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		TArray<FTableRowAnimData> AnimSeqDatas;

	// 从第几个纹理图象开始
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		int RandomAnimImgIndexStart = -1;

	// 创建的动态材质实例
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		TArray<UMaterialInstanceDynamic*> DynamicMaterialInstances;
	   
	// 用一张纹理存储实例数据
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		UTexture2D* TexInstanceData;
	// 动画数据纹理0
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		UTexture2D* TexAnimData0;
	// 动画数据纹理1
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = InstanceGPUSkinActor)
		UTexture2D* TexAnimData1;
	// 动画数据纹理2
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