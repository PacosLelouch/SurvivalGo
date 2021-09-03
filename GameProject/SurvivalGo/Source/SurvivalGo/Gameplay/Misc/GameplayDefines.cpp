// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GameplayDefines.h"
#include "Pawn/GoChessPawn.h"
#include "GameState/GoPlayingGameState.h"
#include "GoMapBuilder.h"
#include "Net/UnrealNetwork.h"

ERoundPhase UGameplayUtilLibrary::GetNextPhase(ERoundPhase Phase, bool& bOnePlayerEnd)
{
	if (Phase == ERoundPhase::End)
	{
		bOnePlayerEnd = true;
		return ERoundPhase::Begin;
	}
	bOnePlayerEnd = false;
	return ERoundPhase((uint8)Phase + (uint8)1);
}

AGoChessPawn* UGameplayUtilLibrary::GetChessByPlayerChessIndex(UObject* WorldContextObject, int32 PlayerIndex, int32 ChessIndex)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}
	UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
	if (!IsValid(GoPlayingGS))
	{
		return nullptr;
	}
	auto* ChessPtr = GoPlayingGS->Chesses.Find(FIntPoint(PlayerIndex, ChessIndex));
	if (!ChessPtr)
	{
		return nullptr;
	}
	auto* MapBuilder = GoPlayingGS->GetMapBuilder();
	if (IsValid(MapBuilder))
	{
		if (!MapBuilder->ChessData.Contains(*ChessPtr))
		{
			MapBuilder->ChessData.Add(*ChessPtr, (*ChessPtr)->Coordinate);
		}
	}
	return *ChessPtr;
	//for (auto& ChessDataPair : MapBuilder->ChessData)
	//{
	//	if (ChessDataPair.Key->OwningPlayerIndex == PlayerIndex && ChessDataPair.Key->ChessIndex == ChessIndex)
	//	{
	//		return ChessDataPair.Key;
	//	}
	//}
	//UE_LOG(LogTemp, Warning, TEXT("ChessPawn OwningPlayerIndex and ChessIndex is not valid."));
	//return nullptr;
}

ANonPlayerObstacle* UGameplayUtilLibrary::GetObstacleById(UObject* WorldContextObject, int32 ObstacleId)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}
	UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
	if (!IsValid(GoPlayingGS))
	{
		return nullptr;
	}
	auto* ObstaclePtr = GoPlayingGS->NonPlayerObstacles.Find(ObstacleId);
	if (!ObstaclePtr)
	{
		return nullptr;
	}
	auto* MapBuilder = GoPlayingGS->GetMapBuilder();
	if (IsValid(MapBuilder))
	{
		auto* BlockPtr = MapBuilder->BlockData.Find((*ObstaclePtr)->Coordinate);
		if (BlockPtr)
		{
			if (!(*BlockPtr)->Obstacles.Contains(*ObstaclePtr))
			{
				(*BlockPtr)->Obstacles.Add(*ObstaclePtr);
			}
		}
	}
	return (*ObstaclePtr);
}

UNetConnection* UGameplayUtilLibrary::GetWorldNetConnection(const AActor* Actor)
{
	UWorld* World = Actor->GetWorld();
	if (!IsValid(World) || !IsValid(World->NetDriver))
	{
		return nullptr;
	}
	return World->NetDriver->ServerConnection;
}

bool UGameplayUtilLibrary::IsInTransaction(EOperationTransactionState TransactionState)
{
	return TransactionState != EOperationTransactionState::NotInTransaction;
}

bool UPhaseEventHandler::CreatePhaseAutoEvent(UObject* WorldContextObject, FPhaseAutoEventSingleCast SingleEvent, int32 Round, int32 PlayerTurn, ERoundPhase Phase)
{
	if (!IsValid(WorldContextObject))
	{
		return false;
	}
	UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return false;
	}
	auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
	if (!IsValid(GoPlayingGS))
	{
		return false;
	}
	if (!GoPlayingGS->CheckRoundIsValidInPlaying(Round, PlayerTurn, Phase, false))
	{
		return false;
	}
	FIntVector Key(Round, PlayerTurn, (int32)Phase);
	auto** EventObjPtr = GoPlayingGS->PhaseAutoEvents.Find(Key);
	if (!EventObjPtr)
	{
		EventObjPtr = &GoPlayingGS->PhaseAutoEvents.Add(Key, NewObject<UPhaseEventHandler>(GoPlayingGS));
	}
	auto* EventObj = *EventObjPtr;
	EventObj->PhaseAutoEventDelegates.Add(SingleEvent);
	//EventObj->PhaseAutoEventDelegate.Add(SingleEvent);
	return true;
}
