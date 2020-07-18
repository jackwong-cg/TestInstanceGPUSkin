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
#include "TableRowGpuSkinAnimData.h"

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


void SaveGpuSkinBlendInfoToTexture(bool checkBlendInfo, const FSkeletalMeshLODModel& skeMeshLodMesh, int meshVertCount, FString exportFolderPath)
{
	// 检测骨骼混合情况
	if (checkBlendInfo)
	{
		// 预检测每个顶点的混合情况
		UE_LOG(LogTemp, Log, TEXT("================ check vertex blend info start ->"));
		int MaxBlendBones = 0;
		int GlobalVertIndex = 0;
		for (int sectionIndex = 0; sectionIndex < skeMeshLodMesh.Sections.Num(); ++sectionIndex)
		{
			const FSkelMeshSection& skelMeshSection = skeMeshLodMesh.Sections[sectionIndex];
			// 子网格的顶点数
			const int vertCount = skelMeshSection.SoftVertices.Num();

			// 遍历子网格所有顶点
			for (int vertexIndex = 0; vertexIndex < vertCount; ++vertexIndex)
			{
				bool findZero = false;
				bool hasZeroInMid = false;

				int BlendBoneCount = 0;
				FString StrBlend = "";
				const FSoftSkinVertex& vert = skelMeshSection.SoftVertices[vertexIndex];
				for (int blendid = 0; blendid < MAX_TOTAL_INFLUENCES; ++blendid)
				{
					int vertexVirtualBoneIndex = vert.InfluenceBones[blendid];
					int vertexBoneIndex = skelMeshSection.BoneMap[vertexVirtualBoneIndex];
					uint8 weight = vert.InfluenceWeights[blendid];
					if (weight > 0)
					{
						if (findZero)
							hasZeroInMid = true;
						BlendBoneCount++;
					}
					else
					{
						findZero = true;
					}
					StrBlend.Append(*FString::Format(TEXT("[{0}, {1}] "), { FString::FormatAsNumber(vertexBoneIndex), FString::FormatAsNumber(weight) }));
				}
				UE_LOG(LogTemp, Log, TEXT("%s Vert [%d],\t BlendCount = %d, Detail : %s"), (hasZeroInMid ? TEXT("HasZero") : TEXT("")), GlobalVertIndex++, BlendBoneCount, *StrBlend);
				if (MaxBlendBones < BlendBoneCount)
				{
					MaxBlendBones = BlendBoneCount;
				}
			}
		}
		UE_LOG(LogTemp, Log, TEXT("MaxBlendBoneCount = %d"), MaxBlendBones);
		UE_LOG(LogTemp, Log, TEXT("================ check vertex blend info end ->"));
	}

	TArray<FFloat16Color> imgBufBlend;
	int ImgBlendX = 128;
	int ImgBlendY = 128;
	int ImgBlendPixelCount = ImgBlendX * ImgBlendY;
	while (ImgBlendPixelCount < meshVertCount * 8)
	{
		if (ImgBlendX < 1024)
			ImgBlendX *= 2;
		else
			ImgBlendY *= 2;
		ImgBlendPixelCount = ImgBlendX * ImgBlendY;
	}
	imgBufBlend.AddZeroed(ImgBlendPixelCount);
	int PixelCountHasProc = 0;

	// 遍历所有的子网格
	for (int sectionIndex = 0; sectionIndex < skeMeshLodMesh.Sections.Num(); ++sectionIndex)
	{
		const FSkelMeshSection& skelMeshSection = skeMeshLodMesh.Sections[sectionIndex];
		// 子网格的顶点数
		const int vertCount = skelMeshSection.SoftVertices.Num();
		// 遍历子网格所有顶点
		for (int vertexIndex = 0; vertexIndex < vertCount; ++vertexIndex)
		{
			// 顶点的TPOSE坐标
			const FSoftSkinVertex& vert = skelMeshSection.SoftVertices[vertexIndex];
			// 用四个像素分别记录最多8根骨骼的混合信息
			FLinearColor blendInfo01, blendInfo23, blendInfo45, blendInfo67;

			blendInfo01.R = (float)skelMeshSection.BoneMap[vert.InfluenceBones[0]];
			blendInfo01.G = (float)vert.InfluenceWeights[0];
			blendInfo01.B = (float)skelMeshSection.BoneMap[vert.InfluenceBones[1]];
			blendInfo01.A = (float)vert.InfluenceWeights[1];

			blendInfo23.R = (float)skelMeshSection.BoneMap[vert.InfluenceBones[2]];
			blendInfo23.G = (float)vert.InfluenceWeights[2];
			blendInfo23.B = (float)skelMeshSection.BoneMap[vert.InfluenceBones[3]];
			blendInfo23.A = (float)vert.InfluenceWeights[3];

			blendInfo45.R = (float)skelMeshSection.BoneMap[vert.InfluenceBones[4]];
			blendInfo45.G = (float)vert.InfluenceWeights[4];
			blendInfo45.B = (float)skelMeshSection.BoneMap[vert.InfluenceBones[5]];
			blendInfo45.A = (float)vert.InfluenceWeights[5];

			blendInfo67.R = (float)skelMeshSection.BoneMap[vert.InfluenceBones[6]];
			blendInfo67.G = (float)vert.InfluenceWeights[6];
			blendInfo67.B = (float)skelMeshSection.BoneMap[vert.InfluenceBones[7]];
			blendInfo67.A = (float)vert.InfluenceWeights[7];

			// 先保存这个顶点的TPose位置信息
			imgBufBlend[PixelCountHasProc++] = FFloat16Color(FLinearColor(vert.Position));
			// 保存这个顶点被最多8跟骨骼影响的混合信息
			imgBufBlend[PixelCountHasProc++] = FFloat16Color(blendInfo01);
			imgBufBlend[PixelCountHasProc++] = FFloat16Color(blendInfo23);
			imgBufBlend[PixelCountHasProc++] = FFloat16Color(blendInfo45);
			imgBufBlend[PixelCountHasProc++] = FFloat16Color(blendInfo67);
			// 留空3个像素
			PixelCountHasProc += 3;
		}
	}

	// 保存最新的那张GpuSkin纹理
	FString BlendTexFilePath = exportFolderPath;// inputExportFilePath->GetText().ToString();
	BlendTexFilePath.Append(TEXT("_GpuSkinBlendTex"));
	SaveRGBA16FloatTexture(imgBufBlend, BlendTexFilePath, ImgBlendX, ImgBlendY);
}


FReply SExportAnimWidget::OnExportAnimation()
{
	if (checkBakeAsVertexAnimation->IsChecked())
	{
		return ExportAsVertexAnimations();
	}
	else
	{
		return ExportAsGpuSkinAnimations();
	}
	return FReply::Handled();
}

FReply SExportAnimWidget::ExportAsVertexAnimations()
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

	//TArray<FAssetData> Selection;
	//FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	//ContentBrowserModule.Get().GetSelectedAssets(Selection);

	// 遍历选中的资源
	for (int i = 0; i < CurSelectedAnimAssets.Num(); ++i)
	{
		UObject* assest = CurSelectedAnimAssets[i].GetAsset();
		UClass* cls = assest->GetClass();
		
		// 判断是否动画序列资源
		if (assest->GetClass() == UAnimSequence::StaticClass())
		{
			UAnimSequence* animSeq = Cast<UAnimSequence>(assest);
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
					TArray<FMatrix> BoneMatsFinal;
					BoneMatsFinal.AddZeroed(arrayBoneData.Num());
					//TMap<FName, FMatrix> mapBoneMatrix;
					//TMap<int, FMatrix> mapBoneIDMatrix;

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

						BoneMatsFinal[boneIndex] = boneMatrix;
						//mapBoneMatrix.Add(arrayBoneData[boneIndex].boneName, boneMatrix);
						//mapBoneIDMatrix.Add(boneIndex, boneMatrix);
					}
					
					int MaxBlendBoneCount = 0;
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
							int BlendCount = 0;
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
									++BlendCount;
									float blendrate = (float)weight / 255.0f;
									pos += BoneMatsFinal[vertexBoneIndex].TransformPosition(vert.Position) * blendrate;
								}
							}
							if (BlendCount > MaxBlendBoneCount)
							{
								MaxBlendBoneCount = BlendCount;
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
					UE_LOG(LogTemp, Log, TEXT("最大的骨骼混合数是 = %d"), MaxBlendBoneCount);


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

FReply SExportAnimWidget::ExportAsGpuSkinAnimations()
{
	bool HasSaveGpuSkinBlendInfoTex = false;
	const FString TEX_FLAG = TEXT("_GpuSkinTex_");
	FString Name, PackageName, DataAssetPath = inputExportFilePath->GetText().ToString(), Folder;
	DataAssetPath.Append("_GpuSkinAnimCfg");
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
	dataTable->RowStruct = FTableRowGpuSkinAnimData::StaticStruct();
	TArray<FTableRowGpuSkinAnimData> animDatas;

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

	//TArray<FAssetData> Selection;
	//FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	//ContentBrowserModule.Get().GetSelectedAssets(Selection);

	// 遍历选中的资源
	for (int i = 0; i < CurSelectedAnimAssets.Num(); ++i)
	{
		UObject* assest = CurSelectedAnimAssets[i].GetAsset();
		UClass* cls = assest->GetClass();

		// 判断是否动画序列资源
		if (assest->GetClass() == UAnimSequence::StaticClass())
		{
			UAnimSequence* animSeq = Cast<UAnimSequence>(assest);
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

				// 获取骨骼数
				int BoneCount = (float)(animSeq->GetSkeleton()->GetReferenceSkeleton().GetRawBoneNum());
				// 计算记录一个动画帧所有骨骼矩阵信息所需要的像素数量。 一个骨骼矩阵需要4个像素（每个像素记录矩阵的一行数据）
				int PixelsPerFrame = BoneCount * 4;

				// 获取动画帧数
				int frameCountOfCurAnimSequence = animSeq->GetNumberOfFrames();
				// 计算存储一帧的动画数据需要多少行像素
				const int NeedRowsPerFrame = FMath::CeilToInt((float)PixelsPerFrame / SizeX);
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
					GenerateFilePath.Append(TEX_FLAG).AppendInt(ImageCount);
					SaveRGBA16FloatTexture(imgBuf, GenerateFilePath, SizeX, SizeY);

					imgBuf.Reset(PixelCount);
					imgBuf.AddZeroed(PixelCount);
					frameCountHasRecord = 0;
					++ImageCount;
				}

				// 记录动画序列的信息
				FTableRowGpuSkinAnimData dataRow;
				dataRow.AnimName = animSeq->GetName();
				dataRow.AnimTimeLength = animSeq->GetTimeAtFrame(frameCountOfCurAnimSequence - 1);
				dataRow.FrameCount = frameCountOfCurAnimSequence;
				dataRow.StartFrameRow = frameCountHasRecord * NeedRowsPerFrame;
				dataRow.EndFrameRow = dataRow.StartFrameRow + NeedRowsPerFrame * frameCountOfCurAnimSequence - 1;
				dataRow.AnimTexPath = inputExportFilePath->GetText().ToString();
				dataRow.AnimTexPath.Append(TEX_FLAG).AppendInt(ImageCount);
				dataRow.PixelRowsPerFrame = NeedRowsPerFrame;
				dataRow.TexIndex = ImageCount;
				animDatas.Add(dataRow);

				// 遍历所有的动画帧
				for (int frameID = 0; frameID < frameCountOfCurAnimSequence; ++frameID)
				{
					TArray<FMatrix> boneMatrixes;
					boneMatrixes.AddZeroed(BoneCount);

					// 获取这一帧所对应的时间
					float testTime = animSeq->GetTimeAtFrame(frameID);
					// 遍历所有的骨骼，计算每根骨骼的最终矩阵
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
						boneMatrixes[boneIndex] = boneMatrix;
					}

					// 计算把这一帧的骨骼矩阵数据往imgBuf里写入的起始索引值
					int StartPixelIndex = frameCountHasRecord * NeedRowsPerFrame * SizeX;
					// 遍历所有的骨骼矩阵，按一个矩阵占用4个像素的规则往imgBuf里写入数据
					for (int boneIndex = 0; boneIndex < boneMatrixes.Num(); ++boneIndex)
					{
						const FMatrix& mat = boneMatrixes[boneIndex];
						float flt3x4[12];
						// 转成3X4的矩阵
						boneMatrixes[boneIndex].To3x4MatrixTranspose(flt3x4);
						imgBuf[StartPixelIndex++] = FFloat16Color(FLinearColor(flt3x4[0], flt3x4[1], flt3x4[2], flt3x4[3]));
						imgBuf[StartPixelIndex++] = FFloat16Color(FLinearColor(flt3x4[4], flt3x4[5], flt3x4[6], flt3x4[7]));
						imgBuf[StartPixelIndex++] = FFloat16Color(FLinearColor(flt3x4[8], flt3x4[9], flt3x4[10], flt3x4[11]));
						imgBuf[StartPixelIndex++] = FFloat16Color(FLinearColor());
					}
					// 记录的总动画帧数累加
					++frameCountHasRecord;

					// 保存顶点混合信息到单独的纹理中
					if (HasSaveGpuSkinBlendInfoTex == false)
					{
						SaveGpuSkinBlendInfoToTexture(checkBoneBlendInfo.Get()->IsChecked(), skeMeshLodMesh, meshVertCount, inputExportFilePath->GetText().ToString());

						HasSaveGpuSkinBlendInfoTex = true;
					}
				}
			}
		}
	}

	// 保存最新的那张GpuSkin纹理
	FString GenerateFilePath = inputExportFilePath->GetText().ToString();
	GenerateFilePath.Append(TEX_FLAG).AppendInt(ImageCount);
	SaveRGBA16FloatTexture(imgBuf, GenerateFilePath, SizeX, SizeY);

	// 读取数据表里引用的纹理
	for (int i = 0; i < animDatas.Num(); ++i)
	{
		FTableRowGpuSkinAnimData& data = animDatas[i];
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
						+ SHorizontalBox::Slot()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("是否烘焙成顶点动画")))
							]
						+ SHorizontalBox::Slot()
							[
								SAssignNew(checkBakeAsVertexAnimation, SCheckBox)
								.IsChecked(true)
							]
					]
				+ SVerticalBox::Slot()
					.MaxHeight(ROW_HEIGHT)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							[
								SNew(STextBlock)
								.Text(FText::FromName(TEXT("是否自动刷新当前选中的动画列表")))
							]
						+ SHorizontalBox::Slot()
							[
								SAssignNew(checkAutoRefreshAnimList, SCheckBox)
								.IsChecked(true)
							]
					]
				+ SVerticalBox::Slot()
					.MaxHeight(ROW_HEIGHT)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						[
							SNew(STextBlock)
							.Text(FText::FromName(TEXT("是否检查顶点混合信息")))
						]
						+ SHorizontalBox::Slot()
						[
							SAssignNew(checkBoneBlendInfo, SCheckBox)
							.IsChecked(true)
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
					.Text(FText::FromString(TEXT("烘培选中的动画")))
					.OnClicked(this, &SExportAnimWidget::OnExportAnimation)
					]

				//+ SVerticalBox::Slot()
				//	.MaxHeight(ROW_HEIGHT)
				//	[
				//		SNew(SButton)
				//		.HAlign(HAlign_Center)
				//		.Text(FText::FromString(TEXT("导出选中的动画到浮点纹理")))
				//		.OnClicked(this, &SExportAnimWidget::OnExportAnimationToFloatTextureClick)
				//	]

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

	if (checkAutoRefreshAnimList.Get()->IsChecked() == false)
		return;

	CurSelectedAnimAssets.Empty();

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

			CurSelectedAnimAssets.Add(Selection[i]);
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