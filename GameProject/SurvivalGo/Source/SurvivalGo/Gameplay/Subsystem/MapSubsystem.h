// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MapSettings.h"
#include "MapSubsystem.generated.h"

class UGoGameInstance;
class AGoMapBuilder;

UCLASS(BlueprintType)
class SURVIVALGO_API UMapSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMapSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "GoMap|Settings")
	bool LoadMap(const FString& InSettingsName);

	UFUNCTION(BlueprintPure, Category = "Playing|Map")
	AGoMapBuilder* GetMapBuilder();

public:
	void RemoveObstacleFromMap(AActor* Obstacle);

	static UMapSubsystem* GetSubsystemInternal(const UObject* WorldContextObject = nullptr);

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	FMapSettings MapSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	FString MapSettingsName;

	UGoGameInstance* GoGameInstance = nullptr;
};
