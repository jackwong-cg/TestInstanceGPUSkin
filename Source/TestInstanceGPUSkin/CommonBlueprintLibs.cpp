#include "CommonBlueprintLibs.h"
#include "Runtime/Core/Public/Stats/Stats.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

DECLARE_STATS_GROUP(TEXT("UCommonBlueprintLibs"), STATGROUP_UCommonBlueprintLibs, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("UCommonBlueprintLibs NavFindPath"), STAT_UCommonBlueprintLibs_NavFindPath, STATGROUP_UCommonBlueprintLibs);
DECLARE_CYCLE_STAT(TEXT("UCommonBlueprintLibs PresureTestNavFindPathNear"), STAT_UCommonBlueprintLibs_PresureTestNavFindPathNear, STATGROUP_UCommonBlueprintLibs);
DECLARE_CYCLE_STAT(TEXT("UCommonBlueprintLibs PresureTestNavFindPathMid"), STAT_UCommonBlueprintLibs_PresureTestNavFindPathMid, STATGROUP_UCommonBlueprintLibs);
DECLARE_CYCLE_STAT(TEXT("UCommonBlueprintLibs PresureTestNavFindPathFar"), STAT_UCommonBlueprintLibs_PresureTestNavFindPathFar, STATGROUP_UCommonBlueprintLibs);

//DEFINE_STAT(STAT_UCommonBlueprintLibs_NavFindPath);
//DEFINE_STAT(STAT_UCommonBlueprintLibs_PresureTestNavFindPathNear);
//DEFINE_STAT(STAT_UCommonBlueprintLibs_PresureTestNavFindPathMid);
//DEFINE_STAT(STAT_UCommonBlueprintLibs_PresureTestNavFindPathFar);

TArray<FVector> UCommonBlueprintLibs::NavFindPath(FVector start, FVector end)
{
	SCOPE_CYCLE_COUNTER(STAT_UCommonBlueprintLibs_NavFindPath);
	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GEngine->GetWorld());
	UNavigationPath* outPath = navSys->FindPathToLocationSynchronously(GEngine->GetWorld(), start, end);
	return outPath->PathPoints;
}

void UCommonBlueprintLibs::PresureTestNavFindPathNear(int TestCount, FVector start, FVector end, float searchRadius)
{
	SCOPE_CYCLE_COUNTER(STAT_UCommonBlueprintLibs_PresureTestNavFindPathNear);
	for (int i = 0; i < TestCount; ++i)
	{
		FVector v0 = start + FVector(FMath::RandRange(-searchRadius, searchRadius), FMath::RandRange(-searchRadius, searchRadius), 0);
		FVector v1 = end + FVector(FMath::RandRange(-searchRadius, searchRadius), FMath::RandRange(-searchRadius, searchRadius), 0);
		NavFindPath(v0, v1);
	}
}

void UCommonBlueprintLibs::PresureTestNavFindPathMid(int TestCount, FVector start, FVector end, float searchRadius)
{
	SCOPE_CYCLE_COUNTER(STAT_UCommonBlueprintLibs_PresureTestNavFindPathMid);
	for (int i = 0; i < TestCount; ++i)
	{
		FVector v0 = start + FVector(FMath::RandRange(-searchRadius, searchRadius), FMath::RandRange(-searchRadius, searchRadius), 0);
		FVector v1 = end + FVector(FMath::RandRange(-searchRadius, searchRadius), FMath::RandRange(-searchRadius, searchRadius), 0);
		NavFindPath(v0, v1);
	}
}

void UCommonBlueprintLibs::PresureTestNavFindPathFar(int TestCount, FVector start, FVector end, float searchRadius)
{
	SCOPE_CYCLE_COUNTER(STAT_UCommonBlueprintLibs_PresureTestNavFindPathFar);
	for (int i = 0; i < TestCount; ++i)
	{
		FVector v0 = start + FVector(FMath::RandRange(-searchRadius, searchRadius), FMath::RandRange(-searchRadius, searchRadius), 0);
		FVector v1 = end + FVector(FMath::RandRange(-searchRadius, searchRadius), FMath::RandRange(-searchRadius, searchRadius), 0);
		NavFindPath(v0, v1);
	}
}