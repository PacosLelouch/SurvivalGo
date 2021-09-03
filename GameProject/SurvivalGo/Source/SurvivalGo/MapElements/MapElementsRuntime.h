// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "MapSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChessManipulator/Movement/MapElementsInterface.h"
#include "UINotifier.h"
#include "MapElementsRuntime.generated.h"

class FLifetimeProperty;
class UNetConnection;
class AGoChessPawn;

UCLASS(BlueprintType, Blueprintable)
class ANonPlayerObstacle : public AActor, public IMapObstacleInterface, public IHealthNotifierInterface
{
	GENERATED_BODY()
public:
	ANonPlayerObstacle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//// Start IMapObstacleInterface

	virtual FMapCoordinate GetCoordinate_Implementation(bool bToBlockCoordinate) const override;

	virtual void ReceiveDamage_Implementation(AGoChessPawn* AttackSource, int32 Damage) override;

	virtual void DestroySelfIfNeeded_Implementation(int32 InstigatePlayer) override;

	//// End IMapObstacleInterface

	virtual UNetConnection* GetNetConnection() const override;

public:
	UFUNCTION(BlueprintCallable, Category = "Obstacle")
	virtual void OnRep_InitialTotalHealth();

	UFUNCTION(BlueprintCallable, Category = "Obstacle")
	virtual void OnRep_CurrentHealth();

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "GoMap|Settings")
		FMapCoordinate Coordinate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "GoMap|Settings")
		int32 ObstacleId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_InitialTotalHealth, Category = "Obstacle")
		int32 InitialTotalHealth = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentHealth, Category = "Obstacle")
		int32 CurrentHealth = 1;
};


UCLASS(BlueprintType, Blueprintable)
class SURVIVALGO_API UMapBlock : public UObject, public IMapBlockInterface
{
	GENERATED_BODY()
public:
	UMapBlock(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//// Start MapBlockInterface

	virtual UClass* GetMapBlockClass_Implementation() override;

	virtual bool IsAdjacent_Implementation(const TScriptInterface<IMapBlockInterface>& Target, EBlockAdjacentType AdjacentType = EBlockAdjacentType::Default) override;

	//// End MapBlockInterface

public:
	UFUNCTION(BlueprintPure, Category = "GoMap|Settings")
	void GetPossibleAdjacentCoordinates(TArray<FMapCoordinate>& OutCoordinates);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	FMapCoordinate Coordinate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	EBlockType BlockType = EBlockType::Walkable;

	// These actors must implement IMapObstacleInterface.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TSet<AActor*> Obstacles;
};

UCLASS(BlueprintType)
class SURVIVALGO_API UMapElementLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	//UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	//static void AddObstacleToMapBuilder(AActor* Obstacle, AGoMapBuilder* MapBuilder, const FMapCoordinate& Coordinate);

	UFUNCTION(BlueprintPure, Category = "GoMap|Obstacle")
	static EObstacleType GetObstacleType(AActor* Obstacle);

	UFUNCTION(BlueprintPure, Category = "GoMap|Settings", meta = (BlueprintAutocast))
	static FRotator GetWorldRotatorByDirection(EPawnDirection Direction);

	UFUNCTION(BlueprintPure, Category = "GoMap|Settings", meta = (BlueprintAutocast))
	static EPawnDirection GetPawnDirectionByRotator(const FRotator& InRotator);

	UFUNCTION(BlueprintPure, Category = "GoMap|Settings")
	static FMapCoordinate ToBlockCoordinate(const FMapCoordinate& InCoordinate);

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	static void InterfaceToBlock_Array(TArray<UMapBlock*>& Dst, const TArray<TScriptInterface<IMapBlockInterface>>& Src);

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	static void InterfaceToBlock_ObjIntMap(TMap<UMapBlock*, int32>& Dst, const TMap<UObject*, int32>& Src);

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	static bool SaveMapSettingsToFile(const FMapSettings& InMapSettings, FString FileName = "Saved/MapSettings/FirstMap.json");

};
