// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Engine/LatentActionManager.h"
#include "LatentActions.h"
#include "ChessManipulatorLibrary.h"
//#include "Kismet/KismetSystemLibrary.h"
//#include "InterpolateMovementAction.generated.h"

class USceneComponent;
class FActionExtraDelegate;

struct FLatentInfoWeak
{
	/** Function to execute on completion */
	FName ExecutionFunction;
	/** Link to fire on completion */
	int32 OutputLink;
	/** Object to call callback on upon completion */
	FWeakObjectPtr CallbackTarget;
};

//DECLARE_DYNAMIC_DELEGATE_OneParam(FActionExtraDelegate, USceneComponent*, Caller);

/** Action that interpolates a component over time to a desired world position */
class CHESSMANIPULATOR_API FInterpolateMovementAction : public FPendingLatentAction
{
public:
	float TotalTimeLocation, TotalTimeRotation;

protected:
	float LocalTotalTime = 0.f;
	/** Time over which interpolation should happen */
	float TotalTime = 0.f;

public:
	float LocalTimeElapsed = 0.f;
	/** Time so far elapsed for the interpolation */
	float TimeElapsed = 0.f;
	/** If we are currently interpolating. If false, update will complete */
	bool bInterpolating;

	FLatentInfoWeak LatentInfoWeak;

	/** Component to interpolate */
	TWeakObjectPtr<USceneComponent> TargetComponent;

	/** If we should modify rotation */
	bool bInterpRotation;
	///** Rotation to interpolate from */
	//FRotator InitialRotation;
	///** Rotation to interpolate to */
	//FRotator TargetRotation;

	/** If we should modify location */
	bool bInterpLocation;
	///** Location to interpolate from */
	//FVector InitialLocation;
	///** Location to interpolate to */
	//FVector TargetLocation;

	TArray<FTransform> TransformArray;
	int32 CurrentTransformPhase = 0;
	EMoveCompleteType* MoveCompleteTypePtr = nullptr;

	/** Should we ease in (ie start slowly) during interpolation */
	bool bEaseIn;
	/** Should we east out (ie end slowly) during interpolation */
	bool bEaseOut;

	/** Force use of shortest Path for rotation **/
	bool bForceShortestRotationPath;

	/** Blend exponent */
	float BlendExp = 2.f;

	bool bSweep = false;
	ETeleportType TeleportType = ETeleportType::None;
	TSharedPtr<FActionExtraDelegate> IntermediateDelegatePtr = nullptr;
	TSharedPtr<FActionExtraDelegate> DoneDelegatePtr = nullptr;

	FInterpolateMovementAction(
		float DurationLocation, float DurationRotation, FLatentActionInfo LatentInfo, USceneComponent* Component,
		bool bInEaseOut, bool bInEaseIn, bool bInForceShortestRotationPath,
		float InBlendExp = 2.f, bool bInSweep = false, ETeleportType InTeleportType = ETeleportType::None,
		bool bInInterpLocation = true, bool bInInterpRotation = true);

	virtual void UpdateOperation(FLatentResponse& Response) override;

	void InitializeTotalTime();

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		static const FNumberFormattingOptions DelayTimeFormatOptions = FNumberFormattingOptions()
			.SetMinimumFractionalDigits(3)
			.SetMaximumFractionalDigits(3);
		return FText::Format(NSLOCTEXT("FInterpolateMovementAction", "ActionTimeFmt", "Move ({0} seconds left)"), FText::AsNumber(TotalTime - TimeElapsed, &DelayTimeFormatOptions)).ToString();
	}
#endif
};
