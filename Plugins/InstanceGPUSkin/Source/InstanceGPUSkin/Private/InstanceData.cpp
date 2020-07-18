#include "InstanceData.h"
#include "InstanceSkeletelMeshActor.h"

void FUInstanceData::Tick(float DeltaTime)
{
	AnimTimeCounter += DeltaTime * AnimPlayRate;

	if (ParentActor == nullptr)
		return;
	if (ParentActor->AnimType == EAnimType::EBakeVertexAnimation)
	{
		FTableRowAnimData* rowInfo = &ParentActor->AnimInfoBakeVertex.AnimSeqDatas[AnimSeq];
		while (AnimTimeCounter > rowInfo->AnimTimeLength)
		{
			AnimTimeCounter -= rowInfo->AnimTimeLength;
		}
		float rate = AnimTimeCounter / rowInfo->AnimTimeLength;
		TexIndex = rowInfo->TexIndex;
		CurAnimRowStart = (int)((float)rowInfo->FrameCount * rate) * rowInfo->PixelRowsPerFrame + rowInfo->StartFrameRow;

	}
	else
	{
		FTableRowGpuSkinAnimData* rowInfo = &ParentActor->AnimInfoGpuSkin.AnimSeqDatas[AnimSeq];
		while (AnimTimeCounter > rowInfo->AnimTimeLength)
		{
			AnimTimeCounter -= rowInfo->AnimTimeLength;
		}
		float rate = AnimTimeCounter / rowInfo->AnimTimeLength;
		TexIndex = rowInfo->TexIndex;
		CurAnimRowStart = (int)((float)rowInfo->FrameCount * rate) * rowInfo->PixelRowsPerFrame + rowInfo->StartFrameRow;
	}

}