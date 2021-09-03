// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/PropertyAccessUtil.h"
#include "LatentActions.h"
#include "Engine/LatentActionManager.h"
#include "GameUtils.generated.h"


UENUM(BlueprintType)
enum class ETickLerpInAction : uint8
{
	Once,
	Return,
	Stop, 
};

UENUM(BlueprintType)
enum class ETickLerpOutAction : uint8
{
	Tick,
	Completed,
};

class SURVIVALGO_API FInterpolateTimeAction : public FPendingLatentAction //TODO
{

public:

	/** Time over which interpolation should happen */
	float TotalTime = 0.f;
	/** Time so far elapsed for the interpolation */
	float TimeElapsed = 0.f;
	/** If we are currently interpolating. If false, update will complete */
	bool bInterpolating;

	float StartValue, EndValue;

	/** Function to execute on completion */
	FName ExecutionFunction;
	/** Link to fire on completion */
	int32 OutputLink;
	/** Object to call callback on upon completion */
	FWeakObjectPtr CallbackTarget;

	/** Component to interpolate */
	TWeakObjectPtr<UObject> Caller;

	float* OutTimePtr = nullptr;
	float* OutValuePtr = nullptr;
	int32* OutExtraIDPtr = nullptr;
	ETickLerpOutAction* OutActionPtr = nullptr;

	FInterpolateTimeAction(
		float Duration, float InStartValue, float InEndValue, const FLatentActionInfo& LatentInfo, 
		UObject* InCaller)
		: TotalTime(Duration)
		, TimeElapsed(0.f)
		, StartValue(InStartValue)
		, EndValue(InEndValue)
		, bInterpolating(true)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, Caller(InCaller)
	{}

	virtual void UpdateOperation(FLatentResponse& Response) override;

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		static const FNumberFormattingOptions DelayTimeFormatOptions = FNumberFormattingOptions()
			.SetMinimumFractionalDigits(3)
			.SetMaximumFractionalDigits(3);
		return FText::Format(NSLOCTEXT("FInterpolateTimeToAction", "ActionTimeFmt", "Move ({0} seconds left)"), FText::AsNumber(TotalTime - TimeElapsed, &DelayTimeFormatOptions)).ToString();
	}
#endif
};


UCLASS(BlueprintType)
class SURVIVALGO_API UGameUtilLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Global", meta = (DisplayName = "GAME_SAVINGS_BASE_PATH"))
	static FString GetGameSavingsBasePath();

	// Linear interpolation for time value. ExtraID for extra information.
	UFUNCTION(BlueprintCallable, Category = "Playing|Utils", 
		meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", 
			ExpandEnumAsExecs = "InActionType,OutActionType", 
			Duration = "1.0", InStartValue = "0.0", InEndValue = "1.0", InExtraID = "-1"))
	static void LerpTime(
		UObject* WorldContextObject, 
		ETickLerpInAction InActionType,
		float Duration, 
		float InStartValue, float InEndValue, 
		int32 InExtraID,
		ETickLerpOutAction& OutActionType, 
		float& OutTime, float& OutValue, 
		int32& OutExtraID,
		FLatentActionInfo LatentInfo);

};
