// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "YapBlueprintFunctionLibrary.generated.h"

struct FYapDialogueHandle;
struct FYapPromptHandle;

#define LOCTEXT_NAMESPACE "Yap"

/**
 * 
 */
UCLASS()
class YAP_API UYapBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#if WITH_EDITOR
	/** Built-in simple helper function to play an Unreal sound. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	static void PlaySoundInEditor(USoundBase* Sound);
#endif
	
	/** Built-in simple helper function to retrieve the length of an Unreal sound. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	static float GetSoundLength(USoundBase* Sound);

	UFUNCTION(BlueprintCallable, Category = "Yap")
	static bool SkipDialogue(const FYapDialogueHandle& Handle);

	UFUNCTION(BlueprintCallable, Category = "Yap")
	static bool RunPrompt(const FYapPromptHandle& Handle);

	UFUNCTION(BlueprintCallable, Category = "Yap")
	static void InvalidateDialogueHandle(UPARAM(ref) FYapDialogueHandle& Handle);
	
	UFUNCTION(BlueprintCallable, Category = "Yap")
	static void InvalidatePromptHandle(UPARAM(ref) FYapPromptHandle& Handle);
};


#undef LOCTEXT_NAMESPACE
