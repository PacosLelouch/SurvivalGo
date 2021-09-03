// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GameUtils.h"
#include "GoGlobalDefines.h"

FString UGameUtilLibrary::GetGameSavingsBasePath()
{
	return GAME_SAVINGS_BASE_PATH;
}

void UGameUtilLibrary::LerpTime(
	UObject* WorldContextObject,
	ETickLerpInAction InActionType,
	float Duration,
	float InStartValue, float InEndValue,
	int32 InExtraID,
	ETickLerpOutAction& OutActionType,
	float& OutTime, float& OutValue,
	int32& OutExtraID,
	FLatentActionInfo LatentInfo)
{
	OutTime = 0.f;
	OutValue = InStartValue;
	OutExtraID = InExtraID;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FInterpolateTimeAction* Action = LatentActionManager.FindExistingAction<FInterpolateTimeAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
		if (!Action)
		{
			if (InActionType == ETickLerpInAction::Once)
			{
				Action = new FInterpolateTimeAction(Duration, InStartValue, InEndValue, LatentInfo, WorldContextObject);
				Action->OutActionPtr = &OutActionType;
				Action->OutTimePtr = &OutTime;
				Action->OutValuePtr = &OutValue;
				Action->OutExtraIDPtr = &OutExtraID;

				LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
			}
		}
		else
		{
			if (InActionType == ETickLerpInAction::Once)
			{
				Action->TotalTime = Duration;
				Action->StartValue = InStartValue;
				Action->EndValue = InEndValue;
				Action->TimeElapsed = 0.f;
				Action->Caller = WorldContextObject;

				Action->OutActionPtr = &OutActionType;
				Action->OutTimePtr = &OutTime;
				Action->OutValuePtr = &OutValue;
				Action->OutExtraIDPtr = &OutExtraID;
			}
			else if (InActionType == ETickLerpInAction::Stop)
			{
				// 'Stop' just stops the interpolation where it is
				Action->bInterpolating = false;

				Action->OutActionPtr = nullptr;
				Action->OutTimePtr = nullptr;
				Action->OutValuePtr = nullptr;
				Action->OutExtraIDPtr = &OutExtraID;
			}
			else if (InActionType == ETickLerpInAction::Return)
			{
				// Return moves back to the beginning
				Action->TotalTime = Action->TimeElapsed;
				Action->TimeElapsed = 0.f;
				Action->StartValue = *Action->OutValuePtr;
				Action->EndValue = InStartValue;
				Action->Caller = WorldContextObject;

				Action->OutActionPtr = &OutActionType;
				Action->OutTimePtr = &OutTime;
				Action->OutValuePtr = &OutValue;
				Action->OutExtraIDPtr = &OutExtraID;

			}
		}
	}
}

void FInterpolateTimeAction::UpdateOperation(FLatentResponse& Response)
{
	// Update elapsed time
	TimeElapsed += Response.ElapsedTime();

	bool bComplete = (TimeElapsed >= TotalTime);

	// If we have a component to modify..
	if (Caller.IsValid() && bInterpolating)
	{
		float DurationPct = FMath::Clamp(TimeElapsed / TotalTime, 0.f, 1.f);
		
		// Update output
		if (OutTimePtr)
		{
			*OutTimePtr = TimeElapsed;
		}
		if (OutValuePtr)
		{
			*OutValuePtr = FMath::Lerp(StartValue, EndValue, DurationPct);
		}
	}

	if (OutActionPtr)
	{
		switch (*OutActionPtr)
		{
		case ETickLerpOutAction::Tick:
			if (bInterpolating)
			{
				Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
			}
			break;
		case ETickLerpOutAction::Completed:
			Response.FinishAndTriggerIf(bComplete || !bInterpolating, ExecutionFunction, OutputLink, CallbackTarget);
			break;
		}
	}
}
