// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "MapElementsRuntime.h"
#include "ChessManipulator/Movement/MapElementsInterface.h"
#include "GoMapBuilder.generated.h"

class AGoChessPawn;
class UHierarchicalInstancedStaticMeshComponent;
class FLifetimeProperty;

UCLASS(BlueprintType, Blueprintable)
class SURVIVALGO_API AGoMapBuilder : public AActor, public IMapSourceInterface
{
	GENERATED_BODY()
public:
	AGoMapBuilder(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UNetConnection* GetNetConnection() const override;

	//// Start MapSourceInterface

	virtual bool GetAdjacents_Implementation(TArray<TScriptInterface<IMapBlockInterface>>& OutTargets, const TScriptInterface<IMapBlockInterface>& Source, EBlockAdjacentType AdjacentType = EBlockAdjacentType::Default, bool bAppend = false) override;

	virtual bool GetAdjacentsWithDistance_Implementation(TMap<UObject*, int32>& OutTargets, const TScriptInterface<IMapBlockInterface>& Source, int32 MaxDistance = 2, EBlockAdjacentType AdjacentType = EBlockAdjacentType::Default, bool bAppend = false) override;
	
	virtual bool GetPath_Implementation(TArray<TScriptInterface<IMapBlockInterface>>& OutTargets, const TScriptInterface<IMapBlockInterface>& Source, const TScriptInterface<IMapBlockInterface>& Target, EBlockAdjacentType AdjacentType = EBlockAdjacentType::Default) override;

	//// End MapSourceInterface

public:
	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	void UpdateDatasFromGameState();

	UFUNCTION(BlueprintPure, Category = "GoMap|Settings")
	FIntVector GetMapSize() const;

	UFUNCTION(BlueprintPure, Category = "GoMap|Settings")
	bool IsCoordinateInBound(const FMapCoordinate& InCoordinate) const;

	UFUNCTION(BlueprintPure, Category = "GoMap|Settings")
	UMapBlock* GetMapBlockWithCoordinate(const FMapCoordinate& InCoordinate) const;

	UFUNCTION(BlueprintPure, Category = "GoMap|Settings")
	FVector GetPosition(const FMapCoordinate& InCoordinate) const;

	UFUNCTION(BlueprintPure, Category = "GoMap|Settings")
	FTransform GetWorldTransform(const FVector& RelativePosition) const;

	UFUNCTION(BlueprintPure, Category = "GoMap|Settings")
	FMapCoordinate GetCoordinate(const FVector& WorldPosition, EPawnDirection Direction = EPawnDirection::None) const;

	// If set StartBlockForDirection, DstTransforms will include rotations.
	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	void BatchGetTransformOfBlock(TArray<FTransform>& DstTransforms, const TArray<UMapBlock*>& SrcBlocks, UMapBlock* StartBlockForDirection = nullptr);

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	void InitBlocksFromGameInstance();

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	void GenerateMapFromSettings();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "GoMap|Settings")
	void BuildMapCompleteServer();

	//UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "GoMap|Settings")
	//void GenerateMapFromSettingsMulticast(const FMapSettings& MapSettings);

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	void GenerateMapFromSettingsInternal(const FMapSettings& MapSettings);

	//UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "GoMap|Settings")
	//void AddObstacleMulticast(AActor* Obstacle, const FMapCoordinate& Coordinate);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GoMap|Settings")
	void AddObstacle(AActor* Obstacle, const FMapCoordinate& Coordinate);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GoMap|Settings")
	void AddChessPawn(AGoChessPawn* ChessPawn, const FMapCoordinate& Coordinate);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Playing|Map", 
		meta = (DisplayName = "Update Chess Move Multicast"))
	void UpdateMapBuilderCoordinateInternalMulticast(int32 PlayerIndex, int32 ChessIndex, const FMapCoordinate& NewCoordinate);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Playing|Map")
	void UpdateChessAttackChessMulticast(
		int32 FromPlayerIndex, int32 FromChessIndex,
		int32 ToPlayerIndex, int32 ToChessIndex);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Playing|Map")
	void UpdateChessAttackObstacleMulticast(
		int32 FromPlayerIndex, int32 FromChessIndex,
		int32 ToObstacleIndex);

	UFUNCTION()
	virtual void OnRep_InitMapSettings();

public:
	void UpdateChessMoveInternal(AGoChessPawn* ChessPawn, const FMapCoordinate& NewCoordinate);

	void UpdateChessAttackInternal(AGoChessPawn* ChessPawn, const TScriptInterface<IMapObstacleInterface>& Target);

	void SettleMoveCost(AGoChessPawn* ChessPawn, int32 Factor = 1);

	void SettleAttackCost(AGoChessPawn* ChessPawn, int32 Factor = 1);

public:
	//UPROPERTY(VisibleAnywhere, ReplicatedUsing = GenerateMapFromSettings, BlueprintReadWrite, Category = "GoMap|Settings")
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_InitMapSettings, BlueprintReadWrite, Category = "GoMap|Settings")
	FMapSettings InitMapSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	bool bIsMapSettingsInited = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TMap<AGoChessPawn*, FMapCoordinate> ChessData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TMap<FMapCoordinate, UMapBlock*> BlockData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GoMap|Settings")
	TMap<EBlockType, UHierarchicalInstancedStaticMeshComponent*> BlockMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	FIntVector MapLowerBound = FIntVector::ZeroValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	FIntVector MapUpperBound = FIntVector::ZeroValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings", meta = (ExposeOnSpawn = true))
	float BlockLength = 0.;
};
