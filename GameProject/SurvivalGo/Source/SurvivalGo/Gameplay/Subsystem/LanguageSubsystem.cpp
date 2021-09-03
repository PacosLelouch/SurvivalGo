// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "LanguageSubsystem.h"
#include "JsonObjectConverter.h"
#include "GameInstance/GoGameInstance.h"
#include "GoGlobalDefines.h"

const FString ULanguageSubsystem::LanguageExtension = TEXT("csv");
const FString ULanguageSubsystem::LanguageBaseDirectory = GAME_SAVINGS_BASE_PATH / TEXT("Languages");
const FString ULanguageSubsystem::LanguageIndexFile = TEXT("LanguageIndex.json");
const ELanguageType ULanguageSubsystem::DefaultLanguageType = ELanguageType::Chinese;

ULanguageSubsystem::ULanguageSubsystem()
	: Super()
	, LanguageIndex
	{ {
		{
			ELanguageType::Chinese,
			{{
				MakeTuple(ELanguageDescriptionType::Common, TEXT("Chinese_Common")),
			}},
		},
		{
			ELanguageType::English,
			{{
				MakeTuple(ELanguageDescriptionType::Common, TEXT("English_Common")),
			}},
		},
	} }
{
}

void ULanguageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	//FString JsonString;
	//FJsonObjectConverter::UStructToJsonObjectString(LanguageIndex, JsonString);
	//FFileHelper::SaveStringToFile(JsonString, *(LanguageBaseDirectory / LanguageIndexFile), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	bLanguageLoaded = LoadLanguage();
}

void ULanguageSubsystem::Deinitialize()
{
	bLanguageLoaded = false;
	Super::Deinitialize();
}

bool ULanguageSubsystem::LoadLanguage()
{
	Language.Descriptions.Empty(1);
	FString LanguageIndexString;
	if (!FFileHelper::LoadFileToString(LanguageIndexString, *(LanguageBaseDirectory / LanguageIndexFile)))
	{
		return false;
	}
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(LanguageIndexString, &LanguageIndex, 0, 0))
	{
		return false;
	}

	TargetLanguageType = DefaultLanguageType;
	if (UWorld* World = GetWorld())
	{
		if (auto* GameInstance = World->GetGameInstance<UGoGameInstance>())
		{
			TargetLanguageType = GameInstance->LanguageType;
		}
	}
	auto& Description = Language.Descriptions.Add(TargetLanguageType);
	for (auto& FilePair : LanguageIndex.DescriptionIndices[TargetLanguageType].SegmentFileNames)
	{
		FString FileName = FPaths::SetExtension(FilePair.Value, LanguageExtension);
		FString CSVString;
		if (!FFileHelper::LoadFileToString(CSVString, *(LanguageBaseDirectory / FileName)))
		{
			continue;
		}
		UDataTable** DTPtr = Description.SegmentTables.Find(FilePair.Key);
		if (!DTPtr)
		{
			DTPtr = &Description.SegmentTables.Add(FilePair.Key, NewObject<UDataTable>(this));
		}
		(*DTPtr)->RowStruct = FLanguageSegment::StaticStruct();
		(*DTPtr)->CreateTableFromCSVString(CSVString);
		auto& RowMap = (*DTPtr)->GetRowMap();
		for (auto& Row : RowMap)
		{
			Description.AliasToRow.Add(reinterpret_cast<FLanguageSegment*>(Row.Value)->Alias, Row.Key);
		}
	}

	return true;
}

void ULanguageSubsystem::BatchSetWidgetTextByRowId(const TMap<UObject*, FName>& WidgetIdMap, FName SetTextFunctionName, ELanguageDescriptionType LanguageDescriptionType)
{
	for (auto& WidgetIdPair : WidgetIdMap)
	{
		UObject* WidgetObject = WidgetIdPair.Key;
		if (!IsValid(WidgetObject))
		{
			continue;
		}
		UFunction* Function = WidgetObject->FindFunction(SetTextFunctionName);
		if (!IsValid(Function))
		{
			continue;
		}
		FString Segment = GetSegmentByRowId(LanguageDescriptionType, WidgetIdPair.Value);
		FText SegmentText = FText::FromString(Segment);
		WidgetObject->ProcessEvent(Function, static_cast<void*>(&SegmentText));
	}
}

void ULanguageSubsystem::BatchSetWidgetTextByAlias(const TMap<UObject*, FName>& WidgetAliasMap, FName SetTextFunctionName, ELanguageDescriptionType LanguageDescriptionType)
{
	for (auto& WidgetAliasPair : WidgetAliasMap)
	{
		UObject* WidgetObject = WidgetAliasPair.Key;
		if (!IsValid(WidgetObject))
		{
			continue;
		}
		UFunction* Function = WidgetObject->FindFunction(SetTextFunctionName);
		if (!IsValid(Function))
		{
			continue;
		}
		FString Segment = GetSegmentByAlias(LanguageDescriptionType, WidgetAliasPair.Value);
		FText SegmentText = FText::FromString(Segment);
		WidgetObject->ProcessEvent(Function, static_cast<void*>(&SegmentText));
	}
}

FString ULanguageSubsystem::GetSegmentByRowId(ELanguageDescriptionType LanguageDescriptionType, FName RowId)
{
	auto* DescriptionPtr = Language.Descriptions.Find(TargetLanguageType);
	if (!DescriptionPtr)
	{
		return FString();
	}
	auto* DTPtr = (*DescriptionPtr).SegmentTables.Find(LanguageDescriptionType);
	if (!DTPtr)
	{
		return FString();
	}
	auto* RowPtr = (*DTPtr)->FindRow<FLanguageSegment>(RowId, TEXT("GetSegmentByRowId"), true);
	return RowPtr ? (*RowPtr).Segment : FString();
}

FString ULanguageSubsystem::GetSegmentByAlias(ELanguageDescriptionType LanguageDescriptionType, FName Alias)
{
	auto* DescriptionPtr = Language.Descriptions.Find(TargetLanguageType);
	if (!DescriptionPtr)
	{
		return FString();
	}
	FName* RowIdPtr = (*DescriptionPtr).AliasToRow.Find(Alias);
	if (!RowIdPtr)
	{
		return FString();
	}
	return GetSegmentByRowId(LanguageDescriptionType, *RowIdPtr);
}
