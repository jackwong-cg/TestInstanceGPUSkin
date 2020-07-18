#pragma once

#include "Engine.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Components/InstancedStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "InstanceData.generated.h"


USTRUCT(BlueprintType)
struct FUInstanceData
{
	GENERATED_USTRUCT_BODY()

public:
	void Tick(float DeltaTime);

public:
	// 实例的世界位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		FVector WorldLocation;
	// 实例的世界朝向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		FVector WorldRotation;
	// 实例的缩放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		FVector WorldScale;

	// 动画ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		int AnimSeq = 0;
	// 当前动画播放累积的时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		float AnimTimeCounter = 0;
	// 动画播放速率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		float AnimPlayRate = 1;

	// 动画引用的纹理索引
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		int TexIndex = 0;

	// 当前动画数据在动画纹理的第几行开始读取
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		int CurAnimRowStart = 0;

	UPROPERTY(EditAnywhere, Category = FUInstanceData)
		class AInstanceSkeletelMeshActor* ParentActor;
};