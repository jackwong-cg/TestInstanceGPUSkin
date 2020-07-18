#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "CommonBlueprintLibs.generated.h"

UCLASS()
class TESTINSTANCEGPUSKIN_API UCommonBlueprintLibs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = CommonBlueprintLibs)
		static TArray<FVector> NavFindPath(FVector start, FVector end);

	UFUNCTION(BlueprintCallable, Category = CommonBlueprintLibs)
		static void PresureTestNavFindPathNear(int TestCount, FVector start, FVector end, float searchRadius);

	UFUNCTION(BlueprintCallable, Category = CommonBlueprintLibs)
		static void PresureTestNavFindPathMid(int TestCount, FVector start, FVector end, float searchRadius);

	UFUNCTION(BlueprintCallable, Category = CommonBlueprintLibs)
		static void PresureTestNavFindPathFar(int TestCount, FVector start, FVector end, float searchRadius);

};