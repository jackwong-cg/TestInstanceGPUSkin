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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		FVector WorldLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		FVector WorldRotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		FVector WorldScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		int AnimSeq = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		float AnimTimeCounter = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		float AnimPlayRate = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CrowsInstData)
		int TexIndex = 0;

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
	UPROPERTY(VisibleDefaultsOnly, Category = InstanceInsects)
		USceneComponent* SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, Category = InstanceInsects)
		UInstancedStaticMeshComponent* instanceStaticMesh;

	UPROPERTY(VisibleDefaultsOnly, Category = InstanceInsects)
		UBoxComponent* boxRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceInsects)
		UDataTable* AnimDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceInsects)
		int GenerateCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceInsects)
		TArray<FTableRowAnimData> AnimSeqDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceInsects)
		UMaterial* SrcMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceInsects)
		UMaterialInstanceDynamic * DynamicMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceInsects)
		UTexture2D* TexInstanceData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceInsects)
		UTexture2D* TexAnimData0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceInsects)
		UTexture2D* TexAnimData1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InstanceInsects)
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


#if WITH_EDITOR
	//ENGINE_API virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//ENGINE_API virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

};