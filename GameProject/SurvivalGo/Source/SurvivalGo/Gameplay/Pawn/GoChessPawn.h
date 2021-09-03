// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/Character.h"
#include "MapSettings.h"
#include "Misc/GameplayDefines.h"
#include "UINotifier.h"
#include "GoChessPawn.generated.h"

class FLifetimeProperty;
class UGoChessMovementComponent;

UCLASS(BlueprintType, Blueprintable)
class SURVIVALGO_API AGoChessPawn : public ACharacter, public IMapObstacleInterface, public IHealthNotifierInterface
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeOperationTransaction, EOperationTransactionState, TransactionState);
	//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInitialTotalHealth, int32, NewInitialTotalHealth);
	//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentHealth, int32, NewCurrentHealth);
public:
	AGoChessPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//// Start IMapObstacleInterface

	virtual EObstacleType GetObstacleType_Implementation() const override;

	virtual FMapCoordinate GetCoordinate_Implementation(bool bToBlockCoordinate) const override;

	virtual void ReceiveDamage_Implementation(AGoChessPawn* AttackSource, int32 Damage) override;

	virtual void DestroySelfIfNeeded_Implementation(int32 InstigatePlayer) override;

	//// End IMapObstacleInterface

	virtual UNetConnection* GetNetConnection() const override;

	UFUNCTION()
	virtual void OnRep_Coordinate();

public:

	UFUNCTION(BlueprintCallable, Category = "Playing")
	void SetIsChessMoving(bool bValue);

	UFUNCTION(BlueprintPure, Category = "Playing", meta = (BlueprintThreadSafe))
	bool IsChessMoving() const;


	UFUNCTION(BlueprintCallable, Category = "Chess")
	virtual void OnRep_InitialTotalHealth();

	UFUNCTION(BlueprintCallable, Category = "Chess")
	virtual void OnRep_CurrentHealth();


	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Playing")
	bool CheckBeforeAttack(const TScriptInterface<IMapObstacleInterface>& Target, const TArray<AGoChessPawn*>& WithOtherChesses);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Playing")
	void MoveOnceInView(const FTransform& FromTransform, const FTransform& ToTransform);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Playing")
	void MoveBatchInView(const FTransform& FromTransform, const TArray<FTransform>& ToTransforms);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Playing")
	void AttackInView(const TScriptInterface<IMapObstacleInterface>& Target, const TArray<AGoChessPawn*>& WithOtherChesses);

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Playing")
	int32 CalculateAttackValue(const TArray<AGoChessPawn*>& WithOtherChesses);

public:
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Playing")
	FOnChangeOperationTransaction OnChangeOperationTransactionDelegate;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Playing")
	int32 OwningPlayerIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Playing")
	int32 ChessIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Coordinate, Category = "Playing")
	FMapCoordinate Coordinate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_InitialTotalHealth, Category = "Chess")
	int32 InitialTotalHealth = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentHealth, Category = "Chess")
	int32 CurrentHealth = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	int32 AttackValue = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	int32 AttackActionCost = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	int32 MoveActionCost = 10;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chess")
	bool bIsLocallySelected = false;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chess")
	//bool bIsChessMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chess")
	UGoChessMovementComponent* ChessMovementComponent = nullptr;
};
