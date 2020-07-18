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
	// ʵ��������λ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		FVector WorldLocation;
	// ʵ�������糯��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		FVector WorldRotation;
	// ʵ��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		FVector WorldScale;

	// ����ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		int AnimSeq = 0;
	// ��ǰ���������ۻ���ʱ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		float AnimTimeCounter = 0;
	// ������������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		float AnimPlayRate = 1;

	// �������õ���������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		int TexIndex = 0;

	// ��ǰ���������ڶ�������ĵڼ��п�ʼ��ȡ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FUInstanceData)
		int CurAnimRowStart = 0;

	UPROPERTY(EditAnywhere, Category = FUInstanceData)
		class AInstanceSkeletelMeshActor* ParentActor;
};