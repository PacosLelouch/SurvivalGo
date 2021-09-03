// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoPlayerPawn.generated.h"

class AGoChessPawn;

UCLASS(BlueprintType, Blueprintable)
class SURVIVALGO_API AGoPlayerPawn : public APawn
{
	GENERATED_BODY()
public:
	AGoPlayerPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:

};
