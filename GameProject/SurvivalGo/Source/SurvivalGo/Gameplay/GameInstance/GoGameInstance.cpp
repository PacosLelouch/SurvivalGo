// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GoGameInstance.h"
#include "Misc/Paths.h"
#include "Subsystem/LanguageSubsystem.h"
#include "GoGlobalDefines.h"

UGoGameInstance::UGoGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MapSettingsBasePath = GAME_SAVINGS_BASE_PATH / TEXT("MapSettings");
}

void UGoGameInstance::Init()
{
	Super::Init();
	if (GEngine)
	{
		auto* LanguageSubsystem = GEngine->GetEngineSubsystem<ULanguageSubsystem>();
		if (LanguageSubsystem)// && !LanguageSubsystem->bLanguageLoaded)
		{
			LanguageSubsystem->bLanguageLoaded = LanguageSubsystem->LoadLanguage();
		}
	}
}

void UGoGameInstance::Shutdown()
{
	Super::Shutdown();
}
