// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ChessManipulatorLibrary.h"
#include "GameFramework/MovementComponent.h"
#include "Engine/LatentActionManager.h"
#include "LatentActions.h"
#include "ChessMovementComponent.generated.h"

class USceneComponent;
class UChessMovementComponent;

UCLASS()
class CHESSMANIPULATOR_API ULatentInfoObject : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void DummyActionInfoCallback();

	UFUNCTION()
	void BatchMovementActionInfoCallback();

	UFUNCTION()
	void BatchMovementActionInfoCallbackWithComponentMoved(USceneComponent* ComponentMoved);

	void Reset();

	static int32 GetNewUUID();

	TArray<FTransform> TempTransforms;
	int32 TempTransformIndex = 0;
	FLatentActionInfo TempCompletedActionInfo;
	USceneComponent* TempMoveComponent = nullptr;
	bool bForceShortestRotationPath = true;
	TEnumAsByte<EMoveComponentAction::Type> MoveAction = EMoveComponentAction::Move;
	EMoveCompleteType* CompletePtr = nullptr;
	UChessMovementComponent* ChessMovementComponent = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class CHESSMANIPULATOR_API UChessMovementComponent : public UActorComponent//UMovementComponent
{
	GENERATED_BODY()
public:
	friend class ULatentInfoObject;
	//DECLARE_DYNAMIC_DELEGATE(FOnChessMovementCompleted);

public:
	UChessMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	UFUNCTION(BlueprintCallable, Category = "ChessManipulator|Movement", meta = (Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "CompleteType", bForceShortestRotationPath = "true"))
	void MoveActorBatch(const FTransform& FromTransform, const TArray<FTransform>& ToTransforms, 
		bool bForceShortestRotationPath, EMoveCompleteType& CompleteType, FLatentActionInfo LatentInfo,
		float OverrideTimeLocationPerTransform = -1.f, float OverrideTimeRotationPerTransform = -1.f);

	UFUNCTION(BlueprintCallable, Category = "ChessManipulator|Movement", meta = (Latent, LatentInfo = "LatentInfo", bForceShortestRotationPath = "true"))
	void MoveActorOnce(const FTransform& FromTransform, const FTransform& ToTransform,
		bool bForceShortestRotationPath, FLatentActionInfo LatentInfo, 
		float OverrideTimeLocation = -1.f, float OverrideTimeRotation = -1.f);

	UFUNCTION(BlueprintPure, Category = "ChessManipulator|Movement")
	USceneComponent* GetActorRootComponent();

	UFUNCTION(BlueprintCallable, Category = "ChessManipulator|Movement")
	void SetIsMoving(bool bValue);

	UFUNCTION()
	void OnMoveCompleted();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ChessManipulator|Movement")
	void OnMoveCompletedWithComponentMoved(USceneComponent* ComponentMoved);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessManipulator|Movement")
	float OverTimeLocationPerTransform = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessManipulator|Movement")
	float OverTimeRotationPerTransform = 0.125f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessManipulator|Movement")
	bool bMovementEaseIn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessManipulator|Movement")
	bool bMovementEaseOut = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessManipulator|Movement")
	bool bMovementSweep = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChessManipulator|Movement")
	ETeleportType MovementTeleportType = ETeleportType::TeleportPhysics;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, BlueprintSetter = SetIsMoving, Category = "ChessManipulator|Movement")
	bool bIsMoving = false;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "ChessManipulator|Movement")
	ULatentInfoObject* LatentInfoObject = nullptr;

protected:
	FLatentActionInfo DummyActionInfo, BatchMovementActionInfo;

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;

	FActionExtraDelegate ExtraIntermediateActionDelegate, ExtraDoneActionDelegate;

	//FOnChessMovementCompleted OnChessMovementCompletedDelegate;
};