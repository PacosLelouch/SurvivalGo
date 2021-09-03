// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MapElementsInterface.generated.h"

UENUM(BlueprintType)
enum class EBlockAdjacentType : uint8
{
	Default,
	Movable,
	Attackable,
};

UINTERFACE(MinimalAPI, BlueprintType)
class UMapBlockInterface : public UInterface
{
	GENERATED_BODY()
};

class CHESSMANIPULATOR_API IMapBlockInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ChessManipulator")
	UClass* GetMapBlockClass();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ChessManipulator")
	bool IsAdjacent(const TScriptInterface<IMapBlockInterface>& Target, EBlockAdjacentType AdjacentType = EBlockAdjacentType::Default);

};

// Interfaces are not supported for key in blueprints.
FORCEINLINE uint32 GetTypeHash(const TScriptInterface<IMapBlockInterface>& MapBlockInterface)
{
	return GetTypeHash(MapBlockInterface.GetObject());
}

UINTERFACE(MinimalAPI, BlueprintType)
class UMapSourceInterface : public UInterface
{
	GENERATED_BODY()
};

class CHESSMANIPULATOR_API IMapSourceInterface
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ChessManipulator")
	bool GetAdjacents(TArray<TScriptInterface<IMapBlockInterface>>& OutTargets, const TScriptInterface<IMapBlockInterface>& Source, EBlockAdjacentType AdjacentType = EBlockAdjacentType::Default, bool bAppend = false);

	// error : UINTERFACEs are not currently supported as key types.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ChessManipulator")
	bool GetAdjacentsWithDistance(TMap<UObject*, int32>& OutTargets, const TScriptInterface<IMapBlockInterface>& Source, int32 MaxDistance = 2, EBlockAdjacentType AdjacentType = EBlockAdjacentType::Default, bool bAppend = false);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ChessManipulator")
	bool GetPath(TArray<TScriptInterface<IMapBlockInterface>>& OutTargets, const TScriptInterface<IMapBlockInterface>& Source, const TScriptInterface<IMapBlockInterface>& Target, EBlockAdjacentType AdjacentType = EBlockAdjacentType::Default);

};
