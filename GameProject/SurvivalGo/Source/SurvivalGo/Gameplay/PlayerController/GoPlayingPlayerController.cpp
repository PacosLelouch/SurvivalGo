// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GoPlayingPlayerController.h"
#include "PlayerState/GoPlayingPlayerState.h"
#include "GameState/GoPlayingGameState.h"
#include "GameMode/GoPlayingGameMode.h"
#include "Pawn/GoChessPawn.h"
#include "GoMapBuilder.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Subsystem/MapSubsystem.h"
#include "MapSettings.h"

AGoPlayingPlayerController::AGoPlayingPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShowMouseCursor = true;
}

void AGoPlayingPlayerController::BeginPlay()
{
	Super::BeginPlay();
	auto* GoPS = GetPlayerState<AGoPlayingPlayerState>();
	if (IsValid(GoPS))
	{
		GoPS->OwningController = this;
	}
}

AGoPlayingGameState* AGoPlayingPlayerController::GetGoPlayingGameState()
{
	if (CachedGoPlayingGS.IsValid())
	{
		return CachedGoPlayingGS.Get();
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
	if (!IsValid(GoPlayingGS))
	{
		return nullptr;
	}
	CachedGoPlayingGS = GoPlayingGS;
	return GoPlayingGS;
}

AGoPlayingGameMode* AGoPlayingPlayerController::GetGoPlayingGameMode()
{
	if (CachedGoPlayingGM.IsValid())
	{
		return CachedGoPlayingGM.Get();
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	auto* GoPlayingGM = World->GetAuthGameMode<AGoPlayingGameMode>();
	if (!IsValid(GoPlayingGM))
	{
		return nullptr;
	}
	CachedGoPlayingGM = GoPlayingGM;
	return GoPlayingGM;
}

AGoMapBuilder* AGoPlayingPlayerController::GetMapBuilder()
{
	UMapSubsystem* MapSubsys = UMapSubsystem::GetSubsystemInternal(this);
	if (MapSubsys)
	{
		return MapSubsys->GetMapBuilder();
	}
	// else
	auto* GoPlayingGS = GetGoPlayingGameState();
	if (!IsValid(GoPlayingGS))
	{
		return nullptr;
	}
	return GoPlayingGS->GetMapBuilder();
}

bool AGoPlayingPlayerController::CanMoveToNextPhaseManually(bool bOnlyInActionPhase)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}
	auto* GoPlayingGS = GetGoPlayingGameState();
	if (!IsValid(GoPlayingGS))
	{
		return false;
	}
	auto* GoPlayingPS = GetPlayerState<AGoPlayingPlayerState>();
	if (GoPlayingGS->PlayerTurn != GoPlayingPS->GetPlayerIndex())
	{
		return false;
	}
	if (bOnlyInActionPhase && GoPlayingGS->RoundPhase != ERoundPhase::Action)
	{
		return false;
	}
	return true;
}

void AGoPlayingPlayerController::TryToMoveToNextPhase_Implementation()
{
	if (!CanMoveToNextPhaseManually(true))
	{
		return;
	}
	auto* GoPlayingGM = GetGoPlayingGameMode();
	if (!GoPlayingGM)
	{
		return;
	}
	GoPlayingGM->MoveToNextPhase();
}

bool AGoPlayingPlayerController::TraceObstacle(AActor*& TargetObstacle, bool& bOwnChess, const FHitResult& InHitResultIfTraced)
{
	FHitResult HitRes = InHitResultIfTraced;
	if (!HitRes.GetActor() && !TraceSingleGetHitResult(HitRes))
	{
		return false;
	}
	if (!HitRes.GetActor())
	{
		return false;
	}
	if (!HitRes.GetActor()->Implements<UMapObstacleInterface>())
	{
		return false;
	}
	TargetObstacle = HitRes.GetActor();
	bOwnChess = false;
	if (auto* Chess = Cast<AGoChessPawn>(TargetObstacle))
	{
		auto* GoPlayingPS = GetPlayerState<AGoPlayingPlayerState>();
		if (IsValid(GoPlayingPS) && Chess->OwningPlayerIndex == GoPlayingPS->GetPlayerIndex())
		{
			bOwnChess = true;
		}
	}
	return true;
}

bool AGoPlayingPlayerController::TraceBlock(UMapBlock*& TargetBlock, const FHitResult& InHitResultIfTraced)
{
	FHitResult HitRes = InHitResultIfTraced;
	if (!HitRes.GetActor() && !TraceSingleGetHitResult(HitRes))
	{
		return false;
	}
	if (!HitRes.GetActor() || !HitRes.Component.IsValid())
	{
		return false;
	}
	auto* MapBuilder = Cast<AGoMapBuilder>(HitRes.GetActor());
	if (!MapBuilder || !HitRes.Component.Get()->IsA<UInstancedStaticMeshComponent>())
	{
		return false;
	}

	FMapCoordinate Coordinate = MapBuilder->GetCoordinate(HitRes.Location);
	UMapBlock* MapBlock = MapBuilder->GetMapBlockWithCoordinate(Coordinate);
	if (!MapBlock)
	{
		return false;
	}

	TargetBlock = MapBlock;

	return true;
}

bool AGoPlayingPlayerController::TraceSingleGetHitResult(FHitResult& OutHitResult)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}
	FVector WorldLocation = FVector::ZeroVector, WorldDirection = FVector::DownVector;
	if (!DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return false;
	}
	if (!World->LineTraceSingleByChannel(
		OutHitResult, WorldLocation, WorldLocation + WorldDirection * TraceDistance,
		ECollisionChannel::ECC_WorldDynamic))
	{
		return false;
	}
	return true;
}
