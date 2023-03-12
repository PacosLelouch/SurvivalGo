// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "ChessMovementComponent.h"
#include "ChessManipulatorLibrary.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

void ULatentInfoObject::DummyActionInfoCallback()
{

}

void ULatentInfoObject::BatchMovementActionInfoCallback()
{
	if (TempTransformIndex == TempTransforms.Num() || !IsValid(ChessMovementComponent))
	{
		return;
	}
	const FTransform& ToTransform = TempTransforms[TempTransformIndex++];
	bool bLast = TempTransformIndex == TempTransforms.Num();
	if (CompletePtr)
	{
		*CompletePtr = bLast ? EMoveCompleteType::AllCompleted : EMoveCompleteType::IntermediateCompleted;
	}
	UChessManipulatorLibrary::MoveComponentInWorldSpace(
		TempMoveComponent, ToTransform.GetLocation(), ToTransform.Rotator(),
		ChessMovementComponent->bMovementEaseOut, ChessMovementComponent->bMovementEaseIn, 
		ChessMovementComponent->OverTimeLocationPerTransform, ChessMovementComponent->OverTimeRotationPerTransform,
		ChessMovementComponent->bMovementSweep, ChessMovementComponent->MovementTeleportType, bForceShortestRotationPath, MoveAction,
		//bLast ? TempCompletedActionInfo : ChessMovementComponent->BatchMovementActionInfo,
		TempCompletedActionInfo, 
		bLast ? ChessMovementComponent->ExtraDoneActionDelegate : ChessMovementComponent->ExtraIntermediateActionDelegate);
}

void ULatentInfoObject::BatchMovementActionInfoCallbackWithComponentMoved(USceneComponent* ComponentMoved)
{
	BatchMovementActionInfoCallback();
}

void ULatentInfoObject::Reset()
{
	TempTransforms.Empty();
	TempTransformIndex = 0;
	TempCompletedActionInfo = FLatentActionInfo();
	TempMoveComponent = nullptr;
	CompletePtr = nullptr;
	ChessMovementComponent = nullptr;
}

int32 ULatentInfoObject::GetNewUUID()
{
	static int32 InternalUUID = 0;
	return ++InternalUUID;
}

UChessMovementComponent::UChessMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//LatentInfoObject = CreateDefaultSubobject<ULatentInfoObject>(TEXT("LatentInfoObject"));
}

void UChessMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	//LatentInfoObject = NewObject<ULatentInfoObject>(this);
	//TimerDelegate.BindUObject(this, &UChessMovementComponent::OnMoveCompleted);
	//LatentInfoObject->ChessMovementComponent = this;
	//DummyActionInfo = FLatentActionInfo(1, ULatentInfoObject::GetNewUUID(), TEXT("DummyActionInfoCallback"), LatentInfoObject);
	//BatchMovementActionInfo = FLatentActionInfo(1, ULatentInfoObject::GetNewUUID(), TEXT("BatchMovementActionInfoCallback"), LatentInfoObject);
	ExtraDoneActionDelegate.BindDynamic(this, &UChessMovementComponent::OnMoveCompletedWithComponentMoved);
	//ExtraIntermediateActionDelegate.BindDynamic(LatentInfoObject, &ULatentInfoObject::BatchMovementActionInfoCallbackWithComponentMoved);
}

void UChessMovementComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	//if (IsValid(LatentInfoObject))
	//{
	//	LatentInfoObject->Reset();
	//}
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UChessMovementComponent::MoveActorBatch(
	const FTransform& FromTransform, const TArray<FTransform>& ToTransforms,
	bool bForceShortestRotationPath, EMoveCompleteType& CompleteType, FLatentActionInfo LatentInfo,
	float OverrideTimeLocationPerTransform, float OverrideTimeRotationPerTransform)
{
	AActor* Actor = GetOwner();
	if (!Actor)
	{
		return;
	}
	USceneComponent* ActorRootComponent = Actor->GetRootComponent();
	if (!ActorRootComponent)
	{
		return;
	}
	//LatentInfoObject->TempTransforms = ToTransforms;
	//LatentInfoObject->TempTransformIndex = 0;
	//LatentInfoObject->TempCompletedActionInfo = LatentInfo;
	//LatentInfoObject->TempMoveComponent = ActorRootComponent;
	//LatentInfoObject->bForceShortestRotationPath = bForceShortestRotationPath;
	//LatentInfoObject->MoveAction = EMoveComponentAction::Move;
	//LatentInfoObject->CompletePtr = &CompleteType;
	SetIsMoving(true);
	ActorRootComponent->SetWorldTransform(FromTransform);
	UChessManipulatorLibrary::MoveComponentInWorldSpaceBatch(
		ActorRootComponent, ToTransforms,
		bMovementEaseOut, bMovementEaseIn, 
		OverrideTimeLocationPerTransform >= 0.f ? OverrideTimeLocationPerTransform : OverTimeLocationPerTransform,
		OverrideTimeRotationPerTransform >= 0.f ? OverrideTimeRotationPerTransform : OverTimeRotationPerTransform,
		bMovementSweep, MovementTeleportType, bForceShortestRotationPath, CompleteType,
		LatentInfo, ExtraIntermediateActionDelegate, ExtraDoneActionDelegate);
	//LatentInfoObject->BatchMovementActionInfoCallback();
	//FTimerManager& TimerManager = Actor->GetWorldTimerManager();
	//float WaitTime = FMath::Max(OverTimeLocationPerTransform, OverTimeRotationPerTransform) * ToTransforms.Num();
	//TimerManager.SetTimer(TimerHandle, TimerDelegate, WaitTime, false, WaitTime);
}

void UChessMovementComponent::MoveActorOnce(
	const FTransform& FromTransform, const FTransform& ToTransform, 
	bool bForceShortestRotationPath, FLatentActionInfo LatentInfo, 
	float OverrideTimeLocation, float OverrideTimeRotation)
{
	AActor* Actor = GetOwner();
	if (!Actor)
	{
		return;
	}
	USceneComponent* ActorRootComponent = Actor->GetRootComponent();
	if (!ActorRootComponent)
	{
		return;
	}
	SetIsMoving(true);
	ActorRootComponent->SetWorldTransform(FromTransform);
	UChessManipulatorLibrary::MoveComponentInWorldSpace(
		ActorRootComponent, ToTransform.GetLocation(), ToTransform.Rotator(),
		bMovementEaseOut, bMovementEaseIn,
		OverrideTimeLocation >= 0.f ? OverrideTimeLocation : OverTimeLocationPerTransform,
		OverrideTimeRotation >= 0.f ? OverrideTimeRotation : OverTimeRotationPerTransform,
		bMovementSweep, MovementTeleportType, bForceShortestRotationPath, EMoveComponentAction::Move, 
		LatentInfo, ExtraDoneActionDelegate);
	//FTimerManager& TimerManager = Actor->GetWorldTimerManager();
	//float WaitTime = FMath::Max(OverTimeLocationPerTransform, OverTimeRotationPerTransform);
	//TimerManager.SetTimer(TimerHandle, TimerDelegate, WaitTime, false, WaitTime);
}

USceneComponent* UChessMovementComponent::GetActorRootComponent()
{
	AActor* Actor = GetOwner();
	if (!Actor)
	{
		return nullptr;
	}
	return Actor->GetRootComponent();
}

void UChessMovementComponent::SetIsMoving(bool bValue)
{
	bIsMoving = bValue;
}

void UChessMovementComponent::OnMoveCompleted()
{
	SetIsMoving(false);
}

void UChessMovementComponent::OnMoveCompletedWithComponentMoved_Implementation(USceneComponent* Caller)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	FTimerHandle TempHandle;
	World->GetTimerManager().SetTimer(TempHandle, [this]()
		{
			SetIsMoving(false);
		}, 0.1, false, 0.1);
	//SetIsMoving(false);
}
