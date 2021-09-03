// Copyright 2021 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "UObject/Interface.h"
#include "UINotifier.generated.h"


UINTERFACE(BlueprintType)
class UHealthNotifierInterface : public UInterface
{
	GENERATED_BODY()
};

class SURVIVALGO_API IHealthNotifierInterface
{
	GENERATED_BODY()
public:

	// Notice that the UI may have to change after some time, it is necessary to provide an extra function to notify UI to update.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "UINotifier|Health")
	void NotifyHealthViewToUpdate();
};



UINTERFACE(BlueprintType)
class UActionValueNotifierInterface : public UInterface
{
	GENERATED_BODY()
};

class SURVIVALGO_API IActionValueNotifierInterface
{
	GENERATED_BODY()
public:

	// Notice that the UI may have to change after some time, it is necessary to provide an extra function to notify UI to update.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "UINotifier|Action")
	void NotifyActionValueViewToUpdate();
};

