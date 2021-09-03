// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MapSettings.h"
#include "Subsystem/LanguageSubsystem.h"
#include "GoGameInstance.generated.h"

class AGoChessPawn;
class ANonPlayerObstacle;
class UStaticMesh;
class UMaterialInterface;

UCLASS(BlueprintType, Blueprintable)
class SURVIVALGO_API UGoGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UGoGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Init() override;

	virtual void Shutdown() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings", meta = (MustImplement = "MapObstacleInterface"))
	TMap<EObstacleType, TSoftClassPtr<ANonPlayerObstacle>> ObstacleSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TMap<EBlockType, TSoftObjectPtr<UStaticMesh>> BlockMeshSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings", meta = (MustImplement = "MapObstacleInterface"))
	TArray<TSoftClassPtr<AGoChessPawn>> ChessClassSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TArray<FLinearColor> PlayerChessColorFlagSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	FString MapSettingsBasePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global|Settings")
	ELanguageType LanguageType = ELanguageType::Chinese;

public:
	// Include listen server. So if HostClientNum == 1, then play in a standalone device.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	int32 HostClientNum = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	int32 HostTotalPlayerNum = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	int32 HostAIPlayerNum = 2;
};

