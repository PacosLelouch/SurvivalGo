// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GoMapBuilder.h"
#include "Net/UnrealNetwork.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Pawn/GoChessPawn.h"
#include "PlayerController/GoPlayingPlayerController.h"
#include "GameState/GoPlayingGameState.h"
#include "GameMode/GoPlayingGameMode.h"
#include "GameInstance/GoGameInstance.h"
#include "PlayerState/GoPlayingPlayerState.h"
//#include "Subsystem/MapSubsystem.h"

AGoMapBuilder::AGoMapBuilder(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bNetLoadOnClient = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	NetPriority = 3.f;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetMobility(EComponentMobility::Stationary);
	UEnum* BlockEnum = StaticEnum<EBlockType>();
	for (int32 i = 0; i + 1 < BlockEnum->NumEnums(); ++i)
	{
		EBlockType NewBlockType = (EBlockType)BlockEnum->GetValueByIndex(i);
		if (NewBlockType == EBlockType::None)
		{
			continue;
		}
		FName NewBlockName(FString(TEXT("BlockMesh_")) + BlockEnum->GetNameStringByValue((int64)NewBlockType));
		auto* NewComp = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(NewBlockName);
		NewComp->SetupAttachment(RootComponent);
		BlockMeshes.Add(NewBlockType, NewComp);
	}
}

void AGoMapBuilder::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();
	//auto* FirstPC = World->GetFirstPlayerController<AGoPlayingPlayerController>();
	//SetOwner(FirstPC);
	auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
	if (IsValid(GoPlayingGS))
	{
		GoPlayingGS->MapBuilder = this;
	}
	//UpdateDatasFromGameState();
	//InitBlocksFromGameInstance();
	//FTimerHandle TempHandle;
	//FTimerDelegate TimerDelegate;
	//TimerDelegate.BindLambda([this]()
	//{
		// May BeginPlay after OnRep, so generate map here? Or delay...
		//GenerateMapFromSettings();
	//});
	//GetWorldTimerManager().SetTimer(TempHandle, TimerDelegate, 0.f, false, 1.f);
}

void AGoMapBuilder::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AGoMapBuilder, InitMapSettings, COND_InitialOrOwner);
}

UNetConnection* AGoMapBuilder::GetNetConnection() const
{
	UNetConnection* SuperConnection = Super::GetNetConnection();
	return SuperConnection ? SuperConnection : UGameplayUtilLibrary::GetWorldNetConnection(this);
}

bool AGoMapBuilder::GetAdjacents_Implementation(
	TArray<TScriptInterface<IMapBlockInterface>>& OutTargets, 
	const TScriptInterface<IMapBlockInterface>& Source, 
	EBlockAdjacentType AdjacentType, bool bAppend)
{
	if (!bAppend)
	{
		OutTargets.Empty();
	}
	int32 PrevNum = OutTargets.Num();
	auto* SourceBlock = Cast<UMapBlock>(Source.GetObject());
	if (!SourceBlock)
	{
		return false;
	}
	const FMapCoordinate& Coordinate = SourceBlock->Coordinate;
	TArray<FMapCoordinate> PAdjCoordinates;
	SourceBlock->GetPossibleAdjacentCoordinates(PAdjCoordinates);
	for (auto& PAdjCoordinate : PAdjCoordinates)
	{
		auto* AdjBlockPtr = BlockData.Find(PAdjCoordinate);
		if (!AdjBlockPtr)
		{
			continue;
		}
		switch (AdjacentType)
		{
		case EBlockAdjacentType::Movable:
			if ((*AdjBlockPtr)->Obstacles.Num() == 0)
			{
				OutTargets.Add(*AdjBlockPtr);
			}
			break;
		case EBlockAdjacentType::Attackable:
			if ((*AdjBlockPtr)->Obstacles.Num() > 0)
			{
				OutTargets.Add(*AdjBlockPtr);
			}
			break;
		case EBlockAdjacentType::Default:
		default:
			OutTargets.Add(*AdjBlockPtr);
			break;
		}
	}
	return OutTargets.Num() > PrevNum;
}

bool AGoMapBuilder::GetAdjacentsWithDistance_Implementation(
	TMap<UObject*, int32>& OutTargets,
	const TScriptInterface<IMapBlockInterface>& Source, 
	int32 MaxDistance, EBlockAdjacentType AdjacentType, bool bAppend)
{
	if (!bAppend)
	{
		OutTargets.Empty();
	}
	int32 PrevNum = OutTargets.Num();
	TMap<int32, TSet<TScriptInterface<IMapBlockInterface>>> DistTarget;
	TSet<TScriptInterface<IMapBlockInterface>> VisitedBlocks;
	DistTarget.Reserve(MaxDistance);
	DistTarget.Add(0, { Source });
	VisitedBlocks.Add(Source);
	for (int32 CurDist = 1; CurDist <= MaxDistance; ++CurDist)
	{
		int32 PrevDist = CurDist - 1;
		auto& NewTargets = DistTarget.Add(CurDist);
		for (auto& CurSource : DistTarget[PrevDist])
		{
			TArray<TScriptInterface<IMapBlockInterface>> TempTargets;
			if (GetAdjacents_Implementation(TempTargets, CurSource, AdjacentType, true))
			{
				for (auto& NewTarget : TempTargets)
				{
					if (!VisitedBlocks.Contains(NewTarget))
					{
						VisitedBlocks.Add(NewTarget);
						NewTargets.Add(NewTarget);
						OutTargets.Add(NewTarget.GetObject(), CurDist);
					}
				}
			}
		}
	}
	return OutTargets.Num() > PrevNum;
}

bool AGoMapBuilder::GetPath_Implementation(
	TArray<TScriptInterface<IMapBlockInterface>>& OutTargets,
	const TScriptInterface<IMapBlockInterface>& Source,
	const TScriptInterface<IMapBlockInterface>& Target, 
	EBlockAdjacentType AdjacentType)
{
	OutTargets.Empty();
	int32 PrevNum = OutTargets.Num();
	TMap<TScriptInterface<IMapBlockInterface>, TScriptInterface<IMapBlockInterface>> Prevs;
	TMap<int32, TSet<TScriptInterface<IMapBlockInterface>>> DistTarget;
	TSet<TScriptInterface<IMapBlockInterface>> VisitedBlocks;
	DistTarget.Add(0, { Source });
	VisitedBlocks.Add(Source);
	bool bFoundTarget = false;
	bool bNoNewBlockLeft = false;
	int32 CurDist = 1;
	for (; !bFoundTarget && !bNoNewBlockLeft; ++CurDist)
	{
		int32 PrevDist = CurDist - 1;
		auto& NewTargets = DistTarget.Add(CurDist);
		int32 CurVisitedNum = VisitedBlocks.Num();
		for (auto& CurSource : DistTarget[PrevDist])
		{
			TArray<TScriptInterface<IMapBlockInterface>> TempTargets;
			if (GetAdjacents_Implementation(TempTargets, CurSource, AdjacentType, true))
			{
				for (auto& NewTarget : TempTargets)
				{
					if (!VisitedBlocks.Contains(NewTarget))
					{
						VisitedBlocks.Add(NewTarget);
						NewTargets.Add(NewTarget);
						Prevs.Add(NewTarget, CurSource);
						if (NewTarget == Target)
						{
							bFoundTarget = true;
							break;
						}
					}
				}
			}
			if (bFoundTarget)
			{
				break;
			}
		}
		if (VisitedBlocks.Num() - CurVisitedNum == 0)
		{
			bNoNewBlockLeft = true;
		}
	}
	if (bFoundTarget)
	{
		OutTargets.Reserve(CurDist);
		auto* CurPtr = &Target;
		auto* PrevPtr = Prevs.Find(*CurPtr);
		while (CurPtr && (*CurPtr) != Source)
		{
			OutTargets.Add(*CurPtr);
			CurPtr = PrevPtr;
			PrevPtr = Prevs.Find(*CurPtr);
		}
		Algo::Reverse(OutTargets);

	}
	return OutTargets.Num() > PrevNum;
}

void AGoMapBuilder::UpdateDatasFromGameState()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
	if (IsValid(GoPlayingGS))
	{
		for (auto& ObstaclePair : GoPlayingGS->NonPlayerObstacles)
		{
			auto* BlockPtr = BlockData.Find(ObstaclePair.Value->Coordinate.ToBlockCoordinate());
			if (BlockPtr)
			{
				if (!(*BlockPtr)->Obstacles.Contains(ObstaclePair.Value))
				{
					(*BlockPtr)->Obstacles.Add(ObstaclePair.Value);
				}
			}
		}
		for (auto& ChessPair : GoPlayingGS->Chesses)
		{
			auto* BlockPtr = BlockData.Find(ChessPair.Value->Coordinate.ToBlockCoordinate());
			if (BlockPtr)
			{
				if (!(*BlockPtr)->Obstacles.Contains(ChessPair.Value))
				{
					(*BlockPtr)->Obstacles.Add(ChessPair.Value);
				}
			}
			if (!ChessData.Contains(ChessPair.Value))
			{
				ChessData.Add(ChessPair.Value, ChessPair.Value->Coordinate);
			}
		}
	}
}

FIntVector AGoMapBuilder::GetMapSize() const
{
	return MapUpperBound - MapLowerBound + FIntVector(1, 1, 1);
}

bool AGoMapBuilder::IsCoordinateInBound(const FMapCoordinate& InCoordinate) const
{
	if (InCoordinate.Coordinate.X < MapLowerBound.X || InCoordinate.Coordinate.X > MapUpperBound.X)
	{
		return false;
	}
	if (InCoordinate.Coordinate.Y < MapLowerBound.Y || InCoordinate.Coordinate.Y > MapUpperBound.Y)
	{
		return false;
	}
	if (InCoordinate.Coordinate.Z < MapLowerBound.Z || InCoordinate.Coordinate.Z > MapUpperBound.Z)
	{
		return false;
	}
	return true;
}

UMapBlock* AGoMapBuilder::GetMapBlockWithCoordinate(const FMapCoordinate& InCoordinate) const
{
	if (!IsCoordinateInBound(InCoordinate))
	{
		return nullptr;
	}
	auto* MapBlockPtr = InCoordinate.Direction == EPawnDirection::None ? 
		BlockData.Find(InCoordinate) :
		BlockData.Find(FMapCoordinate(InCoordinate.Coordinate, EPawnDirection::None));
	if (!MapBlockPtr || !IsValid(*MapBlockPtr))
	{
		return nullptr;
	}
	return *MapBlockPtr;
}

FVector AGoMapBuilder::GetPosition(const FMapCoordinate& InCoordinate) const
{
	FVector Origin = GetActorLocation();
	FVector Offset = FVector(InCoordinate.Coordinate) * BlockLength;
	return Origin + Offset;
}

FTransform AGoMapBuilder::GetWorldTransform(const FVector& RelativePosition) const
{
	FTransform ObstacleRelativeTransform(RelativePosition);
	const FTransform& ObstacleTransform =
		GetRootComponent() ?
		GetRootComponent()->GetComponentTransform() * ObstacleRelativeTransform : ObstacleRelativeTransform;
	return ObstacleTransform;
}

FMapCoordinate AGoMapBuilder::GetCoordinate(const FVector& WorldPosition, EPawnDirection Direction) const
{
	FVector Origin = GetActorLocation();
	FVector LocalPosition = WorldPosition - Origin;
	FVector CoordinateFloat = LocalPosition / BlockLength;
	return FMapCoordinate{
		FIntVector(
			FMath::RoundToInt(CoordinateFloat.X),
			FMath::RoundToInt(CoordinateFloat.Y),
			FMath::RoundToInt(CoordinateFloat.Z)
		), Direction };
}

void AGoMapBuilder::BatchGetTransformOfBlock(TArray<FTransform>& DstTransforms, const TArray<UMapBlock*>& SrcBlocks, UMapBlock* StartBlockForDirection)
{
	DstTransforms.Empty(SrcBlocks.Num());
	FVector LastTargetLocation = FVector::ZeroVector;
	for (int32 i = 0; i < SrcBlocks.Num(); ++i)
	{
		auto* Block = SrcBlocks[i];
		FRotator Rotator = FRotator::ZeroRotator;
		FVector TargetLocation = GetPosition(Block->Coordinate.Coordinate);
		if (StartBlockForDirection)
		{
			FVector SourceLocation = i == 0 ? 
				GetPosition(StartBlockForDirection->Coordinate.Coordinate) :
				LastTargetLocation;
			Rotator = (TargetLocation - SourceLocation).ToOrientationRotator();
		}
		else
		{
			Rotator = UMapElementLibrary::GetWorldRotatorByDirection(Block->Coordinate.Direction);
		}
		DstTransforms.Add(FTransform(
			Rotator,
			TargetLocation));
		LastTargetLocation = TargetLocation;
	}
}

void AGoMapBuilder::InitBlocksFromGameInstance()
{
	auto* GoGI = GetGameInstance<UGoGameInstance>();
	const TMap<EBlockType, TSoftObjectPtr<UStaticMesh>>& BlockMeshSettings = GoGI->BlockMeshSettings;
	for (auto& BlockPair : BlockMeshes)
	{
		auto BlockType = BlockPair.Key;
		auto* BlockMeshComp = BlockPair.Value;
		auto* StaticMeshSoftPtr = BlockMeshSettings.Find(BlockType);
		if (StaticMeshSoftPtr && IsValid(BlockMeshComp))
		{
			UStaticMesh* Mesh = StaticMeshSoftPtr->LoadSynchronous();
			//for (auto& StaticMaterial : Mesh->StaticMaterials)
			//{
			//	StaticMaterial.MaterialInterface->CheckMaterialUsage(EMaterialUsage::MATUSAGE_InstancedStaticMeshes);
			//}
			BlockMeshComp->SetStaticMesh(Mesh);
		}
	}
}

void AGoMapBuilder::GenerateMapFromSettings()
{
	InitBlocksFromGameInstance();
	//GenerateMapFromSettingsMulticast_Implementation(InitMapSettings);
	GenerateMapFromSettingsInternal(InitMapSettings);
	if (GetLocalRole() != ENetRole::ROLE_Authority)
	{
		BuildMapCompleteServer();
	}
}


void AGoMapBuilder::BuildMapCompleteServer_Implementation()
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		auto* GoGM = World->GetAuthGameMode<AGoPlayingGameMode>();
		GoGM->OneClientBuildMapComplete();
	}
}

//void AGoMapBuilder::GenerateMapFromSettingsMulticast_Implementation(const FMapSettings& MapSettings)
void AGoMapBuilder::GenerateMapFromSettingsInternal(const FMapSettings& MapSettings)
{
	UWorld* World = GetWorld();
	MapLowerBound = FIntVector::ZeroValue;
	MapUpperBound = FIntVector::ZeroValue;
	BlockLength = MapSettings.BlockLength;
	BlockData.Reserve(MapSettings.BlockDataSettings.Num());

	AGoPlayingGameMode* GoGM = nullptr;
	if (IsValid(World))
	{
		GoGM = World->GetAuthGameMode<AGoPlayingGameMode>();
	}

	//for (const auto& BlockDataSettingsPair : MapSettings.BlockDataSettings)
	//{
	//	const FMapBlockSettings& BlockSettings = BlockDataSettingsPair.Value;
	TOptional<FIntVector> MinCoord, MaxCoord;
	for (const FMapBlockSettings& BlockSettings : MapSettings.BlockDataSettings)
	{
		// BlockData
		UMapBlock* MapBlock = NewObject<UMapBlock>(this);
		BlockData.Add(BlockSettings.Coordinate, MapBlock);

		MapBlock->Coordinate = BlockSettings.Coordinate;
		MapBlock->BlockType = BlockSettings.BlockType;

		auto* BlockMeshCompPtr = BlockMeshes.Find(BlockSettings.BlockType);
		if (!BlockMeshCompPtr)
		{
			BlockMeshCompPtr = BlockMeshes.Find(EBlockType::None);
		}

		MinCoord = MinCoord ? 
			FIntVector(
				FMath::Min(MinCoord.GetValue().X, BlockSettings.Coordinate.Coordinate.X),
				FMath::Min(MinCoord.GetValue().Y, BlockSettings.Coordinate.Coordinate.Y),
				FMath::Min(MinCoord.GetValue().Z, BlockSettings.Coordinate.Coordinate.Z)) :
			BlockSettings.Coordinate.Coordinate;
		MaxCoord = MaxCoord ?
			FIntVector(
				FMath::Max(MaxCoord.GetValue().X, BlockSettings.Coordinate.Coordinate.X),
				FMath::Max(MaxCoord.GetValue().Y, BlockSettings.Coordinate.Coordinate.Y),
				FMath::Max(MaxCoord.GetValue().Z, BlockSettings.Coordinate.Coordinate.Z)) :
			BlockSettings.Coordinate.Coordinate;

		FVector BlockLocation = GetPosition(BlockSettings.Coordinate);
		FVector Scale3D(1.f);
		UStaticMesh* OriginalMesh = (*BlockMeshCompPtr)->GetStaticMesh();
		if (IsValid(OriginalMesh))
		{
			FVector Extent = OriginalMesh->GetBounds().BoxExtent;
			Scale3D.X = FMath::IsNearlyZero(Extent.X) ? 1.f : BlockLength * 0.5f / Extent.X;
			Scale3D.Y = FMath::IsNearlyZero(Extent.Y) ? 1.f : BlockLength * 0.5f / Extent.Y;
			Scale3D.Z = (Scale3D.X + Scale3D.Y) * 0.5f; 
				//1.f; //FMath::IsNearlyZero(Extent.Z) ? 1.f : BlockLength * 0.5f / Extent.Z;
		}
		(*BlockMeshCompPtr)->AddInstance(FTransform(FQuat::Identity, BlockLocation, Scale3D));

		if (IsValid(GoGM) && BlockSettings.ObstacleSettings.Num() > 0)
		{
			// Obstacles
			GoGM->AddObstacles(this, BlockSettings);
		}
	}

	if (IsValid(GoGM))
	{
		// Chesses
		GoGM->AddChesses(this, MapSettings.ChessDataSettings);
	}
	MapUpperBound = MaxCoord.Get(FIntVector::ZeroValue);
	MapLowerBound = MinCoord.Get(FIntVector::ZeroValue);
	//if (MaxCoord && MinCoord)
	//{
	//	MapSize = MaxCoord.GetValue() - MinCoord.GetValue() + FIntVector(1, 1, 1);
	//}
}

//void AGoMapBuilder::AddObstacleMulticast_Implementation(AActor* Obstacle, const FMapCoordinate& Coordinate)
void AGoMapBuilder::AddObstacle_Implementation(AActor* Obstacle, const FMapCoordinate& Coordinate)
{
	if (IsValid(Obstacle))
	{
		UMapBlock** MapBlockPtr = BlockData.Find(Coordinate.ToBlockCoordinate());
		if (MapBlockPtr && IsValid(*MapBlockPtr))
		{
			(*MapBlockPtr)->Obstacles.Add(Obstacle);
		}
	}
}

void AGoMapBuilder::AddChessPawn_Implementation(AGoChessPawn* ChessPawn, const FMapCoordinate& Coordinate)
{
	if (IsValid(ChessPawn))
	{
		ChessData.Add(ChessPawn, Coordinate);
	}
}

void AGoMapBuilder::UpdateMapBuilderCoordinateInternalMulticast_Implementation(int32 PlayerIndex, int32 ChessIndex, const FMapCoordinate& NewCoordinate)
{
	auto* ChessPawn = UGameplayUtilLibrary::GetChessByPlayerChessIndex(this, PlayerIndex, ChessIndex);
	if (!IsValid(ChessPawn))
	{
		return;
	}
	UpdateChessMoveInternal(ChessPawn, NewCoordinate);
}

void AGoMapBuilder::UpdateChessAttackChessMulticast_Implementation(
	int32 FromPlayerIndex, int32 FromChessIndex,
	int32 ToPlayerIndex, int32 ToChessIndex)
{
	auto* FromChess = UGameplayUtilLibrary::GetChessByPlayerChessIndex(this, FromPlayerIndex, FromChessIndex);
	auto* ToChess = UGameplayUtilLibrary::GetChessByPlayerChessIndex(this, ToPlayerIndex, ToChessIndex);
	if (!IsValid(FromChess) || !IsValid(ToChess))
	{
		return;
	}
	UpdateChessAttackInternal(FromChess, ToChess);
}

void AGoMapBuilder::UpdateChessAttackObstacleMulticast_Implementation(
	int32 FromPlayerIndex, int32 FromChessIndex,
	int32 ToObstacleIndex)
{
	auto* FromChess = UGameplayUtilLibrary::GetChessByPlayerChessIndex(this, FromPlayerIndex, FromChessIndex);
	auto* ToObstacle = UGameplayUtilLibrary::GetObstacleById(this, ToObstacleIndex);
	if (!IsValid(FromChess) || !IsValid(ToObstacle))
	{
		return;
	}
	UpdateChessAttackInternal(FromChess, ToObstacle);
}

void AGoMapBuilder::OnRep_InitMapSettings()
{
	bIsMapSettingsInited = true;
}

void AGoMapBuilder::UpdateChessMoveInternal(AGoChessPawn* ChessPawn, const FMapCoordinate& NewCoordinate)
{
	//FTransform FromTransform(UMapElementLibrary::GetWorldRotatorByDirection(ChessPawn->Coordinate.Direction), GetPosition(ChessPawn->Coordinate));
	//FTransform ToTransform(UMapElementLibrary::GetWorldRotatorByDirection(NewCoordinate.Direction), GetPosition(NewCoordinate));
	//ChessPawn->MoveOnceInView(FromTransform, ToTransform);

	auto* SourceBlockPtr = BlockData.Find(ChessPawn->Coordinate.ToBlockCoordinate());
	auto* TargetBlockPtr = BlockData.Find(NewCoordinate.ToBlockCoordinate());
	if (!(SourceBlockPtr && IsValid(*SourceBlockPtr) && TargetBlockPtr && IsValid(*TargetBlockPtr)))
	{
		return;
	}
	TArray<TScriptInterface<IMapBlockInterface>> MovePath;
	if (!GetPath_Implementation(MovePath, *SourceBlockPtr, *TargetBlockPtr, EBlockAdjacentType::Movable))
	{
		return;
	}
	
	// Remember to switch transaction state to NotInTransaction when transaction is end.
	ChessPawn->OnChangeOperationTransactionDelegate.Broadcast(EOperationTransactionState::Moving);

	FTransform FromTransform(UMapElementLibrary::GetWorldRotatorByDirection(ChessPawn->Coordinate.Direction), GetPosition(ChessPawn->Coordinate));

	TArray<UMapBlock*> MovePathMapBlock;
	UMapElementLibrary::InterfaceToBlock_Array(MovePathMapBlock, MovePath);
	TArray<FTransform> MovePathTransform;
	BatchGetTransformOfBlock(MovePathTransform, MovePathMapBlock, *SourceBlockPtr);
	ChessPawn->MoveBatchInView(FromTransform, MovePathTransform);
	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		ChessPawn->Coordinate = FMapCoordinate(
			NewCoordinate.Coordinate,
			UMapElementLibrary::GetPawnDirectionByRotator(MovePathTransform.Last().Rotator()));
	}

	(*SourceBlockPtr)->Obstacles.Remove(ChessPawn);
	(*TargetBlockPtr)->Obstacles.Add(ChessPawn);

	auto* PawnCoordinateInMapBuilderPtr = ChessData.Find(ChessPawn);
	if (PawnCoordinateInMapBuilderPtr)
	{
		(*PawnCoordinateInMapBuilderPtr) = NewCoordinate;
	}

	SettleMoveCost(ChessPawn, MovePathTransform.Num());
}

void AGoMapBuilder::UpdateChessAttackInternal(AGoChessPawn* ChessPawn, const TScriptInterface<IMapObstacleInterface>& Target)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
	if (!IsValid(GoPlayingGS))
	{
		return;
	}

	UMapBlock** TargetBlockPtr = BlockData.Find(Target->GetCoordinate_Implementation(true));
	if (!TargetBlockPtr || !IsValid(*TargetBlockPtr))
	{
		return;
	}

	TMap<UObject*, int32> AdjacentAtkSources;
	if (!GetAdjacentsWithDistance_Implementation(AdjacentAtkSources, *TargetBlockPtr, 1, EBlockAdjacentType::Attackable))
	{
		return;
	}

	FMapCoordinate ChessBlockCoordinate = ChessPawn->Coordinate.ToBlockCoordinate();
	UMapBlock** SourceBlockPtr = BlockData.Find(ChessBlockCoordinate);
	if (!SourceBlockPtr || !IsValid(*SourceBlockPtr) || !AdjacentAtkSources.Contains(*SourceBlockPtr))
	{
		return;
	}

	// Remember to switch transaction state to NotInTransaction when transaction is end.
	ChessPawn->OnChangeOperationTransactionDelegate.Broadcast(EOperationTransactionState::Attacking);
	
	TArray<AGoChessPawn*> OtherChesses;
	for (auto& ChessPair : GoPlayingGS->Chesses)
	{
		if (IsValid(ChessPair.Value) && !ChessPair.Value->IsActorBeingDestroyed() && 
			ChessPair.Value->OwningPlayerIndex == ChessPawn->OwningPlayerIndex
			&& ChessPair.Value->ChessIndex != ChessPawn->ChessIndex)
		{
			UMapBlock** AdjChessBlock = BlockData.Find(ChessPair.Value->Coordinate.ToBlockCoordinate());
			if (AdjChessBlock && AdjacentAtkSources.Contains(*AdjChessBlock))
			{
				OtherChesses.Add(ChessPair.Value);
			}
		}
	}
	ChessPawn->AttackInView(Target, OtherChesses);

	SettleAttackCost(ChessPawn, 1);

	int32 AttackValue = ChessPawn->CalculateAttackValue(OtherChesses);
	IMapObstacleInterface::Execute_ReceiveDamage(Target.GetObject(), ChessPawn, AttackValue);
}

void AGoMapBuilder::SettleMoveCost(AGoChessPawn* ChessPawn, int32 Factor)
{
	auto* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
	if (!IsValid(GoPlayingGS))
	{
		return;
	}
	auto* TargetPS = GoPlayingGS->GetPlayerState(ChessPawn->OwningPlayerIndex);
	if (!TargetPS)
	{
		return;
	}
	TargetPS->CurrentActionValue -= ChessPawn->MoveActionCost * Factor;
	IActionValueNotifierInterface::Execute_NotifyActionValueViewToUpdate(TargetPS);
}

void AGoMapBuilder::SettleAttackCost(AGoChessPawn* ChessPawn, int32 Factor)
{
	auto* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	auto* GoPlayingGS = World->GetGameState<AGoPlayingGameState>();
	if (!IsValid(GoPlayingGS))
	{
		return;
	}
	auto* TargetPS = GoPlayingGS->GetPlayerState(ChessPawn->OwningPlayerIndex);
	if (!TargetPS)
	{
		return;
	}
	TargetPS->CurrentActionValue -= ChessPawn->AttackActionCost * Factor;
	IActionValueNotifierInterface::Execute_NotifyActionValueViewToUpdate(TargetPS);
}
