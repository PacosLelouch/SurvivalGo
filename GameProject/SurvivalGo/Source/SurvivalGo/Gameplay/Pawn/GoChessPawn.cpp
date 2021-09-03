// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GoChessPawn.h"
#include "GameState/GoPlayingGameState.h"
#include "GoMapBuilder.h"
#include "MapElementsRuntime.h"
#include "Net/UnrealNetwork.h"
#include "GoChessMovementComponent.h"

AGoChessPawn::AGoChessPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetReplicateMovement(false);
	bAlwaysRelevant = true;
	ChessMovementComponent = CreateDefaultSubobject<UGoChessMovementComponent>(TEXT("ChessMovementComponent"));
}

void AGoChessPawn::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* SkMeshComp = GetMesh();
	if (IsValid(SkMeshComp))
	{
		SkMeshComp->SetRenderCustomDepth(true);
		SkMeshComp->SetCustomDepthStencilValue(255 - OwningPlayerIndex);
	}

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		auto* GoGS = World->GetGameState<AGoPlayingGameState>();
		if (IsValid(GoGS))
		{
			GoGS->Chesses.Add(FIntPoint(OwningPlayerIndex, ChessIndex), this);
			AGoMapBuilder* MapBuilder = GoGS->GetMapBuilder();
			if (IsValid(MapBuilder))
			{
				//MapBuilder->AddObstacleMulticast_Implementation(this, Coordinate);
				MapBuilder->AddObstacle(this, Coordinate);
				MapBuilder->AddChessPawn(this, Coordinate);
			}
		}
	}
}

void AGoChessPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoChessPawn, OwningPlayerIndex);
	DOREPLIFETIME(AGoChessPawn, ChessIndex);
	DOREPLIFETIME(AGoChessPawn, Coordinate);
	DOREPLIFETIME(AGoChessPawn, InitialTotalHealth);
	DOREPLIFETIME(AGoChessPawn, CurrentHealth);

}

EObstacleType AGoChessPawn::GetObstacleType_Implementation() const
{
	return EObstacleType::Pawn;
}

FMapCoordinate AGoChessPawn::GetCoordinate_Implementation(bool bToBlockCoordinate) const
{
	return bToBlockCoordinate ? Coordinate.ToBlockCoordinate() : Coordinate;
}

void AGoChessPawn::ReceiveDamage_Implementation(AGoChessPawn* AttackSource, int32 Damage)
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

void AGoChessPawn::DestroySelfIfNeeded_Implementation(int32 InstigatePlayer)
{
	if (CurrentHealth <= 0)
	{
		DestroySelfInternal();
	}
}

UNetConnection* AGoChessPawn::GetNetConnection() const
{
	UNetConnection* SuperConnection = Super::GetNetConnection();
	return SuperConnection ? SuperConnection : UGameplayUtilLibrary::GetWorldNetConnection(this);
}

void AGoChessPawn::OnRep_Coordinate()
{
	UE_LOG(LogTemp, Verbose, TEXT("Coordinate = <%s>"), *Coordinate.Coordinate.ToString());
}

void AGoChessPawn::SetIsChessMoving(bool bValue)
{
	if (ChessMovementComponent)
	{
		ChessMovementComponent->SetIsMoving(bValue);
	}
}

bool AGoChessPawn::IsChessMoving() const
{
	return ChessMovementComponent ? ChessMovementComponent->bIsMoving : false;
}

void AGoChessPawn::OnRep_InitialTotalHealth()
{
}

void AGoChessPawn::OnRep_CurrentHealth()
{
	if (CurrentHealth <= 0 && InitialTotalHealth > 0)
	{
		RemoveSelfFromMap();
	}
}

bool AGoChessPawn::CheckBeforeAttack_Implementation(const TScriptInterface<IMapObstacleInterface>& Target, const TArray<AGoChessPawn*>& WithOtherChesses)
{
	if (!Target)
	{
		return false;
	}
	for (auto* Chess : WithOtherChesses)
	{
		if (!Chess || Chess->OwningPlayerIndex != OwningPlayerIndex)
		{
			return false;
		}
	}
	return true;
}

void AGoChessPawn::MoveOnceInView_Implementation(const FTransform& FromTransform, const FTransform& ToTransform)
{
	// Do nothing, and implement the logic in blueprint.
}

void AGoChessPawn::MoveBatchInView_Implementation(const FTransform& FromTransform, const TArray<FTransform>& ToTransforms)
{
	// Do nothing, and implement the logic in blueprint.
}

void AGoChessPawn::AttackInView_Implementation(const TScriptInterface<IMapObstacleInterface>& Target, const TArray<AGoChessPawn*>& WithOtherChesses)
{
	// Do nothing, and implement the logic in blueprint.
}

int32 AGoChessPawn::CalculateAttackValue_Implementation(const TArray<AGoChessPawn*>& WithOtherChesses)
{
	if (WithOtherChesses.Num() == 0)
	{
		return (AttackValue + 1) >> 1;
	}
	int32 TotalAttackValue = AttackValue;
	for (AGoChessPawn* Chess : WithOtherChesses)
	{
		TotalAttackValue += Chess->AttackValue;
	}
	return TotalAttackValue; //(TotalAttackValue >> 2);
}
