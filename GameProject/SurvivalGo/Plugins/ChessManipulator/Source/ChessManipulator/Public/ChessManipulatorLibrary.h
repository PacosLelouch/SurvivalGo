// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/PropertyAccessUtil.h"
#include "LatentActions.h"
#include "Engine/LatentActionManager.h"
#include "ChessManipulatorLibrary.generated.h"

class USceneComponent;

UENUM(BlueprintType)
enum class EMoveCompleteType : uint8
{
	IntermediateCompleted UMETA(DisplayName = "IntermediateCompleted"),
	AllCompleted UMETA(DisplayName = "AllCompleted"),
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FActionExtraDelegate, USceneComponent*, ComponentMoved);

UCLASS(BlueprintType)
class CHESSMANIPULATOR_API UChessManipulatorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "ChessManipulator|Movement", meta = (Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "MoveAction", OverTimeLocation = "0.2", OverTimeLocation = "0.2"))
	static void MoveComponentInWorldSpace(
		USceneComponent* Component, FVector TargetWorldLocation, FRotator TargetWorldRotation, 
		bool bEaseOut, bool bEaseIn, float OverTimeLocation, float OverTimeRotation,
		bool bSweep, ETeleportType TeleportType, 
		bool bForceShortestRotationPath, TEnumAsByte<EMoveComponentAction::Type> MoveAction, FLatentActionInfo LatentInfo,
		FActionExtraDelegate ExtraDelegate);

	UFUNCTION(BlueprintCallable, Category = "ChessManipulator|Movement", meta = (Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Completed", OverTimeLocation = "0.2", OverTimeLocation = "0.2"))
	static void MoveComponentInWorldSpaceBatch(
		USceneComponent* Component, const TArray<FTransform>& TargetTransforms,
		bool bEaseOut, bool bEaseIn, float OverTimeLocationPerTransform, float OverTimeRotationPerTransform,
		bool bSweep, ETeleportType TeleportType,
		bool bForceShortestRotationPath, EMoveCompleteType& Completed, FLatentActionInfo LatentInfo,
		FActionExtraDelegate ExtraIntermediateDelegate, FActionExtraDelegate ExtraDoneDelegate);

};
