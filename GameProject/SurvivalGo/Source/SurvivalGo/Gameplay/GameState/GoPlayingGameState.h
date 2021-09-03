// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "MapSettings.h"
#include "Misc/GameplayDefines.h"
#include "GoPlayingGameState.generated.h"

class AGoMapBuilder;
class AGoPlayingPlayerState;
class UPhaseEventHandler;


UCLASS(BlueprintType, Blueprintable)
class SURVIVALGO_API AGoPlayingGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPostMoveToNextPhase, int32, Round, int32, PlayerTurn, ERoundPhase, RoundPhase);

public:
	AGoPlayingGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable, Category = "Playing|Delegate")
	void CommitPhaseEventDelegate();

	UFUNCTION(BlueprintCallable, Category = "Playing|Delegate")
	void NotifyAllPhaseEventDelegateCompleted();

	UFUNCTION(BlueprintPure, Category = "Playing|Map")
	AGoMapBuilder* GetMapBuilder();

	UFUNCTION(BlueprintPure, Category = "Playing|Map")
	AGoPlayingPlayerState* GetPlayerState(int32 PlayerIndex);

	UFUNCTION(BlueprintPure, Category = "Playing")
	bool CheckRoundIsValidInPlaying(int32 TargetRound, int32 TargetPlayerTurn, ERoundPhase TargetPhase, bool bCurrentIsValid = false);


	UFUNCTION(BlueprintCallable, Category = "Playing")
	void OnRep_MatchState();

	// Need to insert an animation before OnRep_RoundPhase_Implementation()
	UFUNCTION(BlueprintCallable, Category = "Playing")
	void OnRep_RoundPhase();

	UFUNCTION(BlueprintNativeEvent, Category = "Playing")
	void DealWithMoveToNextPhase();

	//UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Playing")
	//void GenerateMap(const FMapSettings& MapSettings);


	UPROPERTY(BlueprintAssignable, Category = "Playing|Delegate")
	FPostMoveToNextPhase PostMoveToNextPhaseDelegate;

	UPROPERTY(BlueprintReadWrite, Category = "Playing|Delegate")
	TMap<FIntVector, UPhaseEventHandler*> PhaseAutoEvents;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playing|State")
	int32 RemainingDelegateNum = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Playing|State")
	int32 TotalPlayer = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Playing|State")
	int32 AIPlayer = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Playing|State")
	int32 Round = -1;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Playing|State")
	int32 PlayerTurn = -1;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_RoundPhase, BlueprintReadWrite, Category = "Playing|State")
	ERoundPhase RoundPhase = ERoundPhase::Unknown;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_MatchState, BlueprintReadWrite, Category = "Playing|State")
	EGoMatchState MatchState = EGoMatchState::BeforePlaying;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Playing|Map")
	AGoMapBuilder* MapBuilder = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playing|Map")
	TMap<FIntPoint, AGoChessPawn*> Chesses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playing|Map")
	TMap<int32, ANonPlayerObstacle*> NonPlayerObstacles;

};