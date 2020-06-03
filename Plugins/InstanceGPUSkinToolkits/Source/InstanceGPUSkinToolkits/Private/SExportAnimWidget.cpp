// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#if WITH_EDITOR

#include "SExportAnimWidget.h"
#include "Misc/Paths.h"
#include "Widgets/SNullWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/CoreStyle.h"
#include "Application/SlateWindowHelper.h"
#include "SlateOptMacros.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"

#include "Runtime/AssetRegistry/Public/AssetData.h"
#include "Editor/ContentBrowser/Public/ContentBrowserModule.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Editor/ContentBrowser/Private/ContentBrowserSingleton.h"
#include "Runtime/Engine/Classes/Animation/AnimationAsset.h"
#include "Runtime/Engine/Classes/Animation/Skeleton.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshModel.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshLODModel.h"
#include "Runtime/Engine/Classes/Animation/AnimSequence.h"

#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "IImageWrapperModule.h"
#include "ModuleManager.h"
#include "IImageWrapper.h"
#include "ImageUtils.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"

#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Runtime/RenderCore/Public/RenderUtils.h"
#include "Engine/Engine.h"
#include "TableRowAnimData.h"

#include "UnrealEd/Public/PackageHelperFunctions.h"

#define LOCTEXT_NAMESPACE "ExportAnimWidget"

SExportAnimWidget* SExportAnimWidget::sInst = nullptr;
FDelegateHandle SExportAnimWidget::sDelegateHandle;

// 获取指定时间下的动画里某个骨骼的Transform
FTransform GetBoneTransform(USkeletalMesh* skeletalMesh, UAnimSequence* animSequence, float time, FName boneName)
{
	FTransform boneTransform = FTransform::Identity;
	int boneIndex = -1;
	boneIndex = skeletalMesh->RefSkeleton.FindBoneIndex(boneName);
	if (-1 != boneIndex)
	{
		//int trackIndex = skeletalMesh->Skeleton->GetAnimationTrackIndex(boneIndex, animSequence, true);
#if ENGINE_MINOR_VERSION > 22
		int trackIndex = skeletalMesh->Skeleton->GetRawAnimationTrackIndex(boneIndex, animSequence);
#else
		int trackIndex = skeletalMesh->Skeleton->GetAnimationTrackIndex(boneIndex, animSequence, true);
#endif
		if (-1 != trackIndex)
		{
			animSequence->GetBoneTransform(boneTransform, trackIndex, time, true);
		}
	}
	return boneTransform;
}

void PackVectorPart(float v, int& intPart, int& floatPart)
{
	const float minV = -60;
	const float maxV = 60.0f;
	float lerpV = (v - minV) / (maxV - minV);
	float iP = 0;
	float fP = FMath::Modf(lerpV, &iP) * 255.0f;
	intPart = (int)iP;
	floatPart = (int)fP;
}

void SaveRGBA8Texture(TArray<FColor>& imgBuf, FString GenerateFilePath, int SizeX, int SizeY)
{
	int PixelCount = SizeX * SizeY;
	FString Name, PackageName;
	IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(GenerateFilePath, TEXT(""), PackageName, Name);

	UTexture2D* NewBoneLut = nullptr;
	UPackage * Pkg = CreatePackage(NULL, *PackageName);

	LLM_SCOPE(ELLMTag::Textures);
	NewBoneLut = NewObject<UTexture2D>(Pkg, FName(*Name), RF_Public | RF_Standalone);
	EPixelFormat LutPixelFormat = EPixelFormat::PF_B8G8R8A8;
	NewBoneLut->PlatformData = new FTexturePlatformData();
	NewBoneLut->PlatformData->SizeX = SizeX;
	NewBoneLut->PlatformData->SizeY = SizeY;
	NewBoneLut->PlatformData->PixelFormat = LutPixelFormat;
	NewBoneLut->CompressionSettings = TC_VectorDisplacementmap;
	NewBoneLut->SRGB = true;
	NewBoneLut->LODGroup = TEXTUREGROUP_UI;
	NewBoneLut->Filter = TF_Nearest;

	int NumBlocksX = SizeX / GPixelFormats[LutPixelFormat].BlockSizeX;
	int NumBlocksY = SizeY / GPixelFormats[LutPixelFormat].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	NewBoneLut->PlatformData->Mips.Add(Mip);
	Mip->SizeX = SizeX;
	Mip->SizeY = SizeY;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* allocData = Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[LutPixelFormat].BlockBytes);
	Mip->BulkData.Unlock();
	NewBoneLut->Source.Init(SizeX, SizeY, 1, 1, TSF_BGRA8, (uint8*)imgBuf.GetData());

	uint8* TextureData = (uint8*)NewBoneLut->Source.LockMip(0);
	if (TextureData)
	{
		FMemory::Memcpy(TextureData, imgBuf.GetData(), PixelCount * sizeof(FColor));
	}
	NewBoneLut->Source.UnlockMip(0);
	NewBoneLut->CompressionNone = true;
	NewBoneLut->DeferCompression = false;
	NewBoneLut->PostEditChange();
	NewBoneLut->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewBoneLut);
	NewBoneLut->PostEditChange();
}


// 把每通道16位浮点精度的颜色数组写到纹理
// imgBuf ： 颜色数组
// GenerateFilePath ： 文件路径
// SizeX ： 纹理宽度
// SizeY ： 纹理高度
void SaveRGBA16FloatTexture(TArray<FFloat16Color>& imgBuf, FString GenerateFilePath, int SizeX, int SizeY)
{
	int PixelCount = SizeX * SizeY;
	FString Name, PackageName, Folder;
	IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	//AssetTools.CreateUniqueAssetName(GenerateFilePath, TEXT(""), PackageName, Name);
	PackageName = GenerateFilePath;
	PackageName.Split(TEXT("/"), &Folder, &Name, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	UTexture2D* NewBoneLut = nullptr;
	UPackage * Pkg = FindPackage(NULL, *PackageName);
	if (Pkg == nullptr)
	{
		Pkg = CreatePackage(NULL, *PackageName);
	}

	LLM_SCOPE(ELLMTag::Textures);
	NewBoneLut = NewObject<UTexture2D>(Pkg, FName(*Name), RF_Public | RF_Standalone);
	EPixelFormat LutPixelFormat = EPixelFormat::PF_FloatRGBA;
	NewBoneLut->PlatformData = new FTexturePlatformData();
	NewBoneLut->PlatformData->SizeX = SizeX;
	NewBoneLut->PlatformData->SizeY = SizeY;
	NewBoneLut->PlatformData->PixelFormat = LutPixelFormat;
	NewBoneLut->CompressionSettings = TC_HDR;
	NewBoneLut->SRGB = false;
	NewBoneLut->Filter = TextureFilter::TF_Nearest;
	//NewBoneLut->LODGroup = TEXTUREGROUP_16BitData;

	int NumBlocksX = SizeX / GPixelFormats[LutPixelFormat].BlockSizeX;
	int NumBlocksY = SizeY / GPixelFormats[LutPixelFormat].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	NewBoneLut->PlatformData->Mips.Add(Mip);
	Mip->SizeX = SizeX;
	Mip->SizeY = SizeY;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[LutPixelFormat].BlockBytes);
	Mip->BulkData.Unlock();
	NewBoneLut->Source.Init(SizeX, SizeY, 1, 1, TSF_RGBA16F);

	uint8* TextureData = (uint8*)NewBoneLut->Source.LockMip(0);
	if (TextureData != nullptr)
	{
		FMemory::Memcpy(TextureData, imgBuf.GetData(), PixelCount * sizeof(FFloat16Color));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Save texture %s failed! Error - LockMip(0) failed!"), *PackageName);
	}
	NewBoneLut->Source.UnlockMip(0);
	NewBoneLut->CompressionNone = true;
	NewBoneLut->DeferCompression = false;
	NewBoneLut->PostEditChange();
	NewBoneLut->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewBoneLut);
	NewBoneLut->UpdateResource();
	Pkg->SetDirtyFlag(true);

	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(NewBoneLut->GetOutermost());
	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, /*bPromptToSave=*/ false);
}



FReply SExportAnimWidget::OnExportAnimationToFloatTextureClick()
{
	FString Name, PackageName, DataAssetPath = inputExportFilePath->GetText().ToString(), Folder;
	DataAssetPath.Append("_AnimCfg");
	IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	//AssetTools.CreateUniqueAssetName(DataAssetPath, TEXT(""), PackageName, Name);
	PackageName = DataAssetPath;
	PackageName.Split(TEXT("/"), &Folder, &Name, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	UPackage * Pkg = FindPackage(NULL, *PackageName);
	if (Pkg == nullptr)
	{
		Pkg = CreatePackage(NULL, *PackageName);
	}
	UDataTable* dataTable = NewObject<UDataTable>(Pkg, FName(*Name), RF_Public | RF_Standalone);
	dataTable->RowStruct = FTableRowAnimData::StaticStruct();
	TArray<FTableRowAnimData> animDatas;

	// 设置导出的纹理的大小		
	int SizeX = FCString::Atoi(*(inputTexWidth.Get()->GetText().ToString()));
	int SizeY = FCString::Atoi(*(inputTexHeight.Get()->GetText().ToString()));
	int PixelCount = SizeX * SizeY;
	// 初始化浮点颜色格式的数组，用来存储顶点坐标
	TArray<FFloat16Color> imgBuf;
	imgBuf.AddZeroed(PixelCount);
	int r = 0;
	int c = 0;	
	// 当前使用的纹理已经记录了多少帧
	int frameCountHasRecord = 0;
	// 总共记录了多少张纹理
	int ImageCount = 0;

	TArray<FAssetData> Selection;
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().GetSelectedAssets(Selection);

	// 遍历选中的资源
	for (int i = 0; i < Selection.Num(); ++i)
	{
		UObject* assest = Selection[i].GetAsset();
		UClass* cls = assest->GetClass();
		
		// 判断是否动画序列资源
		if (assest->GetClass() == UAnimSequence::StaticClass())
		{
			UAnimSequence* animSeq = Cast<UAnimSequence>(Selection[i].GetAsset());
			USkeletalMesh* sklMesh = animSeq->GetPreviewMesh();

			// 如果预览的网格存在
			if (sklMesh)
			{
				// 骨骼mesh的TPose的所有骨骼的矩阵
				const TArray<FMatrix>& baseInvMatrix = sklMesh->RefBasesInvMatrix;

				// 获取骨骼动画模型
				FSkeletalMeshModel* skeletalMeshModel = sklMesh->GetImportedModel();
				// 获取第0个lod的网格
				const FSkeletalMeshLODModel& skeMeshLodMesh = skeletalMeshModel->LODModels[0];
				TArray<FSoftSkinVertex> arrayVertices;
				// 获取骨骼mesh的所有顶点在TPose里的信息 
				skeMeshLodMesh.GetVertices(arrayVertices);
				int meshVertCount = arrayVertices.Num();

				// 获取所有的骨骼信息
				const TArray<FMeshBoneInfo>& arrayMeshBoneInfo = animSeq->GetSkeleton()->GetReferenceSkeleton().GetRefBoneInfo();

				struct FBoneData
				{
					int parentIndex = -1;
					FName boneName;
				};

				// 记录每个骨骼的父骨骼信息以及骨骼名称
				TArray<FBoneData> arrayBoneData;
				for (int boneIndex = 0; boneIndex < arrayMeshBoneInfo.Num(); ++boneIndex)
				{
					const FMeshBoneInfo& meshBoneInfo = arrayMeshBoneInfo[boneIndex];
					arrayBoneData.Add(FBoneData{ meshBoneInfo.ParentIndex, meshBoneInfo.Name });
					//// 打印骨骼信息
					//UE_LOG(LogTemp, Log, TEXT("ParentBoneID = %d, CurBoneName = %s"), meshBoneInfo.ParentIndex, *meshBoneInfo.Name.ToString());
				}

				// 获取动画帧数
				int frameCountOfCurAnimSequence = animSeq->GetNumberOfFrames();
				// 计算存储一帧的动画数据需要多少行像素
				const int NeedRowsPerFrame = FMath::CeilToInt((float)(meshVertCount) / SizeX);
				// 计算存储这个动画还需要多少个像素行
				int needPixelRows = frameCountOfCurAnimSequence * NeedRowsPerFrame;
				// 计算还剩余多少个闲置的像素行
				int freeRowsLeft = SizeY - NeedRowsPerFrame * frameCountHasRecord;

				// 先判断一张纹理是否能够保存这个动画数据
				if (needPixelRows > SizeY)
				{
					UE_LOG(LogTemp, Error, TEXT("纹理太小(需要 %d 行, 最多只有 %d 行), 不足以保存动画数据[ %s], 跳过保存此动画!"), needPixelRows, SizeY, *animSeq->GetFullName());
					continue;
				}

				// 当前剩余的缓存不够用了，就先把纹理数据给保存下来，并重置缓存
				if (freeRowsLeft < needPixelRows)
				{
					FString GenerateFilePath = inputExportFilePath->GetText().ToString();
					GenerateFilePath.Append("_Tex_").AppendInt(ImageCount);
					SaveRGBA16FloatTexture(imgBuf, GenerateFilePath, SizeX, SizeY);

					imgBuf.Reset(PixelCount);
					imgBuf.AddZeroed(PixelCount);
					frameCountHasRecord = 0;
					++ImageCount;
				}

				// 记录动画序列的信息
				FTableRowAnimData dataRow;
				dataRow.AnimName = animSeq->GetName();
				dataRow.AnimTimeLength = animSeq->GetTimeAtFrame(frameCountOfCurAnimSequence - 1);
				dataRow.FrameCount = frameCountOfCurAnimSequence;
				dataRow.StartFrameRow = frameCountHasRecord * NeedRowsPerFrame;
				dataRow.EndFrameRow = dataRow.StartFrameRow + NeedRowsPerFrame * frameCountOfCurAnimSequence - 1;
				dataRow.AnimTexPath = inputExportFilePath->GetText().ToString();
				dataRow.AnimTexPath.Append("_Tex_").AppendInt(ImageCount);
				dataRow.PixelRowsPerFrame = NeedRowsPerFrame;
				dataRow.TexIndex = ImageCount;
				animDatas.Add(dataRow);

				// 遍历所有的动画帧
				for (int frameID = 0; frameID < frameCountOfCurAnimSequence; ++frameID)
				{
					TMap<FName, FMatrix> mapBoneMatrix;
					TMap<int, FMatrix> mapBoneIDMatrix;
					// 获取这一帧所对应的时间
					float testTime = animSeq->GetTimeAtFrame(frameID);
					// 遍历所有的骨骼
					for (int boneIndex = 0; boneIndex < arrayBoneData.Num(); ++boneIndex)
					{
						// 计算每个骨骼变换到局部空间下的变换矩阵
						FMatrix boneMatrix = baseInvMatrix[boneIndex];
						int parentIndex = boneIndex;
						while (-1 != parentIndex)
						{
							FTransform boneTransform = GetBoneTransform(
								sklMesh,
								animSeq,
								testTime,
								arrayBoneData[parentIndex].boneName);
							boneMatrix *= boneTransform.ToMatrixWithScale();
							parentIndex = arrayBoneData[parentIndex].parentIndex;
						}
						mapBoneMatrix.Add(arrayBoneData[boneIndex].boneName, boneMatrix);
						mapBoneIDMatrix.Add(boneIndex, boneMatrix);
					}
					
					int curVertexIndexG = 0;
					// 遍历所有的子网格
					for (int sectionIndex = 0; sectionIndex < skeMeshLodMesh.Sections.Num(); ++sectionIndex)
					{
						const FSkelMeshSection& skelMeshSection = skeMeshLodMesh.Sections[sectionIndex];
						// 子网格的顶点数
						const int vertCount = skelMeshSection.SoftVertices.Num();
						// 遍历子网格所有顶点
						for (int vertexIndex = 0; vertexIndex < vertCount; ++vertexIndex)
						{
							// 计算顶点蒙皮后的局部空间坐标
							FVector pos = FVector::ZeroVector;
							const FSoftSkinVertex& vert = skelMeshSection.SoftVertices[vertexIndex];
							for (int blendid = 0; blendid < MAX_TOTAL_INFLUENCES; ++blendid)
							{
								int vertexVirtualBoneIndex = vert.InfluenceBones[blendid];
								int vertexBoneIndex = skelMeshSection.BoneMap[vertexVirtualBoneIndex];
								uint8 weight = vert.InfluenceWeights[blendid];
								if (weight > 0)
								{
									float blendrate = (float)weight / 255.0f;
									pos += mapBoneIDMatrix[vertexBoneIndex].TransformPosition(vert.Position) * blendrate;
								}
							}

							// 计算这个顶点在这一帧所在的像素行和列
							r = frameCountHasRecord * NeedRowsPerFrame + curVertexIndexG / SizeX;
							c = curVertexIndexG % SizeX;

							// 计算顶点相对于TPose下的偏移
							FVector offset = pos - vert.Position;
							int index = r * SizeX + c;
							// 把位置偏移数据写道纹理数据里
							imgBuf[index] = FFloat16Color( FLinearColor(offset));

							++curVertexIndexG;
						}
					}

					// 记录的总动画帧数累加
					++frameCountHasRecord;
				}
			}
		}
	}

	// 保存最新的那张纹理
	FString GenerateFilePath = inputExportFilePath->GetText().ToString();
	GenerateFilePath.Append("_Tex_").AppendInt(ImageCount);
	SaveRGBA16FloatTexture(imgBuf, GenerateFilePath, SizeX, SizeY);

	// 读取数据表里引用的纹理
	for (int i = 0; i < animDatas.Num(); ++i)
	{
		FTableRowAnimData& data = animDatas[i];
		FName rowName(*FString::FromInt(i));
		data.TargetTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(data.AnimTexPath)));
		dataTable->AddRow(rowName, data);
	}

	// 保存数据表
	dataTable->PostEditChange();
	dataTable->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(dataTable);
	Pkg->SetDirtyFlag(true);
	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(dataTable->GetOutermost());
	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, /*bPromptToSave=*/ false);

	return FReply::Handled();
}


FReply SExportAnimWidget::OnExportAnimationToCommonTextureClick()
{
	float MIN_OFF = 999999;
	float MAX_OFF = -999999;

	TArray<FAssetData> Selection;
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().GetSelectedAssets(Selection);
	// 获取选中的资源
	for (int i = 0; i < Selection.Num(); ++i)
	{
		UObject* assest = Selection[i].GetAsset();
		UClass* cls = assest->GetClass();
		// 判断是否动画序列资源
		if (assest->GetClass() == UAnimSequence::StaticClass())
		{
			UAnimSequence* animSeq = Cast<UAnimSequence>(Selection[i].GetAsset());
			USkeletalMesh* sklMesh = animSeq->GetPreviewMesh();
			// 如果预览的网格存在
			if (sklMesh)
			{
				const TArray<FRawAnimSequenceTrack>& animDataLst = animSeq->GetRawAnimationData();
				// 骨骼mesh的TPose的所有骨骼的矩阵
				const TArray<FMatrix>& baseInvMatrix = sklMesh->RefBasesInvMatrix;

				// 
				FSkeletalMeshModel* skeletalMeshModel = sklMesh->GetImportedModel();
				const FSkeletalMeshLODModel& skeMeshLodMesh = skeletalMeshModel->LODModels[0];
				TArray<FSoftSkinVertex> arrayVertices;

				// 获取骨骼mesh的所有顶点在TPose里的骨骼空间坐标 
				skeMeshLodMesh.GetVertices(arrayVertices);
				int vCount = arrayVertices.Num();

				// 获取所有的骨骼信息
				const TArray<FMeshBoneInfo>& arrayMeshBoneInfo = animSeq->GetSkeleton()->GetReferenceSkeleton().GetRefBoneInfo();

				struct FBoneData
				{
					int parentIndex = -1;
					FName boneName;
				};

				// 记录每个骨骼的父骨骼信息以及骨骼名称
				TArray<FBoneData> arrayBoneData;
				for (int boneIndex = 0; boneIndex < arrayMeshBoneInfo.Num(); ++boneIndex)
				{
					const FMeshBoneInfo& meshBoneInfo = arrayMeshBoneInfo[boneIndex];
					arrayBoneData.Add(FBoneData{ meshBoneInfo.ParentIndex, meshBoneInfo.Name });
					UE_LOG(LogTemp, Log, TEXT("ParentBoneID = %d, CurBoneName = %s"), meshBoneInfo.ParentIndex, *meshBoneInfo.Name.ToString());
				}

				// 设置导出的纹理的大小
				int SizeX = FCString::Atoi(*(inputTexWidth.Get()->GetText().ToString()));
				int SizeY = FCString::Atoi(*(inputTexHeight.Get()->GetText().ToString()));
				int PixelCount = SizeX * SizeY;
				// 初始化浮点颜色格式的数组，用来存储顶点坐标
				TArray<FColor> imgBufXY, imgBufZW;
				imgBufXY.AddZeroed(PixelCount);
				imgBufZW.AddZeroed(PixelCount);
				int r = 0;
				int c = 0;

				// 遍历动画的所有帧信息
				int frameCount = animSeq->GetNumberOfFrames();
				for (int frameID = 0; frameID < frameCount; ++frameID)
				{
					TMap<FName, FMatrix> mapBoneMatrix;
					TMap<int, FMatrix> mapBoneIDMatrix;
					// 获取这一帧所对应的时间
					float testTime = animSeq->GetTimeAtFrame(frameID);
					// 遍历所有的骨骼
					for (int boneIndex = 0; boneIndex < arrayBoneData.Num(); ++boneIndex)
					{
						// 计算每个骨骼变换到局部空间下的变换矩阵
						FMatrix boneMatrix = baseInvMatrix[boneIndex];
						int parentIndex = boneIndex;
						while (-1 != parentIndex)
						{
							FTransform boneTransform = GetBoneTransform(
								sklMesh,
								animSeq,
								testTime,
								arrayBoneData[parentIndex].boneName);
							boneMatrix *= boneTransform.ToMatrixWithScale();
							parentIndex = arrayBoneData[parentIndex].parentIndex;
						}
						mapBoneMatrix.Add(arrayBoneData[boneIndex].boneName, boneMatrix);
						mapBoneIDMatrix.Add(boneIndex, boneMatrix);
					}

					// 遍历网格
					for (int sectionIndex = 0; sectionIndex < skeMeshLodMesh.Sections.Num(); ++sectionIndex)
					{
						const FSkelMeshSection& skelMeshSection = skeMeshLodMesh.Sections[sectionIndex];
						// 网格的顶点数
						const int vertCount = skelMeshSection.SoftVertices.Num();
						// 计算一个动画帧下所有网格顶点占据的像素行数
						const int rowsPerFrame = FMath::CeilToInt((float)(vertCount) / SizeX);
						// 遍历所有顶点
						for (int vertexIndex = 0; vertexIndex < vertCount; ++vertexIndex)
						{
							// 计算顶点蒙皮后的局部空间坐标
							FVector pos = FVector::ZeroVector;
							const FSoftSkinVertex& vert = skelMeshSection.SoftVertices[vertexIndex];
							for (int blendid = 0; blendid < MAX_TOTAL_INFLUENCES; ++blendid)
							{
								int vertexVirtualBoneIndex = vert.InfluenceBones[blendid];
								int vertexBoneIndex = skelMeshSection.BoneMap[vertexVirtualBoneIndex];
								uint8 weight = vert.InfluenceWeights[blendid];
								if (weight > 0)
								{
									float blendrate = (float)weight / 255.0f;
									pos += mapBoneIDMatrix[vertexBoneIndex].TransformPosition(vert.Position) * blendrate;
								}
							}

							// 计算这个顶点在这一帧所在的像素行和列
							r = frameID * rowsPerFrame + vertexIndex / SizeX;
							c = vertexIndex % SizeX;

							// 计算顶点相对于TPose下的偏移
							FVector offset = pos - vert.Position;
							int index = r * SizeX + c;
							// 把位置偏移数据写道纹理数据里
							int xiP, xfP, yiP, yfP, ziP, zfP;
							PackVectorPart(offset.X, xiP, xfP);
							PackVectorPart(offset.Y, yiP, yfP);
							PackVectorPart(offset.Z, ziP, zfP);
							imgBufXY[index] = FColor(xiP, xfP, yiP, yfP);
							imgBufZW[index] = FColor(ziP, zfP, 0, 255);

							if (offset.X < MIN_OFF) MIN_OFF = offset.X;
							if (offset.Y < MIN_OFF) MIN_OFF = offset.Y;
							if (offset.Z < MIN_OFF) MIN_OFF = offset.Z;
							if (offset.X > MAX_OFF) MAX_OFF = offset.X;
							if (offset.Y > MAX_OFF) MAX_OFF = offset.Y;
							if (offset.Z > MAX_OFF) MAX_OFF = offset.Z;
						}
					}
				}

				FString GenerateFilePathXY = FString::Printf(TEXT("/Game/InstAnimations/%s_BoneLutComXY"), *animSeq->GetFName().ToString());
				FString GenerateFilePathZW = FString::Printf(TEXT("/Game/InstAnimations/%s_BoneLutComZW"), *animSeq->GetFName().ToString());
				SaveRGBA8Texture(imgBufXY, GenerateFilePathXY, SizeX, SizeY);
				SaveRGBA8Texture(imgBufZW, GenerateFilePathZW, SizeX, SizeY);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MIN_OFF = %f, MAX_OFF = %f"), MIN_OFF, MAX_OFF);

	return FReply::Handled();

}

void SExportAnimWidget::DestroyInstance()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FContentBrowserModule::FOnAssetSelectionChanged& AssetSelectionChangedDelegate = ContentBrowserModule.GetOnAssetSelectionChanged();
	AssetSelectionChangedDelegate.Remove(sDelegateHandle);

	sInst = nullptr;
}


void SExportAnimWidget::Construct(const FArguments& InDelcaration)
{
	sInst = this;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FContentBrowserModule::FOnAssetSelectionChanged& AssetSelectionChangedDelegate = ContentBrowserModule.GetOnAssetSelectionChanged();
	sDelegateHandle = AssetSelectionChangedDelegate.AddLambda([this](const TArray<FAssetData>&, bool)
	{
		this->RefreshUI();
	});

	const int ROW_HEIGHT = 40;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
					.MaxHeight(ROW_HEIGHT)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("图片宽度")))
							]
						+ SHorizontalBox::Slot()
							[
								SAssignNew(inputTexWidth, SEditableTextBox)
								.Text(FText::FromString(TEXT("1024")))
							]
						+ SHorizontalBox::Slot()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("图片高度")))
							]
						+ SHorizontalBox::Slot()
							[
								SAssignNew(inputTexHeight, SEditableTextBox)
								.Text(FText::FromString(TEXT("1024")))
							]
					]
				+ SVerticalBox::Slot()
					.MaxHeight(ROW_HEIGHT)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							[
								SNew(STextBlock)
								.Text(FText::FromName(TEXT("导出路径")))
							]
						+ SHorizontalBox::Slot()
							[
								SAssignNew(inputExportFilePath, SEditableTextBox)
								.Text(FText::FromString(TEXT("/Game/ExportAnimations/Crow/Crow")))
							]
					]
				+ SVerticalBox::Slot()
					.MaxHeight(ROW_HEIGHT)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString(TEXT("导出选中的动画到浮点纹理")))
						.OnClicked(this, &SExportAnimWidget::OnExportAnimationToFloatTextureClick)
					]
				//+ SVerticalBox::Slot()
				//	.MaxHeight(30)
				//	[
				//		SNew(SButton)
				//		.HAlign(HAlign_Center)
				//		.Text(FText::FromString(TEXT("导出选中的动画到普通纹理")))
				//		.OnClicked(this, &SExampleLayoutEx::OnExportAnimationToCommonTextureClick)
				//	]
				+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(txtSelectAnimation, SMultiLineEditableText)
						.IsReadOnly(true)
						.AutoWrapText(false)
						.Text(FText::FromString(""))
					]
			]
		];
}


void SExportAnimWidget::RefreshUI()
{
	if (sInst == nullptr)
		return;	

	TArray<FAssetData> Selection;
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().GetSelectedAssets(Selection);

	FString anim = "";
	for (int i = 0; i < Selection.Num(); ++i)
	{
		UObject* assest = Selection[i].GetAsset();
		UClass* cls = assest->GetClass();
		// 判断是否动画序列资源
		if (assest->GetClass() == UAnimSequence::StaticClass())
		{
			anim.Append(Selection[i].GetFullName());
			anim.Append(";\n");
		}
	}
	txtSelectAnimation.Get()->SetText(FText::FromString(anim));

}


void SExportAnimWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	timeAccum += InDeltaTime;
	if (timeAccum - time > 2.0f)
	{
		time = timeAccum;

	}
}

	
	bool SExportAnimWidget::OnVisualizeTooltip( const TSharedPtr<SWidget>& TooltipContent )
	{
		return true;
	}


#undef LOCTEXT_NAMESPACE



#endif