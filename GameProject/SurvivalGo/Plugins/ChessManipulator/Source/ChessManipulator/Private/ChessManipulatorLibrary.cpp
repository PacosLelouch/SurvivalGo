// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "ChessManipulatorLibrary.h"
#include "../Movement/InterpolateMovementAction.h"
#include "Engine/World.h"

void UChessManipulatorLibrary::MoveComponentInWorldSpace(
	USceneComponent* Component, FVector TargetWorldLocation, FRotator TargetWorldRotation, 
	bool bEaseOut, bool bEaseIn, float OverTimeLocation, float OverTimeRotation, 
	bool bSweep, ETeleportType TeleportType,
	bool bForceShortestRotationPath, TEnumAsByte<EMoveComponentAction::Type> MoveAction, FLatentActionInfo LatentInfo,
	FActionExtraDelegate ExtraDelegate)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(Component, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FInterpolateMovementAction* Action = LatentActionManager.FindExistingAction<FInterpolateMovementAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		const FTransform ComponentTransform = Component ? Component->GetComponentTransform() : FTransform::Identity;
		const FVector ComponentLocation = ComponentTransform.GetLocation();
		const FRotator ComponentRotation = ComponentTransform.GetRotation().Rotator();

		// If not currently running
		bool bMovingLocation = !(ComponentLocation - TargetWorldLocation).IsNearlyZero();
		bool bMovingRotation = !(ComponentRotation - TargetWorldRotation).IsNearlyZero();
		if (!Action)
		{
			if (MoveAction == EMoveComponentAction::Move)
			{
				// Only act on a 'move' input if not running
				Action = new FInterpolateMovementAction(
					OverTimeLocation, OverTimeRotation, LatentInfo, Component, bEaseOut, bEaseIn, bForceShortestRotationPath,
					2.f, bSweep, TeleportType, 
					bMovingLocation, bMovingRotation);

				Action->CurrentTransformPhase = 0;
				Action->DoneDelegatePtr = MakeShareable(new FActionExtraDelegate(ExtraDelegate));

				Action->TransformArray =
				{
					FTransform(ComponentRotation, ComponentLocation),
					FTransform(TargetWorldRotation, TargetWorldLocation),
				};

				Action->InitializeTotalTime();

				LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
			}
		}
		else
		{
			Action->MoveCompleteTypePtr = nullptr;
			Action->bInterpLocation = bMovingLocation;
			Action->bInterpRotation = bMovingRotation;

			Action->bSweep = bSweep;
			Action->TeleportType = TeleportType;

			Action->IntermediateDelegatePtr = nullptr;
			Action->DoneDelegatePtr = MakeShareable(new FActionExtraDelegate(ExtraDelegate));
			Action->CurrentTransformPhase = 0;

			if (MoveAction == EMoveComponentAction::Move)
			{
				// A 'Move' action while moving restarts interpolation

				Action->TotalTimeLocation = OverTimeLocation;
				Action->TotalTimeRotation = OverTimeRotation;
				Action->TimeElapsed = 0.f;

				Action->TransformArray =
				{
					FTransform(ComponentRotation, ComponentLocation),
					FTransform(TargetWorldRotation, TargetWorldLocation),
				};

				Action->InitializeTotalTime();
			}
			else if (MoveAction == EMoveComponentAction::Stop)
			{
				// 'Stop' just stops the interpolation where it is
				Action->bInterpolating = false;
			}
			else if (MoveAction == EMoveComponentAction::Return)
			{
				// Return moves back to the beginning
				Action->TotalTimeLocation = Action->TimeElapsed;
				Action->TotalTimeRotation = Action->TimeElapsed;
				Action->TimeElapsed = 0.f;

				// Set our target to be our initial, and set the new initial to be the current position
				if (Action->TransformArray.Num() > 0)
				{
					Algo::Reverse(Action->TransformArray);
					Action->TransformArray[0] = FTransform(ComponentRotation, ComponentLocation);
				}
			}
		}
	}
}

void UChessManipulatorLibrary::MoveComponentInWorldSpaceBatch(USceneComponent* Component, const TArray<FTransform>& TargetTransforms, bool bEaseOut, bool bEaseIn, float OverTimeLocationPerTransform, float OverTimeRotationPerTransform, bool bSweep, ETeleportType TeleportType, bool bForceShortestRotationPath, EMoveCompleteType& Completed, FLatentActionInfo LatentInfo, FActionExtraDelegate ExtraIntermediateDelegate, FActionExtraDelegate ExtraDoneDelegate)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(Component, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FInterpolateMovementAction* Action = LatentActionManager.FindExistingAction<FInterpolateMovementAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		const FTransform ComponentTransform = Component ? Component->GetComponentTransform() : FTransform::Identity;
		const FVector ComponentLocation = ComponentTransform.GetLocation();
		const FRotator ComponentRotation = ComponentTransform.GetRotation().Rotator();

		// If not currently running
		bool bMovingLocation = true; //!(ComponentLocation - TargetWorldLocation).IsNearlyZero();
		bool bMovingRotation = true; //!(ComponentRotation - TargetWorldRotation).IsNearlyZero();
		if (!Action)
		{
			// Only act on a 'move' input if not running
			Action = new FInterpolateMovementAction(
				OverTimeLocationPerTransform, OverTimeRotationPerTransform, LatentInfo, Component, bEaseOut, bEaseIn, bForceShortestRotationPath,
				2.f, bSweep, TeleportType,
				bMovingLocation, bMovingRotation);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
		Action->MoveCompleteTypePtr = &Completed;
		Action->bInterpLocation = bMovingLocation;
		Action->bInterpRotation = bMovingRotation;

		Action->bSweep = bSweep;
		Action->TeleportType = TeleportType;

		Action->CurrentTransformPhase = 0;
		Action->IntermediateDelegatePtr = MakeShareable(new FActionExtraDelegate(ExtraIntermediateDelegate));
		Action->DoneDelegatePtr = MakeShareable(new FActionExtraDelegate(ExtraDoneDelegate));

			
		// A 'Move' action while moving restarts interpolation
		Action->TotalTimeLocation = OverTimeLocationPerTransform;
		Action->TotalTimeRotation = OverTimeRotationPerTransform;
		Action->TimeElapsed = 0.f;

		Action->TransformArray.Empty(TargetTransforms.Num() + 1);
		Action->TransformArray.Add(ComponentTransform);
		Action->TransformArray.Append(TargetTransforms);

		Action->InitializeTotalTime();
	}
}
