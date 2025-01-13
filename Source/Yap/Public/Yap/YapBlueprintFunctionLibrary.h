// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "YapBlueprintFunctionLibrary.generated.h"

struct FYapPromptHandle;

#define LOCTEXT_NAMESPACE "Yap"

/**
 * 
 */
UCLASS()
class YAP_API UYapBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Yap Audio")
	static void PlaySoundInEditor(USoundBase* Sound);

	UFUNCTION(BlueprintCallable, Category = "Yap Audio")
	static float GetSoundLength(USoundBase* Sound);
};

#undef LOCTEXT_NAMESPACE