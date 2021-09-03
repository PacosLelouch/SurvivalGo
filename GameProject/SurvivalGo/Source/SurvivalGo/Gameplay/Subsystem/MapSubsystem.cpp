// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "MapSubsystem.h"
#include "Engine/World.h"
#include "GameInstance/GoGameInstance.h"
#include "GameState/GoPlayingGameState.h"
#include "GoMapBuilder.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"
#include "Pawn/GoChessPawn.h"

UMapSubsystem::UMapSubsystem()
	: Super()
{

}

void UMapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GoGameInstance = Cast<UGoGameInstance>(GetGameInstance());
}

void UMapSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UMapSubsystem::LoadMap(const FString& InSettingsName)
{
	MapSettingsName = InSettingsName;
	FString MapSettingsPath = FPaths::SetExtension(GoGameInstance->MapSettingsBasePath / MapSettingsName, TEXT("json"));
	FString MapSettingsStr;
	if (!FFileHelper::LoadFileToString(MapSettingsStr, *MapSettingsPath))
	{
		return false;
	}
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(MapSettingsStr, &MapSettings, 0, 0))
	{
		return false;
	}
	return true;
}

AGoMapBuilder* UMapSubsystem::GetMapBuilder()
{
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
	return GoPlayingGS->GetMapBuilder();
}

void UMapSubsystem::RemoveObstacleFromMap(AActor* Obstacle)
{
	auto* MapBuilder = GetMapBuilder();
	if (!MapBuilder)
	{
		UE_LOG(LogMapSettings, Warning, TEXT("MapBuilder not exist while destroying actor!"));
		return;
	}
	UMapBlock** BlockPtr = MapBuilder->BlockData.Find(IMapObstacleInterface::Execute_GetCoordinate(Obstacle, true));
	if (BlockPtr && IsValid(*BlockPtr))
	{
		(*BlockPtr)->Obstacles.Remove(Obstacle);
	}
	if (auto* Chess = Cast<AGoChessPawn>(Obstacle))
	{
		MapBuilder->ChessData.Remove(Chess);
	}
}

UMapSubsystem* UMapSubsystem::GetSubsystemInternal(const UObject* WorldContextObject)
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
	UGameInstance* GI = World->GetGameInstance();
	if (!IsValid(GI))
	{
		return nullptr;
	}
	return GI->GetSubsystem<UMapSubsystem>();
}
