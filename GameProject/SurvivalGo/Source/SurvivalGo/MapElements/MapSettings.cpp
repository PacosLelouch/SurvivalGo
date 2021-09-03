// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "MapSettings.h"
#include "GoMapBuilder.h"
#include "Pawn/GoChessPawn.h"
#include "GameState/GoPlayingGameState.h"
#include "Subsystem/MapSubsystem.h"

FMapCoordinate::FMapCoordinate(const FIntVector& IntVector, EPawnDirection InDirection)
	: Coordinate(IntVector)
	, Direction(InDirection) 
{}

bool FMapCoordinate::operator==(const FMapCoordinate& Other) const
{
	return Coordinate == Other.Coordinate && Direction == Other.Direction;
}

FMapCoordinate FMapCoordinate::ToBlockCoordinate() const
{
	return FMapCoordinate(Coordinate, EPawnDirection::None);
}


void IMapObstacleInterface::DestroySelfInternal()
{
	AActor* Actor = Cast<AActor>(this);
	if (!Actor)
	{
		return;
	}
	UWorld* World = Actor->GetWorld();
	if (IsValid(World))
	{
		auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
		if (IsValid(GoPlayingGS))
		{
			if (auto* Obstacle = Cast<ANonPlayerObstacle>(Actor))
			{
				GoPlayingGS->NonPlayerObstacles.Remove(Obstacle->ObstacleId);
			}
			else if(auto* Chess = Cast<AGoChessPawn>(Actor))
			{
				GoPlayingGS->Chesses.Remove(FIntPoint(Chess->OwningPlayerIndex, Chess->ChessIndex));
			}
		}
	}

	RemoveSelfFromMap(); // In case.

	if (Actor->GetLocalRole() == ENetRole::ROLE_Authority)
	{
		Actor->Destroy();
	}
}

void IMapObstacleInterface::RemoveSelfFromMap()
{
	AActor* Actor = Cast<AActor>(this);
	if (!Actor)
	{
		return;
	}
	auto* GI = Actor->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("MapBuilder not exist while destroying actor!"));
		return;
	}
	auto* MapSubsys = GI->GetSubsystem<UMapSubsystem>();
	if (!MapSubsys)
	{
		UE_LOG(LogMapSettings, Warning, TEXT("MapBuilder not exist while destroying actor!"));
		return;
	}

	MapSubsys->RemoveObstacleFromMap(Actor);
}
