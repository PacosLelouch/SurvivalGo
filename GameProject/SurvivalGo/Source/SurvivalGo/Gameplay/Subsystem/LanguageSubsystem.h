// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "Engine/DataTable.h"
#include "LanguageSubsystem.generated.h"

UENUM(BlueprintType)
enum class ELanguageType : uint8
{
	Chinese,
	English,
};

UENUM(BlueprintType)
enum class ELanguageDescriptionType : uint8
{
	Common,
};

USTRUCT(BlueprintType)
struct SURVIVALGO_API FLanguageSegment : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Language")
	FName Alias;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Language")
	FString Segment;
};

USTRUCT(BlueprintType)
struct SURVIVALGO_API FLanguageDescription
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Language")
	TMap<ELanguageDescriptionType, UDataTable*> SegmentTables;

	UPROPERTY(BlueprintReadOnly, Category = "Language")
	TMap<FName, FName> AliasToRow;
};

USTRUCT(BlueprintType)
struct SURVIVALGO_API FLanguage
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Language")
	TMap<ELanguageType, FLanguageDescription> Descriptions;
};


USTRUCT(BlueprintType)
struct SURVIVALGO_API FLanguageDescriptionIndex
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Language")
	TMap<ELanguageDescriptionType, FString> SegmentFileNames;
};

USTRUCT(BlueprintType)
struct SURVIVALGO_API FLanguageIndex
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Language")
	TMap<ELanguageType, FLanguageDescriptionIndex> DescriptionIndices;
};

UCLASS(BlueprintType)
class SURVIVALGO_API ULanguageSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	ULanguageSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	bool LoadLanguage();

	UFUNCTION(BlueprintCallable, Category = "Language")
	void BatchSetWidgetTextByRowId(const TMap<UObject*, FName>& WidgetIdMap, FName SetTextFunctionName = TEXT("SetText"), ELanguageDescriptionType LanguageDescriptionType = ELanguageDescriptionType::Common);

	UFUNCTION(BlueprintCallable, Category = "Language")
	void BatchSetWidgetTextByAlias(const TMap<UObject*, FName>& WidgetAliasMap, FName SetTextFunctionName = TEXT("SetText"), ELanguageDescriptionType LanguageDescriptionType = ELanguageDescriptionType::Common);

	UFUNCTION(BlueprintCallable, Category = "Language")
	FString GetSegmentByRowId(ELanguageDescriptionType LanguageDescriptionType = ELanguageDescriptionType::Common, FName RowId = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Language")
	FString GetSegmentByAlias(ELanguageDescriptionType LanguageDescriptionType = ELanguageDescriptionType::Common, FName Alias = NAME_None);

public:
	UPROPERTY(BlueprintReadWrite, Category = "Language")
	FLanguage Language;

	UPROPERTY(BlueprintReadOnly, Category = "Language")
	FLanguageIndex LanguageIndex;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Language")
	ELanguageType TargetLanguageType;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Language")
	bool bLanguageLoaded = false;

protected:
	static const FString LanguageExtension;
	static const FString LanguageIndexFile;
	static const FString LanguageBaseDirectory;
	static const ELanguageType DefaultLanguageType;
};
