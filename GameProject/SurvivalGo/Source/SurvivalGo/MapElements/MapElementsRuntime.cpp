// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "MapElementsRuntime.h"
#include "GoMapBuilder.h"
#include "Net/UnrealNetwork.h"
#include "GameState/GoPlayingGameState.h"
#include "Pawn/GoChessPawn.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"

ANonPlayerObstacle::ANonPlayerObstacle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;
	bReplicates = true;
}

void ANonPlayerObstacle::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		auto* GoGS = World->GetGameState<AGoPlayingGameState>();
		if (IsValid(GoGS))
		{
			GoGS->NonPlayerObstacles.Add(ObstacleId, this);
			AGoMapBuilder* MapBuilder = GoGS->GetMapBuilder();
			if (IsValid(MapBuilder))
			{
				//MapBuilder->AddObstacleMulticast_Implementation(this, Coordinate);
				MapBuilder->AddObstacle(this, Coordinate);
			}
		}
	}
}

void ANonPlayerObstacle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANonPlayerObstacle, Coordinate);
	DOREPLIFETIME(ANonPlayerObstacle, ObstacleId);
	DOREPLIFETIME(ANonPlayerObstacle, InitialTotalHealth);
	DOREPLIFETIME(ANonPlayerObstacle, CurrentHealth);
}

FMapCoordinate ANonPlayerObstacle::GetCoordinate_Implementation(bool bToBlockCoordinate) const
{
	return bToBlockCoordinate ? Coordinate.ToBlockCoordinate() : Coordinate;
}

void ANonPlayerObstacle::ReceiveDamage_Implementation(AGoChessPawn* AttackSource, int32 Damage)
{
	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0, InitialTotalHealth);
		if (CurrentHealth <= 0)
		{
			RemoveSelfFromMap();
		}
	}
	//IHealthNotifierInterface::Execute_NotifyHealthViewToUpdate(this);

	//IMapObstacleInterface::Execute_DestroySelfIfNeeded(this, AttackSource->OwningPlayerIndex);
}

void ANonPlayerObstacle::DestroySelfIfNeeded_Implementation(int32 InstigatePlayer)
{
	if (CurrentHealth <= 0)
	{
		DestroySelfInternal();
	}
}

UNetConnection* ANonPlayerObstacle::GetNetConnection() const
{
	UNetConnection* SuperConnection = Super::GetNetConnection();
	return SuperConnection ? SuperConnection : UGameplayUtilLibrary::GetWorldNetConnection(this);
}

void ANonPlayerObstacle::OnRep_InitialTotalHealth()
{
}

void ANonPlayerObstacle::OnRep_CurrentHealth()
{
	if (CurrentHealth <= 0 && InitialTotalHealth > 0)
	{
		RemoveSelfFromMap();
	}
}

bool FMapBlockSettings::IsValidObstacleTypeInSettings(EObstacleType ObstacleType)
{
	static const TSet<EObstacleType> InvalidType
	{
		EObstacleType::NotObstacle,
		EObstacleType::Pawn,
	};
	return !InvalidType.Contains(ObstacleType);
}

UMapBlock::UMapBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UClass* UMapBlock::GetMapBlockClass_Implementation()
{
	return GetClass();
}

bool UMapBlock::IsAdjacent_Implementation(const TScriptInterface<IMapBlockInterface>& Target, EBlockAdjacentType AdjacentType)
{
	auto* TargetBlock = Cast<UMapBlock>(Target.GetObject());
	if (!TargetBlock || TargetBlock == this)
	{
		return false;
	}
	auto Diff = TargetBlock->Coordinate.Coordinate - Coordinate.Coordinate;
	auto SumElementAbs = FMath::Abs(Diff.X) + FMath::Abs(Diff.Y) + FMath::Abs(Diff.Z);
	if (SumElementAbs > 1)
	{
		return false;
	}
	switch (AdjacentType)
	{
	case EBlockAdjacentType::Movable:
		return TargetBlock->Obstacles.Num() == 0;
	case EBlockAdjacentType::Attackable:
		if (TargetBlock->Obstacles.Num() == 0)
		{
			return false;
		}
		//TODO
		return true;
	}
	return true;
}


void UMapBlock::GetPossibleAdjacentCoordinates(TArray<FMapCoordinate>& OutCoordinates)
{
	OutCoordinates =
	{
		FMapCoordinate(Coordinate.Coordinate + FIntVector(1, 0, 0)),
		FMapCoordinate(Coordinate.Coordinate + FIntVector(0, 1, 0)),
		FMapCoordinate(Coordinate.Coordinate + FIntVector(-1, 0, 0)),
		FMapCoordinate(Coordinate.Coordinate + FIntVector(0, -1, 0)),
	};
}


//void UMapElementLibary::AddObstacleToMapBuilder(AActor* Obstacle, AGoMapBuilder* MapBuilder, const FMapCoordinate& Coordinate)
//{
//	if (IsValid(MapBuilder) && IsValid(Obstacle))
//	{
//		UMapBlock** MapBlockPtr = MapBuilder->BlockData.Find(Coordinate);
//		if (MapBlockPtr && IsValid(*MapBlockPtr))
//		{
//			(*MapBlockPtr)->Obstacles.Add(Obstacle);
//		}
//	}
//}

EObstacleType UMapElementLibrary::GetObstacleType(AActor* Obstacle)
{
	IMapObstacleInterface* MOI = Cast<IMapObstacleInterface>(Obstacle);
	if (MOI)
	{
		return MOI->GetObstacleType();
	}
	return EObstacleType::NotObstacle;
}

FRotator UMapElementLibrary::GetWorldRotatorByDirection(EPawnDirection Direction)
{
	switch (Direction)
	{
	case EPawnDirection::Up:
		return FRotator(0.f, 0.f, 0.f);
	case EPawnDirection::Right:
		return FRotator(0.f, 90.f, 0.f);
	case EPawnDirection::Down:
		return FRotator(0.f, 180.f, 0.f);
	case EPawnDirection::Left:
		return FRotator(0.f, -90.f, 0.f);
	}
	return FRotator(0.f, 0.f, 0.f);
}

EPawnDirection UMapElementLibrary::GetPawnDirectionByRotator(const FRotator& InRotator)
{
	//FRotator NormalizedRotator = InRotator.GetNormalized();
	auto NormalizedYaw = InRotator.Yaw;
	while (NormalizedYaw < 0.f)
	{
		NormalizedYaw += 360.f;
	}
	while (NormalizedYaw >= 360.f)
	{
		NormalizedYaw -= 360.f;
	}
	if (NormalizedYaw <= 45.f || NormalizedYaw > 315.f)
	{
		return EPawnDirection::Up;
	}
	else if (NormalizedYaw > 45.f && NormalizedYaw <= 135.f)
	{
		return EPawnDirection::Right;
	}
	else if (NormalizedYaw > 135.f && NormalizedYaw <= 225.f)
	{
		return EPawnDirection::Down;
	}
	else if (NormalizedYaw > 225.f && NormalizedYaw <= 315.f)
	{
		return EPawnDirection::Left;
	}
	return EPawnDirection::None;
}

FMapCoordinate UMapElementLibrary::ToBlockCoordinate(const FMapCoordinate& InCoordinate)
{
	return InCoordinate.ToBlockCoordinate();
}

void UMapElementLibrary::InterfaceToBlock_Array(TArray<UMapBlock*>& Dst, const TArray<TScriptInterface<IMapBlockInterface>>& Src)
{
	Dst.Empty(Src.Num());
	for (auto& SrcEle : Src)
	{
		Dst.Add(Cast<UMapBlock>(SrcEle.GetObject()));
	}
}

void UMapElementLibrary::InterfaceToBlock_ObjIntMap(TMap<UMapBlock*, int32>& Dst, const TMap<UObject*, int32>& Src)
{
	Dst.Empty(Src.Num());
	for (auto& SrcElePair : Src)
	{
		Dst.Add(Cast<UMapBlock>(SrcElePair.Key), SrcElePair.Value);
	}
}

bool UMapElementLibrary::SaveMapSettingsToFile(const FMapSettings& InMapSettings, FString FileName)
{
	FString JsonString;
	if (!FJsonObjectConverter::UStructToJsonObjectString(InMapSettings, JsonString))
	{
		return false;
	}
	if (!FFileHelper::SaveStringToFile(JsonString, *(FPaths::ProjectDir() / FileName), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		return false;
	}
	return true;
}
