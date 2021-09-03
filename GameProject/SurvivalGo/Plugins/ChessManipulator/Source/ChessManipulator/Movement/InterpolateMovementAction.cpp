// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "InterpolateMovementAction.h"
#include "Components/SceneComponent.h"
#include "ChessManipulatorLibrary.h"

FInterpolateMovementAction::FInterpolateMovementAction(
	float DurationLocation, float DurationRotation, FLatentActionInfo LatentInfo, USceneComponent* Component,
	bool bInEaseOut, bool bInEaseIn, bool bInForceShortestRotationPath,
	float InBlendExp, bool bInSweep, ETeleportType InTeleportType,
	bool bInInterpLocation, bool bInInterpRotation)
	: TotalTimeLocation(DurationLocation)
	, TotalTimeRotation(DurationRotation)
	, bInterpolating(true)
	, LatentInfoWeak({ LatentInfo.ExecutionFunction, LatentInfo.Linkage, LatentInfo.CallbackTarget })
	, TargetComponent(Component)
	, bInterpRotation(true)
	//, InitialRotation(FRotator::ZeroRotator)
	//, TargetRotation(FRotator::ZeroRotator)
	, bInterpLocation(true)
	//, InitialLocation(FVector::ZeroVector)
	//, TargetLocation(FVector::ZeroVector)
	, bEaseIn(bInEaseIn)
	, bEaseOut(bInEaseOut)
	, bForceShortestRotationPath(bInForceShortestRotationPath)
	, BlendExp(InBlendExp)
	, bSweep(bInSweep)
	, TeleportType(InTeleportType)
{
	//for (const FLatentActionInfo& ActionInfo : LatentInfos)
	//{
	//	LatentInfoWeaks.Add(FLatentInfoWeak{ ActionInfo.ExecutionFunction, ActionInfo.Linkage, ActionInfo.CallbackTarget });
	//}
	InitializeTotalTime();
}

void FInterpolateMovementAction::UpdateOperation(FLatentResponse& Response)
{
	if (TransformArray.Num() < 2)
	{
		Response.FinishAndTriggerIf(true,
			LatentInfoWeak.ExecutionFunction, LatentInfoWeak.OutputLink, LatentInfoWeak.CallbackTarget);
		return;
	}

	// Update elapsed time
	LocalTimeElapsed += Response.ElapsedTime();
	TimeElapsed += Response.ElapsedTime();

	int32 PrevTransformPhase = CurrentTransformPhase;
	while (LocalTimeElapsed >= LocalTotalTime && CurrentTransformPhase + 1 < TransformArray.Num())
	{
		++CurrentTransformPhase;
		LocalTimeElapsed -= LocalTotalTime;
	}

	bool bComplete = (TimeElapsed >= TotalTime);

	// If we have a component to modify..
	if (TargetComponent.IsValid() && bInterpolating)
	{
		FTransform TargetTransform = TargetComponent->GetComponentTransform();
		float DurationPctLocation = FMath::IsNearlyZero(TotalTimeLocation) ? 0.5f : 
			FMath::Clamp(LocalTimeElapsed / TotalTimeLocation, 0.f, 1.f);
		float DurationPctRotation = FMath::IsNearlyZero(TotalTimeRotation) ? 0.5f : 
			FMath::Clamp(LocalTimeElapsed / TotalTimeRotation, 0.f, 1.f);
		float BlendPctLocation, BlendPctRotation;
		if (bEaseIn)
		{
			if (bEaseOut)
			{
				// EASE IN/OUT
				BlendPctLocation = FMath::InterpEaseInOut(0.f, 1.f, DurationPctLocation, BlendExp);
				BlendPctRotation = FMath::InterpEaseInOut(0.f, 1.f, DurationPctRotation, BlendExp);
			}
			else
			{
				// EASE IN
				BlendPctLocation = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPctLocation, BlendExp));
				BlendPctRotation = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPctRotation, BlendExp));
			}
		}
		else
		{
			if (bEaseOut)
			{
				// EASE OUT
				BlendPctLocation = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPctLocation, 1.f / BlendExp));
				BlendPctRotation = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPctRotation, 1.f / BlendExp));
			}
			else
			{
				// LINEAR
				BlendPctLocation = FMath::Lerp(0.f, 1.f, DurationPctLocation);
				BlendPctRotation = FMath::Lerp(0.f, 1.f, DurationPctRotation);
			}
		}

		const FVector& InitialLocation = TransformArray[CurrentTransformPhase].GetLocation();
		const FRotator& InitialRotation = TransformArray[CurrentTransformPhase].Rotator();
		const FVector& TargetLocation = TransformArray[FMath::Min(CurrentTransformPhase + 1, TransformArray.Num() - 1)].GetLocation();
		const FRotator& TargetRotation = TransformArray[FMath::Min(CurrentTransformPhase + 1, TransformArray.Num() - 1)].Rotator();

		// Update location
		if (bInterpLocation)
		{
			FVector NewLocation = bComplete ? TargetLocation : FMath::Lerp(InitialLocation, TargetLocation, BlendPctLocation);
			//TargetComponent->SetWorldLocation(NewLocation, bSweep, nullptr, TeleportType);
			TargetTransform.SetLocation(NewLocation);
		}

		if (bInterpRotation && TargetComponent.IsValid())
		{
			FRotator NewRotation;
			// If we are done just set the final rotation
			if (bComplete)
			{
				NewRotation = TargetRotation;
			}
			else if (bForceShortestRotationPath)
			{
				// We want the shortest path 
				FQuat AQuat(InitialRotation);
				FQuat BQuat(TargetRotation);

				FQuat Result = FQuat::Slerp(AQuat, BQuat, BlendPctRotation);
				Result.Normalize();
				NewRotation = Result.Rotator();
			}
			else
			{
				// dont care about it being the shortest path - just lerp
				NewRotation = FMath::Lerp(InitialRotation, TargetRotation, BlendPctRotation);
			}
			//TargetComponent->SetWorldRotation(NewRotation, bSweep, nullptr, TeleportType);
			TargetTransform.SetRotation(NewRotation.Quaternion());
		}
		TargetComponent->SetWorldTransform(TargetTransform, bSweep, nullptr, TeleportType);
	}

	//for (int32 i = 0; i < LatentInfoWeaks.Num(); ++i)
	//{
	//	//if (i + 1 == LatentInfoWeaks.Num())
	//	//{
	//		Response.FinishAndTriggerIf(bComplete || !bInterpolating, 
	//			LatentInfoWeaks[i].ExecutionFunction, LatentInfoWeaks[i].OutputLink, LatentInfoWeaks[i].CallbackTarget);
	//	//}
	//	//else
	//	//{
	//	//	Response.TriggerLink(bComplete || !bInterpolating,
	//	//		LatentInfoWeaks[i].ExecutionFunction, LatentInfoWeaks[i].OutputLink, LatentInfoWeaks[i].CallbackTarget);
	//	//}
	//}
	if (bComplete || !bInterpolating)
	{
		if (MoveCompleteTypePtr)
		{
			*MoveCompleteTypePtr = EMoveCompleteType::AllCompleted;
		}
		Response.FinishAndTriggerIf(true,
			LatentInfoWeak.ExecutionFunction, LatentInfoWeak.OutputLink, LatentInfoWeak.CallbackTarget);
		if (DoneDelegatePtr && DoneDelegatePtr->IsBound())
		{
			DoneDelegatePtr->Execute(TargetComponent.Get());
		}
	}
	else if (PrevTransformPhase != CurrentTransformPhase && CurrentTransformPhase + 1 < TransformArray.Num())
	{
		if (MoveCompleteTypePtr)
		{
			*MoveCompleteTypePtr = EMoveCompleteType::IntermediateCompleted;
		}
		Response.TriggerLink(
			LatentInfoWeak.ExecutionFunction, LatentInfoWeak.OutputLink, LatentInfoWeak.CallbackTarget);
		if (IntermediateDelegatePtr && IntermediateDelegatePtr->IsBound())
		{
			IntermediateDelegatePtr->Execute(TargetComponent.Get());
		}
	}
}

void FInterpolateMovementAction::InitializeTotalTime()
{
	LocalTotalTime = FMath::Max(bInterpLocation ? TotalTimeLocation : 0.f, bInterpRotation ? TotalTimeRotation : 0.f);
	TotalTime = LocalTotalTime * FMath::Max(1, TransformArray.Num() - 1);
}
