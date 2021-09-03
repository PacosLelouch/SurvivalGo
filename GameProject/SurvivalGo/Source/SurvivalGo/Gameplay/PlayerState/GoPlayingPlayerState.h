// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Misc/GameplayDefines.h"
#include "UINotifier.h"
#include "GoPlayingPlayerState.generated.h"

class FLifetimeProperty;

UCLASS(BlueprintType, Blueprintable)
class SURVIVALGO_API AGoPlayingPlayerState : public APlayerState, public IActionValueNotifierInterface
{
	GENERATED_BODY()
public:
	AGoPlayingPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintPure, Category = "Playing", meta = (ControllerClass = "AController", DeterminesOutputType = "ControllerClass"))
	AController* GetOwningController(TSubclassOf<AController> ControllerClass);

	//UFUNCTION(BlueprintPure, Category = "Playing")
	//AController* GetOwningController();

	UFUNCTION(BlueprintPure, Category = "Playing")
	int32 GetPlayerIndex(bool bGetParentIfNoOverride = true);

	UFUNCTION(BlueprintCallable, Category = "Playing")
	void ReceiveMoveToNextPhase(int32 Round, int32 PlayerTurn, ERoundPhase RoundPhase);

	void RecoverActionValue();

	virtual void OnRep_PlayerId() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Playing")
	void OnRep_CurrentActionValue();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Playing")
	void OnRep_OverridePlayerIndex();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_OverridePlayerIndex, Category = "Playing")
	int32 OverridePlayerIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Playing")
	int32 TotalActionValue = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentActionValue, Category = "Playing")
	int32 CurrentActionValue = 30;

	UPROPERTY(VisibleAnywhere, Category = "Playing")
	AController* OwningController = nullptr;
};
