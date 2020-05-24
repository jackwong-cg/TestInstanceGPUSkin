#include "Core/InstanceGPUSkinActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Core/Public/Async/ParallelFor.h"



void FUCrowsInstData::Tick(float DeltaTime)
{
	AnimTimeCounter += DeltaTime * AnimPlayRate;
	FTableRowAnimData* rowInfo = &ParentActor->AnimSeqDatas[AnimSeq];
	while (AnimTimeCounter > rowInfo->AnimTimeLength)
	{
		AnimTimeCounter -= rowInfo->AnimTimeLength;
	}
	float rate = AnimTimeCounter / rowInfo->AnimTimeLength;
	TexIndex = rowInfo->TexIndex;
	CurAnimRowStart = (int)((float)rowInfo->FrameCount * rate) * rowInfo->PixelRowsPerFrame + rowInfo->StartFrameRow;
}


//////////////////////////////////////////////////////////////////////////

AInstanceGPUSkinActor::AInstanceGPUSkinActor()
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

void AInstanceGPUSkinActor::BeginPlay()
{
	Super::BeginPlay();

	if (instanceStaticMesh->GetStaticMesh())
	{
		//instanceStaticMesh->OverrideInstanceRandom = true;
		FStaticMeshLODResources& LODModel = instanceStaticMesh->GetStaticMesh()->RenderData->LODResources[0];
		{
			// Mesh doesn't have a color vertex buffer yet!  We'll create one now.
			LODModel.VertexBuffers.ColorVertexBuffer.InitFromSingleColor(FColor(255, 0, 0, 255), LODModel.GetNumVertices());
			for (int i = 0; i < LODModel.GetNumVertices(); i++)
			{
				LODModel.VertexBuffers.ColorVertexBuffer.VertexColor(i) = FColor(i / 255, i % 255, 0, 255);
			}
			// @todo MeshPaint: Make sure this is the best place to do this
			BeginInitResource(&LODModel.VertexBuffers.ColorVertexBuffer);
		}
		instanceStaticMesh->GetStaticMesh()->InitResources();
		instanceStaticMesh->MarkRenderStateDirty();
	}


	TArray<UTexture2D*> TexLst;
	AnimSeqDatas.Reset(0);
	if (AnimDataTable)
	{
		FString ContextString;
		TArray<FName> RowNames = AnimDataTable->GetRowNames();
		for (auto& name : RowNames)
		{
			FTableRowAnimData* pRow = AnimDataTable->FindRow<FTableRowAnimData>(name, ContextString);
			if (pRow)
			{
				if (pRow->TargetTex == nullptr || pRow->TargetTex->GetPathName() != pRow->AnimTexPath)
				{
					pRow->TargetTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(pRow->AnimTexPath)));
				}
				AnimSeqDatas.Add(*pRow);

				if (TexLst.Contains(pRow->TargetTex.Get()) == false)
				{
					TexLst.Add(pRow->TargetTex.Get());
				}
			}
		}
	}

	if (TexLst.Num() > 2)
	{
		TexAnimData0 = TexLst[0];
		TexAnimData1 = TexLst[1];
		TexAnimData2 = TexLst[2];
	}
	else if (TexLst.Num() > 1)
	{
		TexAnimData0 = TexLst[0];
		TexAnimData1 = TexLst[1];
		TexAnimData2 = TexLst[1];
	}
	else
	{
		TexAnimData0 = TexLst[0];
		TexAnimData1 = TexLst[0];
		TexAnimData2 = TexLst[0];
	}

	DynamicMaterialInstance = instanceStaticMesh->CreateDynamicMaterialInstance(0, SrcMaterial);
	instanceStaticMesh->SetMaterial(0, DynamicMaterialInstance);

	int DynamicTextureWidth = 1024, DynamicTextureHeight = 1024;
	int TotalCount = DynamicTextureWidth * DynamicTextureHeight;
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

	FVector location = GetActorLocation();
	FVector ext = boxRange->GetScaledBoxExtent();

	for (int i = 0; i < GenerateCount; ++i)
	{
		FUCrowsInstData inst;
		inst.AnimSeq = UKismetMathLibrary::RandomIntegerInRange(0, AnimSeqDatas.Num() - 1);
		FTableRowAnimData& animData = AnimSeqDatas[inst.AnimSeq];
		inst.AnimTimeCounter = 0;
		inst.AnimPlayRate = UKismetMathLibrary::RandomFloatInRange(0.5f, 1.0f);
		inst.AnimTimeCounter = UKismetMathLibrary::RandomFloatInRange(0.0f, 1.0f);
		inst.CurAnimRowStart = animData.StartFrameRow;
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
}

void AInstanceGPUSkinActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FName TexParamName(TEXT("InstanceData")),
		Anim0ParamName(TEXT("AnimTex0")),
		Anim1ParamName(TEXT("AnimTex1")),
		Anim2ParamName(TEXT("AnimTex2")),
		InstanceCountParamName("InstanceCount"),
		TextureWidthParamName("TextureWidth"),
		TextureHeightParamName("TextureHeight");

	DynamicMaterialInstance->SetTextureParameterValue(TexParamName, TexInstanceData);
	DynamicMaterialInstance->SetTextureParameterValue(Anim0ParamName, TexAnimData0);
	DynamicMaterialInstance->SetTextureParameterValue(Anim1ParamName, TexAnimData1);
	DynamicMaterialInstance->SetTextureParameterValue(Anim2ParamName, TexAnimData2);
	DynamicMaterialInstance->SetScalarParameterValue(InstanceCountParamName, InstLst.Num());
	DynamicMaterialInstance->SetScalarParameterValue(TextureWidthParamName, TexAnimData0->GetSizeX());
	DynamicMaterialInstance->SetScalarParameterValue(TextureHeightParamName, TexAnimData0->GetSizeY());

	CurrentBufferIdx = !CurrentBufferIdx;

	ParallelFor(GenerateCount, [&](int32 idx)
	{
		FUCrowsInstData* inst = &InstLst[idx];
		inst->Tick(DeltaTime);

		float r = (float)idx;
		float g = (float)inst->TexIndex;
		float b = (float)inst->CurAnimRowStart;
		float a = 0;
		ParamDoubleBuffer[CurrentBufferIdx][idx] = FLinearColor(r, g, b, a);
	});

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

void AInstanceGPUSkinActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName().IsEqual(FName(TEXT("AnimDataTable"))))
	{
		AnimSeqDatas.Reset(0);

		if (AnimDataTable)
		{
			FString ContextString;
			TArray<FName> RowNames = AnimDataTable->GetRowNames();
			for (auto& name : RowNames)
			{
				FTableRowAnimData* pRow = AnimDataTable->FindRow<FTableRowAnimData>(name, ContextString);
				if (pRow)
				{
					if (pRow->TargetTex == nullptr || pRow->TargetTex->GetPathName() != pRow->AnimTexPath)
					{
						pRow->TargetTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(pRow->AnimTexPath)));
					}
					AnimSeqDatas.Add(*pRow);
				}
			}
		}
	}
}

#endif