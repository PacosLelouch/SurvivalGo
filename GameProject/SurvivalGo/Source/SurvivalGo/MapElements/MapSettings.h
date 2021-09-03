// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "Misc/Crc.h"
#include "UObject/Interface.h"
#include "MapSettings.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogMapSettings, Log, All)

class AGoChessPawn;

UENUM(BlueprintType)
enum class EObstacleType : uint8
{
	NotObstacle,
	Pawn,
	BreakableWall,
	NonBreakableObstacle,
};

UENUM(BlueprintType)
enum class EPawnDirection : uint8
{
	None,
	Up,
	Right,
	Down,
	Left,
};

UENUM(BlueprintType)
enum class EBlockType : uint8
{
	None,
	Walkable,
};

USTRUCT(BlueprintType)
struct SURVIVALGO_API FMapCoordinate
{
	GENERATED_BODY()
public:
	FMapCoordinate() {}

	FMapCoordinate(const FIntVector& IntVector, EPawnDirection InDirection = EPawnDirection::None);

	bool operator==(const FMapCoordinate& Other) const;

	FMapCoordinate ToBlockCoordinate() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	FIntVector Coordinate = FIntVector::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	EPawnDirection Direction = EPawnDirection::None;
};

FORCEINLINE uint32 GetTypeHash(const FMapCoordinate& Coordinate)
{
	return GetTypeHash(Coordinate.Coordinate) + GetTypeHash(Coordinate.Direction);
}

template <> struct TIsPODType<FMapCoordinate> { enum { Value = true }; };

USTRUCT(BlueprintType)
struct SURVIVALGO_API FPlayerPawnSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	int32 PlayerId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	int32 DefaultChessLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	FMapCoordinate Coordinate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	EObstacleType ObstacleType = EObstacleType::Pawn;
};

USTRUCT(BlueprintType)
struct SURVIVALGO_API FMapBlockSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	FMapCoordinate Coordinate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	EBlockType BlockType = EBlockType::Walkable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TArray<EObstacleType> ObstacleSettings;

public:
	static bool IsValidObstacleTypeInSettings(EObstacleType ObstacleType);
};

USTRUCT(BlueprintType)
struct SURVIVALGO_API FMapSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TArray<FPlayerPawnSettings> ChessDataSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	TArray<FMapBlockSettings> BlockDataSettings;
	//TMap<FMapCoordinate, FMapBlockSettings> BlockDataSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoMap|Settings")
	float BlockLength = 250.f;
};

UINTERFACE(BlueprintType)
class UMapObstacleInterface : public UInterface
{
	GENERATED_BODY()
};

class SURVIVALGO_API IMapObstacleInterface
{
	GENERATED_BODY()
protected:
	void DestroySelfInternal();

	void RemoveSelfFromMap();

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GoMap|Obstacle")
	EObstacleType GetObstacleType() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GoMap|Obstacle")
	FMapCoordinate GetCoordinate(bool bToBlockCoordinate) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Playing")
	void ReceiveDamage(AGoChessPawn* AttackSource, int32 Damage);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Playing")
	void DestroySelfIfNeeded(int32 InstigatePlayer);
};
