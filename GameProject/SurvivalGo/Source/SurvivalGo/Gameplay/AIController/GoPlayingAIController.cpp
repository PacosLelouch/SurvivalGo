// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GoPlayingAIController.h"
#include "PlayerState/GoPlayingPlayerState.h"
#include "GameMode/GoPlayingGameMode.h"
#include "Pawn/GoPlayerPawn.h"

AGoPlayingAIController::AGoPlayingAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsPlayerState = true;
}

void AGoPlayingAIController::BeginPlay()
{
	Super::BeginPlay();
	auto* GoPS = GetPlayerState<AGoPlayingPlayerState>();
	if (IsValid(GoPS))
	{
		GoPS->OwningController = this;
		UWorld* World = GetWorld();
		auto* GoGM = World->GetAuthGameMode<AGoPlayingGameMode>();
		if (GoGM)
		{
			auto* ExtraPawn = World->SpawnActorDeferred<AGoPlayerPawn>(GoGM->DefaultPawnClass, FTransform::Identity);
			Possess(ExtraPawn);
		}
	}
}
