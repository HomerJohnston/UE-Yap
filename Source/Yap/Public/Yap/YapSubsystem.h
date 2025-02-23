// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "YapConversation.h"
#include "YapCharacterComponent.h"
#include "YapBroker.h"
#include "YapPromptHandle.h"
#include "Enums/YapMaturitySetting.h"
#include "Yap/YapRunningFragment.h"
#include "Yap/YapBitReplacement.h"
#include "Yap/YapDataStructures.h"

#include "YapSubsystem.generated.h"

class UYapConversationHandler;
class UYapBroker;
class UFlowNode_YapDialogue;
struct FYapPromptHandle;
class UFlowAsset;
class IYapConversationHandler;
struct FYapBit;
class UYapCharacterComponent;
enum class EYapMaturitySetting : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlowYapConversationEvent, FName, ConversationName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFlowYapDialogueEvent, FName, ConversationName, const FYapBit&, DialogueInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlowYapOpenConversation, const FGameplayTag&, ConversationName);
DECLARE_DYNAMIC_DELEGATE(FFlowYapConversationFinishedOpening);

// ================================================================================================

UENUM()
enum class EYapCloseConversationResult : uint8
{
	Undefined,
	Failed,
	Closing,
	Closed,
};

// ================================================================================================

UCLASS()
class YAP_API UYapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

friend class UFlowNode_YapDialogue;
friend struct FYapFragment;
friend struct FYapPromptHandle;
	
public:
	UYapSubsystem();
	
	// =========================================
	// STATE
	// =========================================
protected:

	/**  */
	static TWeakObjectPtr<UWorld> World;
	
	/** All registered conversation handlers. It is assumed developers will only have one or two of these at a time, no need for fast lookup. Calling order will be preserved in order of registration. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> ConversationHandlers;

	/** All registered free speech handlers. It is assumed developers will only have one or two of these at a time, no need for fast lookup. Calling order will be preserved in order of registration. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> FreeSpeechHandlers;

	/** The broker object. Active only during play. Editor work uses the CDO instead. */
	UPROPERTY(Transient)
	TObjectPtr<UYapBroker> Broker;

	UPROPERTY(Transient)
	FGameplayTag ActiveConversationName;
	
	/** Queue of conversations. The top one is always going to be "active". If two "Open Conversation" nodes run, the second one will wait in this queue until the first one closes. */
	UPROPERTY(Transient)
	TArray<FYapConversation> ConversationQueue;
	
	/** Stores the tag of a fragment and the owning dialogue node where that fragment can be found */
	UPROPERTY(Transient)
	TMap<FGameplayTag, UFlowNode_YapDialogue*> TaggedFragments;

	/** Stores overrides of bit replacements. Currently, can only store one at a time per fragment; new assignments simply replace the old one. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FYapBitReplacement> BitReplacements;

	/** All registered character components. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<UYapCharacterComponent>> YapCharacterComponents;

	/** Helper to ensure that multiple character components are never registered for the same actor. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> RegisteredYapCharacterActors;

	// Running dialogue instances. Since only one fragment of a dialogue node can be running at any time, we don't need handles to map to individual fragments.
	UPROPERTY(Transient)
	TMap<FYapFragmentHandle, TWeakObjectPtr<UFlowNode_YapDialogue>> GuidDialogueMap;

	/** A skip command will flush all of these. We use a stack because Yap allows multiple fragments to run simulataneously (through negative padding). */
	UPROPERTY(Transient)
	TMap<FYapFragmentHandle, FYapRunningFragment> RunningFragments;

	TMap<FYapFragmentHandle, FYapRunningSpeech> RunningSpeech;
	
	static bool bGetGameMaturitySettingWarningIssued;

	static FYapRunningFragment InvalidHandle;
	
	// =========================================
	// PUBLIC API - Your game should use these
	// =========================================
public:
	
	/** Register a conversation handler. Conversation handlers will receive yap dialogue events. Must implement IYapConversationHandler either in C++ or BP. */
	static void RegisterConversationHandler(UObject* NewHandler);

	/** Unregister a conversation handler. */
	static void UnregisterConversationHandler(UObject* HandlerToRemove);
	
	/** Register a conversation handler. Conversation handlers will receive yap dialogue events. Must implement IYapConversationHandler either in C++ or BP. */
	static void RegisterFreeSpeechHandler(UObject* NewHandler);

	/** Register a conversation handler. Conversation handlers will receive yap dialogue events. Must implement IYapConversationHandler either in C++ or BP. */
	static void UnregisterFreeSpeechHandler(UObject* HandlerToRemove);

	/** Given a character identity tag, find the character component in the world. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	static UYapCharacterComponent* FindCharacterComponent(FGameplayTag CharacterTag);

	// =========================================
	// YAP API - These are called by Yap classes
	// =========================================
public:

	static const TWeakObjectPtr<UWorld> GetStaticWorld()
	{
		return World;
	}

	static UYapSubsystem* Get()
	{
		if (World.IsValid())
		{
			return World->GetSubsystem<UYapSubsystem>();
		}

		return nullptr;
	}
	
public:
	static UYapBroker* GetBroker();
	
	static EYapMaturitySetting GetCurrentMaturitySetting();

	/**  */
	FYapFragment* FindTaggedFragment(const FGameplayTag& FragmentTag);

protected:  // TODO should some of these be public?
	/**  */
	void RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode);

public:
	/**  */
	FYapConversation& OpenConversation(const FGameplayTag& ConversationName); // Called by Open Conversation node

	EYapCloseConversationResult RequestCloseConversation(const FGameplayTag& ConversationName);

protected:
	void StartOpeningConversation(FYapConversation& Conversation);
	/**  */
	EYapCloseConversationResult StartClosingConversation(const FGameplayTag& ConversationName); // Called by Close Conversation node

	void StartNextQueuedConversation();
	
	
	/**  */
	FYapPromptHandle BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex);

	/**  */
	void OnFinishedBroadcastingPrompts();

public:
	/**  */
	UFUNCTION(BlueprintCallable)
	FYapFragmentHandle RunSpeech(const FYapData_SpeechBegins& SpeechData);
	
	/**  */
	FGameplayTag GetConversation(UFlowAsset* FlowAsset);
	
	/**  */
	static FYapConversation& GetConversation(FYapConversationHandle Handle);

	static FYapConversation& GetConversation(const FGameplayTag& ConversationName);
	
public:
	// TODO should I make a ref struct for FYapPromptHandle too?
	/** The prompt handle will call this function, passing in itself. */
	static bool RunPrompt(const FYapPromptHandle& Handle);

	/**  */
	static bool SkipDialogue(const FYapFragmentHandle& Handle);

	/**  */ // TODO: ability to instantly playback/skip through multiple nodes until some sort of target point is hit, maybe a custom node? (imagine skipping an entire cutscene)
	// static bool SkipDialogueTo(???);

	static FYapRunningFragment& GetFragmentHandle(FYapFragmentHandle HandleRef);

public:
	/**  */
	void RegisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);

	/**  */
	void UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);
	
public:
	/**  */
	void Initialize(FSubsystemCollectionBase& Collection) override;

	/**  */
	void Deinitialize() override;

	/**  */
	void OnWorldBeginPlay(UWorld& InWorld) override;
	
protected:
	void OnSpeechComplete(FYapFragmentHandle Handle);

	void OnFragmentComplete(FYapFragmentHandle Handle);
	
	/**  */
	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	// Thanks to Blue Man for template help
	template<typename TUInterface, typename TIInterface, auto TFunction, auto TExecFunction, typename... TArgs>
	void BroadcastEventHandlerFunc(TArray<TObjectPtr<UObject>>& HandlersArray, TArgs&&... Args)
	{
		bool bHandled = false;
	
		for (int i = 0; i < HandlersArray.Num(); ++i)
		{
			UObject* HandlerObj = HandlersArray[i];

			if (!IsValid(HandlerObj))
			{
				continue;
			}
		
			if (TIInterface* CppInterface = Cast<TIInterface>(HandlerObj))
			{
				(CppInterface->*TFunction)(Args...);
			}
			else
			{
				check(HandlerObj->Implements<TUInterface>());
				(*TExecFunction)(HandlerObj, Args...);				
			}
		
			bHandled = true;
		}
	
		if (!bHandled)
		{
			UE_LOG(LogYap, Error, TEXT("No Yap Conversation Listeners are currently registered! You must inherit a class from IYapConversationListeners, implement its functions, and register it to the Yap subsystem."));
		}
	}
};