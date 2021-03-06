#include "InstanceSkeletelMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Core/Public/Async/ParallelFor.h"

FTransform AInstanceSkeletelMeshActor::GetInstTransform(int id)
{
	FTransform ret = FTransform::Identity;
	if (id >= 0 && id < InstLst.Num())
	{
		ret.SetLocation(InstLst[id].WorldLocation);
		ret.SetRotation(FQuat::MakeFromEuler(InstLst[id].WorldRotation));
		ret.SetScale3D(InstLst[id].WorldScale);
	}
	return ret;
}

FUInstanceData AInstanceSkeletelMeshActor::GetInst(int id)
{
	if (id >= 0 && id < InstLst.Num())
	{
		return InstLst[id];
	}
	return FUInstanceData();
}

AInstanceSkeletelMeshActor::AInstanceSkeletelMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	instanceStaticMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstanceStaticMesh"));
	instanceStaticMesh->SetupAttachment(SceneRoot);

	boxRange = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxGenRange"));
	boxRange->SetupAttachment(SceneRoot);
	boxRange->SetCollisionProfileName("NoCollision");
}

void AInstanceSkeletelMeshActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("LOGDEBUG - AInstanceSkeletelMeshActor [%s] do BeginPlay()"), *this->GetName());

	// 把每个顶点的索引值编码写到对应的颜色的RG通道里
	if (instanceStaticMesh->GetStaticMesh())
	{
		FStaticMeshLODResources& LODModel = instanceStaticMesh->GetStaticMesh()->RenderData->LODResources[0];
		{
			int VertexCount = LODModel.GetNumVertices();
			int VertexColorCount = LODModel.VertexBuffers.ColorVertexBuffer.GetNumVertices();
			// Mesh doesn't have a color vertex buffer yet!  We'll create one now.
			LODModel.VertexBuffers.ColorVertexBuffer.InitFromSingleColor(FColor(255, 0, 0, 255), LODModel.GetNumVertices());
			for (int i = 0; i < VertexCount; i++)
			{
				LODModel.VertexBuffers.ColorVertexBuffer.VertexColor(i) = FColor(i / 255, i % 255, 0, 255);
			}
			// @todo MeshPaint: Make sure this is the best place to do this
			BeginInitResource(&LODModel.VertexBuffers.ColorVertexBuffer);
		}
		instanceStaticMesh->GetStaticMesh()->InitResources();
		instanceStaticMesh->MarkRenderStateDirty();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LOGDEBUG - Not set staticmesh for AInstanceGPUSkinActor [%s]"), *this->GetName());
	}

	// 创建动态材质实例
	int MatCount = instanceStaticMesh->GetNumMaterials();
	for (int i = 0; i < MatCount; ++i)
	{
		UMaterialInterface* srcMat = instanceStaticMesh->GetMaterial(i);
		UMaterialInstanceDynamic* DynamicMaterialInstance = instanceStaticMesh->CreateDynamicMaterialInstance(i, srcMat);
		instanceStaticMesh->SetMaterial(i, DynamicMaterialInstance);
		DynamicMaterialInstances.Add(DynamicMaterialInstance);

		UE_LOG(LogTemp, Log, TEXT("LOGDEBUG -Create UMaterialInstanceDynamic (%s)"), *srcMat->GetName());
	}

	// 创建实例数据纹理
	int DynamicTextureWidth = 32, DynamicTextureHeight = 32;
	int TotalCount = DynamicTextureWidth * DynamicTextureHeight;
	while (TotalCount < GenerateCount)
	{
		if (DynamicTextureWidth < 1024)
			DynamicTextureWidth *= 2;
		else if (DynamicTextureHeight < 1024)
			DynamicTextureHeight *= 2;
		else
		{
			DynamicTextureWidth *= 2;
			DynamicTextureHeight *= 2;
		}
		TotalCount = DynamicTextureWidth * DynamicTextureHeight;
	}
	TexInstanceData = UTexture2D::CreateTransient(DynamicTextureWidth, DynamicTextureHeight, PF_FloatRGBA);
	TexInstanceData->UpdateResource();
	TexInstanceData->AddressX = TA_Clamp;
	TexInstanceData->AddressY = TA_Clamp;
	TexInstanceData->Filter = TF_Nearest;
	TexInstanceData->RefreshSamplerStates();
	ParamDoubleBuffer.SetNum(2);
	ParamDoubleBuffer[0].SetNum(TotalCount);
	ParamDoubleBuffer[1].SetNum(TotalCount);
	for (int i = 0; i < TotalCount; ++i)
	{
		ParamDoubleBuffer[0][i] = FLinearColor(0, 0, 0, 1);
		ParamDoubleBuffer[1][i] = FLinearColor(0, 0, 0, 1);
	}

	struct TexAnimRange
	{
		int start;
		int end;
	};
	TArray<TexAnimRange> TexAnimRangeLst;
	TArray<UTexture2D*> TexLst;
	
	// 统计有多少张动画纹理，以及每张动画纹理记录了哪些动画	
	{
		FString ContextString;		
		if (AnimType == EAnimType::EBakeVertexAnimation && AnimInfoBakeVertex.AnimDataTableBakeVertex)
		{
			AnimInfoBakeVertex.AnimSeqDatas.Reset(0);
			TArray<FName> RowNames = AnimInfoBakeVertex.AnimDataTableBakeVertex->GetRowNames();
			for (int i = 0; i < RowNames.Num(); ++i)
			{
				const FName& name = RowNames[i];
				FTableRowAnimData* pRow = AnimInfoBakeVertex.AnimDataTableBakeVertex->FindRow<FTableRowAnimData>(name, ContextString);
				if (pRow)
				{
					if (pRow->TargetTex == nullptr || pRow->TargetTex->GetPathName() != pRow->AnimTexPath)
					{
						pRow->TargetTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(pRow->AnimTexPath)));
					}
					AnimInfoBakeVertex.AnimSeqDatas.Add(*pRow);

					if (TexLst.Contains(pRow->TargetTex.Get()) == false)
					{
						TexAnimRange range;
						range.start = range.end = i;
						TexLst.Add(pRow->TargetTex.Get());
						TexAnimRangeLst.Add(range);
					}
					else
					{
						TexAnimRangeLst[TexAnimRangeLst.Num() - 1].end = i;
					}
				}
			}
		}
		else if(AnimInfoGpuSkin.AnimDataTableGpuSkin)
		{
			AnimInfoGpuSkin.AnimSeqDatas.Reset(0);
			TArray<FName> RowNames = AnimInfoGpuSkin.AnimDataTableGpuSkin->GetRowNames();
			for (int i = 0; i < RowNames.Num(); ++i)
			{
				const FName& name = RowNames[i];
				FTableRowGpuSkinAnimData* pRow = AnimInfoGpuSkin.AnimDataTableGpuSkin->FindRow<FTableRowGpuSkinAnimData>(name, ContextString);
				if (pRow)
				{
					if (pRow->TargetTex == nullptr || pRow->TargetTex->GetPathName() != pRow->AnimTexPath)
					{
						pRow->TargetTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(pRow->AnimTexPath)));
					}
					AnimInfoGpuSkin.AnimSeqDatas.Add(*pRow);

					if (TexLst.Contains(pRow->TargetTex.Get()) == false)
					{
						TexAnimRange range;
						range.start = range.end = i;
						TexLst.Add(pRow->TargetTex.Get());
						TexAnimRangeLst.Add(range);
					}
					else
					{
						TexAnimRangeLst[TexAnimRangeLst.Num() - 1].end = i;
					}
				}
			}
		}

		for (int i = 0; i < TexAnimRangeLst.Num(); ++i)
		{
			UE_LOG(LogTemp, Log, TEXT("LOGDEBUG - Tex_%d anim range from %d - %d"), i, TexAnimRangeLst[i].start, TexAnimRangeLst[i].end);
		}
	}

	if (RandomAnimImgIndexStart < 0 || RandomAnimImgIndexStart >= TexLst.Num())
		RandomAnimImgIndexStart = 0;
	int StartAnimIndex = TexAnimRangeLst[RandomAnimImgIndexStart].start;
	int EndAnimIndex = StartAnimIndex;
	int dst = TexAnimRangeLst.Num() - 1 - StartAnimIndex;
	if (dst > 2)
		dst = 2;
	EndAnimIndex = TexAnimRangeLst[StartAnimIndex + dst].end;

	int AnimLeft = TexLst.Num() - 1 - RandomAnimImgIndexStart;
	if (AnimLeft > 1)
	{
		TexAnimData0 = TexLst[RandomAnimImgIndexStart];
		TexAnimData1 = TexLst[RandomAnimImgIndexStart + 1];
		TexAnimData2 = TexLst[RandomAnimImgIndexStart + 2];
	}
	else if (AnimLeft > 0)
	{
		TexAnimData0 = TexLst[RandomAnimImgIndexStart];
		TexAnimData1 = TexLst[RandomAnimImgIndexStart + 1];
		TexAnimData2 = TexLst[RandomAnimImgIndexStart + 1];
	}
	else
	{
		TexAnimData0 = TexLst[RandomAnimImgIndexStart];
		TexAnimData1 = TexLst[RandomAnimImgIndexStart];
		TexAnimData2 = TexLst[RandomAnimImgIndexStart];
	}

	// 创建GenerateCount个实例对象
	FVector location = GetActorLocation();
	FVector ext = boxRange->GetScaledBoxExtent();
	for (int i = 0; i < GenerateCount; ++i)
	{
		FUInstanceData inst;
		inst.AnimSeq = UKismetMathLibrary::RandomIntegerInRange(StartAnimIndex, EndAnimIndex);
		if (AnimType == EAnimType::EBakeVertexAnimation)
		{
			if (MakeAllInstPlaySpecAnimID >= 0 && inst.AnimSeq < AnimInfoBakeVertex.AnimSeqDatas.Num())
				inst.AnimSeq = MakeAllInstPlaySpecAnimID;
			FTableRowAnimData& animData = AnimInfoBakeVertex.AnimSeqDatas[inst.AnimSeq];
			inst.CurAnimRowStart = animData.StartFrameRow;
		}
		else
		{
			if (MakeAllInstPlaySpecAnimID >= 0 && inst.AnimSeq < AnimInfoGpuSkin.AnimSeqDatas.Num())
				inst.AnimSeq = MakeAllInstPlaySpecAnimID;
			FTableRowGpuSkinAnimData& animData = AnimInfoGpuSkin.AnimSeqDatas[inst.AnimSeq];
			inst.CurAnimRowStart = animData.StartFrameRow;
		}
		inst.AnimTimeCounter = 0;
		inst.AnimPlayRate = UKismetMathLibrary::RandomFloatInRange(0.5f, 1.0f);
		inst.AnimTimeCounter = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);		
		inst.ParentActor = this;
		inst.WorldLocation = location + FVector(
			UKismetMathLibrary::RandomFloatInRange(-ext.X, ext.X),
			UKismetMathLibrary::RandomFloatInRange(-ext.Y, ext.Y),
			UKismetMathLibrary::RandomFloatInRange(-ext.Z, ext.Z));
		inst.WorldRotation = FVector(0, 0, UKismetMathLibrary::RandomFloatInRange(0, 360));
		inst.WorldScale = FVector::OneVector;

		InstLst.Add(inst);
		FTransform trn;
		trn.SetLocation(inst.WorldLocation);
		trn.SetRotation(FQuat::MakeFromEuler(inst.WorldRotation));
		trn.SetScale3D(inst.WorldScale);
		instanceStaticMesh->AddInstanceWorldSpace(trn);
	}
	UE_LOG(LogTemp, Log, TEXT("LOGDEBUG - Generate %d instance for %s"), GenerateCount, *this->GetName());
}

void AInstanceSkeletelMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FName InstanceDataTexParamName(TEXT("InstanceDataTex")),
		InstanceCountParamName("InstanceCount"),
		InstanceDataTextureWidthParamName("InstanceDataTexWidth"),
		InstanceDataTextureHeightParamName("InstanceDataTexHeight"),
		Anim0ParamName(TEXT("AnimTex0")),
		Anim1ParamName(TEXT("AnimTex1")),
		Anim2ParamName(TEXT("AnimTex2")),
		AnimTextureWidthParamName("AnimTextureWidth"),
		AnimTextureHeightParamName("AnimTextureHeight"),
		VertBlendInfoTexParamName("VertBlendInfoTex"),
		VertBlendTexWidthParamName("VertBlendTexWidth"),
		VertBlendTexHeightParamName("VertBlendTexHeight");

	// 写入动态材质实例的参数
	for (int i = 0; i < DynamicMaterialInstances.Num(); ++i)
	{
		UMaterialInstanceDynamic* DynamicMaterialInstance = DynamicMaterialInstances[i];
		DynamicMaterialInstance->SetTextureParameterValue(InstanceDataTexParamName, TexInstanceData);
		DynamicMaterialInstance->SetScalarParameterValue(InstanceCountParamName, InstLst.Num());
		DynamicMaterialInstance->SetScalarParameterValue(InstanceDataTextureWidthParamName, TexInstanceData->GetSizeX());
		DynamicMaterialInstance->SetScalarParameterValue(InstanceDataTextureHeightParamName, TexInstanceData->GetSizeY());
		DynamicMaterialInstance->SetTextureParameterValue(Anim0ParamName, TexAnimData0);
		DynamicMaterialInstance->SetTextureParameterValue(Anim1ParamName, TexAnimData1);
		DynamicMaterialInstance->SetTextureParameterValue(Anim2ParamName, TexAnimData2);		
		DynamicMaterialInstance->SetScalarParameterValue(AnimTextureWidthParamName, TexAnimData0->GetSizeX());
		DynamicMaterialInstance->SetScalarParameterValue(AnimTextureHeightParamName, TexAnimData0->GetSizeY());
		if (AnimType == EAnimType::EGpuSkinAnimation)
		{
			DynamicMaterialInstance->SetTextureParameterValue(VertBlendInfoTexParamName, AnimInfoGpuSkin.TexVertBlendInfo);
			DynamicMaterialInstance->SetScalarParameterValue(VertBlendTexWidthParamName, AnimInfoGpuSkin.TexVertBlendInfo->GetSizeX());
			DynamicMaterialInstance->SetScalarParameterValue(VertBlendTexHeightParamName, AnimInfoGpuSkin.TexVertBlendInfo->GetSizeY());
		}
	}

	CurrentBufferIdx = !CurrentBufferIdx;

	// 并行计算每个实例的动画，并把实例信息写入到实例缓冲区
	ParallelFor(GenerateCount, [&](int32 idx)
	{
		FUInstanceData* inst = &InstLst[idx];
		inst->Tick(DeltaTime);

		// R通道对应第几个实例
		float r = (float)idx;
		// G通道对应实例引用的动画纹理
		float g = (float)inst->TexIndex;
		// B通道对应当前帧动画从第几行像素开始读取
		float b = (float)inst->CurAnimRowStart;
		float a = 0;
		ParamDoubleBuffer[CurrentBufferIdx][idx] = FLinearColor(r, g, b, a);
	});

	// 把实例缓存写到纹理中
	uint8 * pData = (uint8*)(&ParamDoubleBuffer[CurrentBufferIdx][0]);
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		UpdateDynamicTextureCode,
		UTexture2D*, pTexture, TexInstanceData,
		const uint8*, pData, pData,
		{
			FUpdateTextureRegion2D region;
			region.SrcX = 0;
			region.SrcY = 0;
			region.DestX = 0;
			region.DestY = 0;
			region.Width = pTexture->GetSizeX();
			region.Height = pTexture->GetSizeY();
			FTexture2DResource* resource = (FTexture2DResource*)pTexture->Resource;
			RHIUpdateTexture2D(resource->GetTexture2DRHI(), 0, region, region.Width * sizeof(FFloat16Color), pData);
		});

}



#if WITH_EDITOR

void AInstanceSkeletelMeshActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName().IsEqual(FName(TEXT("AnimDataTableBakeVertex"))))
	{
		AnimInfoBakeVertex.AnimSeqDatas.Reset(0);
		if (AnimInfoBakeVertex.AnimDataTableBakeVertex)
		{
			FString ContextString;
			TArray<FName> RowNames = AnimInfoBakeVertex.AnimDataTableBakeVertex->GetRowNames();
			for (auto& name : RowNames)
			{
				FTableRowAnimData* pRow = AnimInfoBakeVertex.AnimDataTableBakeVertex->FindRow<FTableRowAnimData>(name, ContextString);
				if (pRow)
				{
					if (pRow->TargetTex == nullptr || pRow->TargetTex->GetPathName() != pRow->AnimTexPath)
					{
						pRow->TargetTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(pRow->AnimTexPath)));
					}
					AnimInfoBakeVertex.AnimSeqDatas.Add(*pRow);
				}
			}
		}
	}
	else if (PropertyChangedEvent.GetPropertyName().IsEqual(FName(TEXT("AnimDataTableGpuSkin"))))
	{
		AnimInfoGpuSkin.AnimSeqDatas.Reset(0);
		if (AnimInfoGpuSkin.AnimDataTableGpuSkin)
		{
			FString ContextString;
			TArray<FName> RowNames = AnimInfoGpuSkin.AnimDataTableGpuSkin->GetRowNames();
			for (auto& name : RowNames)
			{
				FTableRowGpuSkinAnimData* pRow = AnimInfoGpuSkin.AnimDataTableGpuSkin->FindRow<FTableRowGpuSkinAnimData>(name, ContextString);
				if (pRow)
				{
					if (pRow->TargetTex == nullptr || pRow->TargetTex->GetPathName() != pRow->AnimTexPath)
					{
						pRow->TargetTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(pRow->AnimTexPath)));
					}
					AnimInfoGpuSkin.AnimSeqDatas.Add(*pRow);
				}
			}
		}
	}
}

#endif