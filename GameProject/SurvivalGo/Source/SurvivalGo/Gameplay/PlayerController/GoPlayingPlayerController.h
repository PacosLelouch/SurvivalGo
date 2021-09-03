// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GoPlayingPlayerController.generated.h"

class AGoPlayingGameState;
class AGoPlayingGameMode;
class AGoChessPawn;
class AGoMapBuilder;
class UMapBlock;
class IMapObstacleInterface;

UCLASS(BlueprintType, Blueprintable)
class SURVIVALGO_API AGoPlayingPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AGoPlayingPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Playing|Map")
	AGoPlayingGameState* GetGoPlayingGameState();

	UFUNCTION(BlueprintPure, Category = "Playing|Map")
	AGoPlayingGameMode* GetGoPlayingGameMode();

	UFUNCTION(BlueprintPure, Category = "Playing|Map")
	AGoMapBuilder* GetMapBuilder();

	UFUNCTION(BlueprintPure, Category = "Playing")
	bool CanMoveToNextPhaseManually(bool bOnlyInActionPhase = true);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Playing")
	void TryToMoveToNextPhase();

	UFUNCTION(BlueprintCallable, Category = "Playing", meta = (AutoCreateRefTerm = "InHitResultIfTraced"))
	bool TraceObstacle(AActor*& TargetObstacle, bool& bOwnChess, const FHitResult& InHitResultIfTraced);

	UFUNCTION(BlueprintCallable, Category = "Playing", meta = (AutoCreateRefTerm = "InHitResultIfTraced"))
	bool TraceBlock(UMapBlock*& TargetBlock, const FHitResult& InHitResultIfTraced);

public:
	UFUNCTION(BlueprintCallable, Category = "Playing")
	bool TraceSingleGetHitResult(FHitResult& OutHitResult);

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Playing")
	AGoChessPawn* SelectedChess = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Playing")
	TScriptInterface<IMapObstacleInterface> SelectedObstacleForView = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playing")
	float TraceDistance = 1000000.f;

protected:
	TWeakObjectPtr<AGoPlayingGameState> CachedGoPlayingGS;
	TWeakObjectPtr<AGoPlayingGameMode> CachedGoPlayingGM;
};
