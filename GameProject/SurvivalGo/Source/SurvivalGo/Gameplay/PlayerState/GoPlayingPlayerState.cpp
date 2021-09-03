// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GoPlayingPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GameState/GoPlayingGameState.h"

AGoPlayingPlayerState::AGoPlayingPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void AGoPlayingPlayerState::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
		if (IsValid(GoPlayingGS))
		{
			GoPlayingGS->PostMoveToNextPhaseDelegate.AddDynamic(this, &AGoPlayingPlayerState::ReceiveMoveToNextPhase);
		}
	}
}

void AGoPlayingPlayerState::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
		if (IsValid(GoPlayingGS))
		{
			GoPlayingGS->PostMoveToNextPhaseDelegate.RemoveDynamic(this, &AGoPlayingPlayerState::ReceiveMoveToNextPhase);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AGoPlayingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoPlayingPlayerState, OverridePlayerIndex);
	DOREPLIFETIME(AGoPlayingPlayerState, TotalActionValue);
	DOREPLIFETIME(AGoPlayingPlayerState, CurrentActionValue);
}

AController* AGoPlayingPlayerState::GetOwningController(TSubclassOf<AController> ControllerClass)
//AController* AGoPlayingPlayerState::GetOwningController()
{
	if (IsValid(OwningController))
	{
		return OwningController->IsA(ControllerClass) ? OwningController : nullptr;
	}
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	for (auto It = World->GetControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (this == Controller->GetPlayerState<AGoPlayingPlayerState>())
		{
			OwningController = Controller;
			return OwningController->IsA(ControllerClass) ? OwningController : nullptr;
		}
	}
	return nullptr;
}

int32 AGoPlayingPlayerState::GetPlayerIndex(bool bGetParentIfNoOverride)
{
	return bGetParentIfNoOverride && OverridePlayerIndex == -1 ? GetPlayerId() : OverridePlayerIndex;
}

void AGoPlayingPlayerState::ReceiveMoveToNextPhase(int32 Round, int32 PlayerTurn, ERoundPhase RoundPhase)
{
	if (PlayerTurn == GetPlayerIndex(true) && RoundPhase == ERoundPhase::Begin)
	{
		if (GetLocalRole() == ENetRole::ROLE_Authority)
		{
			RecoverActionValue();
		}
	}

}

void AGoPlayingPlayerState::RecoverActionValue()
{
	CurrentActionValue = TotalActionValue;
	OnRep_CurrentActionValue();
}

void AGoPlayingPlayerState::OnRep_PlayerId()
{
	Super::OnRep_PlayerId();
}

void AGoPlayingPlayerState::OnRep_CurrentActionValue_Implementation()
{
	IActionValueNotifierInterface::Execute_NotifyActionValueViewToUpdate(this);
}

void AGoPlayingPlayerState::OnRep_OverridePlayerIndex_Implementation()
{
}
