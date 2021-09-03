// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MapSettings.h"
#include "GoPlayingGameMode.generated.h"

class AGoMapBuilder;
class AGoPlayingAIController;
class UGoGameInstance;

UCLASS(BlueprintType, Blueprintable)
class SURVIVALGO_API AGoPlayingGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPureAction);

public:
	AGoPlayingGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	void CreateAIPlayers();

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	void LoadMapSettings(FString SettingsName);

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	bool GenerateScene(const FMapSettings& MapSettings);

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	bool AddObstacles(AGoMapBuilder* MapBuilder, const FMapBlockSettings& BlockSettings);

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	bool AddChesses(AGoMapBuilder* MapBuilder, const TArray<FPlayerPawnSettings>& PlayerPawnSettingsArray);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Playing")
	void PostMapBuildComplete();


	UFUNCTION(BlueprintCallable, Category = "Playing")
	void InitPlayerNum(int32 InTotalPlayer, int32 AIPlayerNum = 0);

	UFUNCTION(BlueprintCallable, Category = "Playing")
	void InitPhase();

	UFUNCTION(BlueprintCallable, Category = "Playing")
	void MoveToNextPhase();

	UPROPERTY(BlueprintAssignable, Category = "Playing|Delegate")
	FPureAction PostMapBuildCompleteDelegate;

public:
	void OneClientBuildMapComplete();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Playing")
	bool bMapSettingsHaveLoaded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Playing")
	int32 NumMapHasGenerated = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Playing")
	bool bMapHasGenerated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TSubclassOf<AGoPlayingAIController> AIControllerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TSubclassOf<AGoMapBuilder> MapBuilderClass;

	int32 GlobalObstacleId = 0;
};