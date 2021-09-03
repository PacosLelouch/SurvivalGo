// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GoPlayingGameState.h"
#include "GameMode/GoPlayingGameMode.h"
#include "GoMapBuilder.h"
#include "PlayerState/GoPlayingPlayerState.h"
#include "EngineUtils.h"
#include "Misc/GameplayDefines.h"


AGoPlayingGameState::AGoPlayingGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//MapBuilder = CreateDefaultSubobject<AGoMapBuilder>(TEXT("MapBuilder"));
}

void AGoPlayingGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AGoPlayingGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoPlayingGameState, TotalPlayer);
	DOREPLIFETIME(AGoPlayingGameState, AIPlayer);
	DOREPLIFETIME(AGoPlayingGameState, Round);
	DOREPLIFETIME(AGoPlayingGameState, PlayerTurn);
	DOREPLIFETIME(AGoPlayingGameState, RoundPhase);
	DOREPLIFETIME(AGoPlayingGameState, MatchState);
}

void AGoPlayingGameState::CommitPhaseEventDelegate()
{
	--RemainingDelegateNum;
	if (RemainingDelegateNum <= 0)
	{
		NotifyAllPhaseEventDelegateCompleted();
	}
}

void AGoPlayingGameState::NotifyAllPhaseEventDelegateCompleted()
{
	RemainingDelegateNum = 0;
	if (GetLocalRole() == ENetRole::ROLE_Authority && RoundPhase != ERoundPhase::Action && AuthorityGameMode)
	{
		Cast<AGoPlayingGameMode>(AuthorityGameMode)->MoveToNextPhase();
	}
}

AGoMapBuilder* AGoPlayingGameState::GetMapBuilder()
{
	if (!IsValid(MapBuilder))
	{
		UWorld* World = GetWorld();
		for (TActorIterator<AGoMapBuilder> It(World); It; ++It)
		{
			if (IsValid(*It))
			{
				MapBuilder = *It;
				break;
			}
		}
	}
	return MapBuilder;
}

AGoPlayingPlayerState* AGoPlayingGameState::GetPlayerState(int32 PlayerIndex)
{
	for (auto* RawPlayerState : PlayerArray)
	{
		if (auto* PlayerState = Cast<AGoPlayingPlayerState>(RawPlayerState))
		{
			if (PlayerState->GetPlayerIndex(true) == PlayerIndex)
			{
				return PlayerState;
			}
		}
	}
	return nullptr;
}

bool AGoPlayingGameState::CheckRoundIsValidInPlaying(int32 TargetRound, int32 TargetPlayerTurn, ERoundPhase TargetPhase, bool bCurrentIsValid)
{
	if (MatchState != EGoMatchState::Playing)
	{
		return false;
	}
	if (TargetRound < Round)
	{
		return false;
	}
	else if (TargetRound == Round)
	{
		if (TargetPlayerTurn < PlayerTurn)
		{
			return false;
		}
		else if (TargetPlayerTurn == PlayerTurn)
		{
			if (bCurrentIsValid ? TargetRound < Round : TargetRound <= Round)
			{
				return false;
			}
		}
	}
	return true;
}

void AGoPlayingGameState::OnRep_MatchState()
{

}

void AGoPlayingGameState::OnRep_RoundPhase()
{
	DealWithMoveToNextPhase();
}

void AGoPlayingGameState::DealWithMoveToNextPhase_Implementation()
{
	PostMoveToNextPhaseDelegate.Broadcast(Round, PlayerTurn, RoundPhase);
	if (MatchState == EGoMatchState::Playing)
	{
		bool bWithoutDelegate = true;
		FIntVector Key(Round, PlayerTurn, (int32)RoundPhase);
		if (UPhaseEventHandler** HandlerPtr = PhaseAutoEvents.Find(Key))
		{
			if (IsValid(*HandlerPtr) && (*HandlerPtr)->PhaseAutoEventDelegates.Num() > 0)
			{
				RemainingDelegateNum = (*HandlerPtr)->PhaseAutoEventDelegates.Num();
				bWithoutDelegate = false;
				UEnum* EnumClass = StaticEnum<ERoundPhase>();
				UE_LOG(LogGoPlaying, Log, TEXT("Broadcast %d delegates in phase (%d, %d, %s)"),
					RemainingDelegateNum, Round, PlayerTurn, *EnumClass->GetNameStringByValue((int64)RoundPhase));
				for (auto& Delegate : (*HandlerPtr)->PhaseAutoEventDelegates)
				{
					Delegate.Execute(this);
				}
			}
			//if (IsValid(*HandlerPtr) && (*HandlerPtr)->PhaseAutoEventDelegate.IsBound())
			//{
			//	RemainingDelegateNum = (*HandlerPtr)->PhaseAutoEventDelegate.GetAllObjects().Num(); // Useful or not?
			//	UEnum* EnumClass = StaticEnum<ERoundPhase>();
			//	UE_LOG(LogGoPlaying, Log, TEXT("Broadcast %d delegates in phase (%d, %d, %s)"),
			//		RemainingDelegateNum, Round, PlayerTurn, *EnumClass->GetNameStringByValue((int64)RoundPhase));
			//	(*HandlerPtr)->PhaseAutoEventDelegate.Broadcast(this);
			//}
			PhaseAutoEvents.Remove(Key);
		}
		if (bWithoutDelegate)
		{
			NotifyAllPhaseEventDelegateCompleted();
		}
	}
}


//void AGoPlayingGameState::GenerateMap_Implementation(const FMapSettings& MapSettings)
//{
//	if (!MapBuilder)
//	{
//		return;
//	}
//	MapBuilder->GenerateMapFromSettings(MapSettings);
//}
