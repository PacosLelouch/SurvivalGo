// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GoPlayingGameMode.h"
#include "GameInstance/GoGameInstance.h"
#include "GameState/GoPlayingGameState.h"
#include "PlayerController/GoPlayingPlayerController.h"
#include "AIController/GoPlayingAIController.h"
#include "PlayerState/GoPlayingPlayerState.h"
#include "Pawn/GoPlayerPawn.h"
#include "Pawn/GoChessPawn.h"
#include "Subsystem/MapSubsystem.h"
#include "GoMapBuilder.h"

DEFINE_LOG_CATEGORY_STATIC(LogGoPlayingGameMode, Warning, All)

AGoPlayingGameMode::AGoPlayingGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AGoPlayingGameState::StaticClass();
	PlayerControllerClass = AGoPlayingPlayerController::StaticClass();
	PlayerStateClass = AGoPlayingPlayerState::StaticClass();
	DefaultPawnClass = AGoPlayerPawn::StaticClass();

	MapBuilderClass = AGoMapBuilder::StaticClass();
	AIControllerClass = AGoPlayingAIController::StaticClass();
}

void AGoPlayingGameMode::BeginPlay()
{
	Super::BeginPlay();
	GlobalObstacleId = 0;
}

void AGoPlayingGameMode::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	GlobalObstacleId = 0;
	Super::EndPlay(EndPlayReason);
}

void AGoPlayingGameMode::CreateAIPlayers()
{
	UWorld* World = GetWorld();
	auto* GoGS = GetGameState<AGoPlayingGameState>();
	if (IsValid(World) && IsValid(GoGS))
	{
		int32 TotalPlayer = GoGS->TotalPlayer;
		int32 AIPlayer = GoGS->AIPlayer;
		for (int32 i = 0; i < TotalPlayer; ++i)
		{
			AGoPlayingAIController* AIController = World->SpawnActorDeferred<AGoPlayingAIController>(AIControllerClass, FTransform::Identity, this);
			AIController->bWantsPlayerState = i < AIPlayer;
			AIController->bActivateAsPlayer = i < AIPlayer;
			AIController->FinishSpawning(FTransform::Identity);
		}
	}
}

void AGoPlayingGameMode::LoadMapSettings(FString SettingsName)
{
	UMapSubsystem* MapSubsystem = UMapSubsystem::GetSubsystemInternal(this);
	if (!MapSubsystem->LoadMap(SettingsName))
	{
		return;
	}
	bMapSettingsHaveLoaded = true;
}

bool AGoPlayingGameMode::GenerateScene(const FMapSettings& MapSettings)
{
	AGoPlayingGameState* GoPlayingGS = GetGameState<AGoPlayingGameState>();
	UWorld* World = GetWorld();
	if (!bMapSettingsHaveLoaded || !IsValid(World) || !IsValid(GoPlayingGS))
	{
		return false;
	}

	FTimerHandle TempTimerHandle;
	//MapBuilder->SetReplicates(true);
	AGoMapBuilder* MapBuilder = World->SpawnActorDeferred<AGoMapBuilder>(MapBuilderClass, FTransform::Identity, nullptr);
	MapBuilder->InitMapSettings = MapSettings;
	MapBuilder->FinishSpawning(FTransform::Identity);
	MapBuilder->bIsMapSettingsInited = true;

	// Server build map complete.
	OneClientBuildMapComplete();

	return true;
}

bool AGoPlayingGameMode::AddObstacles(AGoMapBuilder* MapBuilder, const FMapBlockSettings& BlockSettings)
{
	if (!IsValid(MapBuilder))
	{
		return false;
	}
	UWorld* World = GetWorld();
	auto* GoGI = GetGameInstance<UGoGameInstance>();
	if (!IsValid(GoGI))
	{
		return false;
	}
	const TMap<EObstacleType, TSoftClassPtr<ANonPlayerObstacle>>& ObstacleSettings = GoGI->ObstacleSettings;
	FVector ObstaclePosition = MapBuilder->GetPosition(BlockSettings.Coordinate);
	const FTransform & ObstacleTransform = MapBuilder->GetWorldTransform(ObstaclePosition);
	for (int32 i = 0; i < BlockSettings.ObstacleSettings.Num(); ++i)
	{
		const auto& ObstacleType = BlockSettings.ObstacleSettings[i];
		auto* ObstacleClassPtr = ObstacleSettings.Find(ObstacleType);
		if (ObstacleClassPtr && FMapBlockSettings::IsValidObstacleTypeInSettings(ObstacleType))
		{
			auto* ObstacleClass = (*ObstacleClassPtr).LoadSynchronous();
			ANonPlayerObstacle* ObstacleActor = //World->SpawnActor<ANonPlayerObstacle>(ObstacleClass);
				World->SpawnActorDeferred<ANonPlayerObstacle>(ObstacleClass, ObstacleTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			ObstacleActor->Coordinate = BlockSettings.Coordinate;
			ObstacleActor->ObstacleId = GlobalObstacleId++;
			//MapBuilder->AddObstacleMulticast(ObstacleActor, BlockSettings.Coordinate);
			ObstacleActor->FinishSpawning(ObstacleTransform);
		}
	}
	return true;
}

bool AGoPlayingGameMode::AddChesses(AGoMapBuilder* MapBuilder, const TArray<FPlayerPawnSettings>& PlayerPawnSettingsArray)
{
	if (!IsValid(MapBuilder))
	{
		return false;
	}
	UWorld* World = GetWorld();
	auto* GoGI = GetGameInstance<UGoGameInstance>();
	auto* GoPlayingGS = GetGameState<AGoPlayingGameState>();
	if (!IsValid(GoGI) || !IsValid(GoPlayingGS))
	{
		return false;
	}
	TMap<int32, AGoPlayingPlayerState*> PlayerStates;
	PlayerStates.Reserve(GoPlayingGS->PlayerArray.Num());
	for (auto RawPlayerStateObjPtr : GoPlayingGS->PlayerArray)
	{
		auto* RawPlayerState = RawPlayerStateObjPtr.Get();
		if (auto* PlayerState = Cast<AGoPlayingPlayerState>(RawPlayerState))
		{
			PlayerStates.Add(PlayerState->GetPlayerIndex(), PlayerState);
		}
	}

	TMap<int32, int32> ChessIndexCache;
	ChessIndexCache.Reserve(PlayerStates.Num());
	for (auto& PlayerStatePair : PlayerStates)
	{
		ChessIndexCache.Add(PlayerStatePair.Key, 0);
	}

	for (const auto& PawnDataElement : PlayerPawnSettingsArray)
	{
		int32 PlayerIndex = PawnDataElement.PlayerId;
		int32 ChessLevel = PawnDataElement.DefaultChessLevel; // May be override by player.
		if (PlayerIndex >= PlayerStates.Num()
			|| ChessLevel >= GoGI->ChessClassSettings.Num() 
			|| !PlayerStates.Contains(PlayerIndex))
		{
			continue;
		}
		auto* CurrentPS = PlayerStates[PlayerIndex];
		const TSoftClassPtr<AGoChessPawn>& PawnClassRes = GoGI->ChessClassSettings[ChessLevel];
		TSubclassOf<AGoChessPawn> PawnActorClass = PawnClassRes.LoadSynchronous();
		//if (!PawnActorClass->IsChildOf(AGoChessPawn::StaticClass()))
		//{
		//	UE_LOG(LogGoPlayingGameMode, Warning, TEXT("[%s] is not a GoChessPawn class."), *PawnActorClass->GetFName().ToString());
		//	return false;
		//}
		FVector PawnPosition = MapBuilder->GetPosition(PawnDataElement.Coordinate);
		FTransform ObstacleTransform = MapBuilder->GetWorldTransform(PawnPosition);
		ObstacleTransform.SetRotation(UMapElementLibrary::GetWorldRotatorByDirection(PawnDataElement.Coordinate.Direction).Quaternion());
		AGoChessPawn* Chess = World->SpawnActorDeferred<AGoChessPawn>(PawnActorClass, ObstacleTransform, 
			CurrentPS->OwningController ? CurrentPS->OwningController->GetPawn() : nullptr, 
			nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Chess->Coordinate = PawnDataElement.Coordinate;
		Chess->OwningPlayerIndex = PlayerIndex;
		Chess->ChessIndex = ChessIndexCache[PlayerIndex]++;
		Chess->FinishSpawning(ObstacleTransform);
	}
	return true;
}

void AGoPlayingGameMode::PostMapBuildComplete_Implementation()
{
	PostMapBuildCompleteDelegate.Broadcast();
}

void AGoPlayingGameMode::InitPlayerNum(int32 InTotalPlayer, int32 AIPlayerNum)
{
	auto* GoPlayingGS = GetGameState<AGoPlayingGameState>();
	if (!GoPlayingGS)
	{
		return;
	}
	GoPlayingGS->TotalPlayer = InTotalPlayer;
	GoPlayingGS->AIPlayer = AIPlayerNum;
	GoPlayingGS->MatchState = EGoMatchState::BeforePlaying;
}

void AGoPlayingGameMode::InitPhase()
{
	auto* GoPlayingGS = GetGameState<AGoPlayingGameState>();
	if (!GoPlayingGS)
	{
		return;
	}
	GoPlayingGS->Round = 0;
	GoPlayingGS->PlayerTurn = 0;
	GoPlayingGS->RoundPhase = ERoundPhase::Begin;
	GoPlayingGS->MatchState = EGoMatchState::Playing;

	GoPlayingGS->OnRep_RoundPhase();
}

void AGoPlayingGameMode::MoveToNextPhase()
{
	auto* GoPlayingGS = GetGameState<AGoPlayingGameState>();
	if (!GoPlayingGS)
	{
		return;
	}

	bool bOnePlayerEnd = false;
	GoPlayingGS->RoundPhase = UGameplayUtilLibrary::GetNextPhase(GoPlayingGS->RoundPhase, bOnePlayerEnd);

	if (bOnePlayerEnd)
	{
		if (GoPlayingGS->PlayerTurn + 1 == GoPlayingGS->TotalPlayer)
		{
			GoPlayingGS->PlayerTurn = 0;
			++GoPlayingGS->Round;
		}
		else
		{
			++GoPlayingGS->PlayerTurn;
		}
	}

	GoPlayingGS->OnRep_RoundPhase();
}

void AGoPlayingGameMode::OneClientBuildMapComplete()
{
	++NumMapHasGenerated;
	auto* GoGS= GetGameState<AGoPlayingGameState>();
	if (NumMapHasGenerated == GoGS->TotalPlayer - GoGS->AIPlayer)
	{
		bMapHasGenerated = true;
		PostMapBuildComplete();
	}
}
