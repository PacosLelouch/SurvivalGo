// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MapSettings.h"
#include "GameplayDefines.generated.h"

class AGoChessPawn;
class ANonPlayerObstacle;
class UNetConnection;
class AGoPlayingGameState;

DEFINE_LOG_CATEGORY_STATIC(LogGoPlaying, Log, All)

UENUM(BlueprintType)
enum class ERoundPhase : uint8
{
	Unknown,
	Begin,
	Action,
	End,
};

UENUM(BlueprintType)
enum class EGoMatchState : uint8
{
	BeforePlaying,
	Playing,
	GameOver,
};

UENUM(BlueprintType)
enum class EOperationTransactionState : uint8
{
	NotInTransaction,
	Moving,
	Attacking,
};

UCLASS(BlueprintType)
class SURVIVALGO_API UGameplayUtilLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Playing|Phase")
	static ERoundPhase GetNextPhase(ERoundPhase Phase, bool& bOnePlayerEnd);

	UFUNCTION(BlueprintPure, Category = "Playing|Map", meta = (WorldContext = "WorldContextObject"))
	static AGoChessPawn* GetChessByPlayerChessIndex(UObject* WorldContextObject, int32 PlayerIndex, int32 ChessIndex);

	UFUNCTION(BlueprintPure, Category = "Playing|Map", meta = (WorldContext = "WorldContextObject"))
	static ANonPlayerObstacle* GetObstacleById(UObject* WorldContextObject, int32 ObstacleId);

	UFUNCTION(BlueprintPure, Category = "Playing|Net")
	static UNetConnection* GetWorldNetConnection(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Playing|Controller")
	static bool IsInTransaction(EOperationTransactionState TransactionState);
};

UCLASS(BlueprintType)
class SURVIVALGO_API UPhaseEventHandler : public UObject
{
	GENERATED_BODY()
public:
	//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPhaseAutoEvent, AGoPlayingGameState*, GoPlayingGS);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FPhaseAutoEventSingleCast, AGoPlayingGameState*, GoPlayingGS);
public:
	UFUNCTION(BlueprintCallable, Category = "Playing|Delegate", meta = (WorldContextObject = "WorldContextObject"))
	static bool CreatePhaseAutoEvent(UObject* WorldContextObject, FPhaseAutoEventSingleCast SingleEvent, int32 Round, int32 PlayerTurn, ERoundPhase Phase);

	//UPROPERTY(BlueprintAssignable)
	//FPhaseAutoEvent PhaseAutoEventDelegate;
	UPROPERTY(BlueprintReadOnly, Category = "Playing|Delegate")
	TArray<FPhaseAutoEventSingleCast> PhaseAutoEventDelegates;
};
