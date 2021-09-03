// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GoPlayingAIController.generated.h"


UCLASS(BlueprintType, Blueprintable)
class AGoPlayingAIController : public AAIController
{
	GENERATED_BODY()
public:
	AGoPlayingAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Playing|AI")
	bool bActivateAsPlayer = false;
};
