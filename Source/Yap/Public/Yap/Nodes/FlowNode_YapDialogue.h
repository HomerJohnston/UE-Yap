// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Nodes/FlowNode.h"
#include "Yap/Handles/YapRunningFragment.h"
#include "Yap/YapFragment.h"
#include "Yap/Handles/YapPromptHandle.h"
#include "Yap/Handles/YapSpeechHandle.h"

#include "FlowNode_YapDialogue.generated.h"

class UYapCharacter;

// ------------------------------------------------------------------------------------------------
/**
 * Determines how a Talk node evaluates. Player Prompt nodes don't use this.
 */
UENUM()
enum class EYapDialogueTalkSequencing : uint8
{
	RunAll				UMETA(ToolTip = "The node will always try to run every fragment. The node will execute the Out pin after it finishes trying to run all fragments."), 
	RunUntilFailure		UMETA(ToolTip = "The node will attempt to run every fragment. If any one fails, the node will execute the Out pin."),
	SelectOne			UMETA(ToolTip = "The node will attempt to run every fragment. If any one passes, the node will execute the Out pin."),
	COUNT				UMETA(Hidden)
};

// ------------------------------------------------------------------------------------------------
/**
 * Node type. Freestyle talking or player prompt. Changes the execution flow of dialogue.
 */
UENUM()
enum class EYapDialogueNodeType : uint8
{
	Talk,
	PlayerPrompt,
	COUNT				UMETA(Hidden)
};

USTRUCT()
struct FYapFragmentRunData
{
	GENERATED_BODY()
	
	FYapFragmentRunData() {}

	FYapFragmentRunData(uint8 InFragmentIndex, FTimerHandle InSpeechTimerHandle, FTimerHandle InPaddingTimerHandle)
		: FragmentIndex(InFragmentIndex)
		, SpeechTimerHandle(InSpeechTimerHandle)
		, PaddingTimerHandle(InPaddingTimerHandle)
	{}

	UPROPERTY(Transient)
	int32 FragmentIndex = INDEX_NONE;

	UPROPERTY(Transient)
	FTimerHandle SpeechTimerHandle;
	
	UPROPERTY(Transient)
	FTimerHandle PaddingTimerHandle;
};

// ------------------------------------------------------------------------------------------------
/**
 * Emits Dialogue through UYapSubsystem.
 */
UCLASS(NotBlueprintable, BlueprintType, meta = (DisplayName = "Dialogue", Keywords = "yap")) /*, ToolTip = "Emits Yap dialogue events"*/
class YAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class SYapConditionDetailsViewWidget;
	friend class UFlowGraphNode_YapDialogue;
#endif

	// TODO should I get rid of this?
	friend class UYapSubsystem;

public:
	UFlowNode_YapDialogue();

	// TODO constexpr?
	static FName OutputPinName;
	
	static FName BypassPinName;
	
	// ============================================================================================
	// SETTINGS
	// ============================================================================================
	
protected:
	/** What type of node we are. */
	UPROPERTY(BlueprintReadOnly)
	EYapDialogueNodeType DialogueNodeType;

	/** What is this dialogue's type-group? Leave unset to use the default type-group. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag TypeGroup;
	
	/** Maximum number of times we can successfully enter & exit this node. Any further attempts will trigger the Bypass output. */
	UPROPERTY(BlueprintReadOnly)
	int32 NodeActivationLimit;

	/** Controls how Talk nodes flow. See EYapDialogueTalkSequencing. */
	UPROPERTY(BlueprintReadOnly)
	EYapDialogueTalkSequencing TalkSequencing;

	/** Controls if dialogue can be interrupted. */
	UPROPERTY(BlueprintReadOnly)
	TOptional<bool> Skippable;

	/** Controls if dialogue automatically advances (only applicable if it has a time duration set). */
	UPROPERTY(BlueprintReadOnly)
	TOptional<bool> AutoAdvance;

	/** Tags can be used to interact with this dialogue node during the game. Dialogue nodes can be looked up and/or modified by UYapSubsystem by their tag. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;

	/** Conditions which must be met for this dialogue to run. All conditions must pass (AND, not OR evaluation). If any conditions fail, Bypass output is triggered. */
	UPROPERTY(Instanced, BlueprintReadOnly)
	TArray<TObjectPtr<UYapCondition>> Conditions;

	/** Unique node ID for audio system. */
	UPROPERTY(EditAnywhere)
	FString AudioID;
	
	/** Actual dialogue contents. */
	UPROPERTY(EditAnywhere)
	TArray<FYapFragment> Fragments;

	// ============================================================================================
	// STATE
	// ============================================================================================
	
protected:
	/** How many times this node has been successfully ran. */
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 NodeActivationCount = 0;

	/**  */
	UPROPERTY(Transient)
	int32 RunningFragmentIndex = INDEX_NONE;

	/**  */
	UPROPERTY(Transient)
	TMap<FYapPromptHandle, uint8> PromptIndices;

	/**  */
	UPROPERTY(Transient)
	FYapSpeechHandle RunningSpeechHandle;
	
	// ============================================================================================
	// PUBLIC API
	// ============================================================================================

public:
	/** Is this dialogue a Talk node or a Player Prompt node? */
	bool IsPlayerPrompt() const { return DialogueNodeType == EYapDialogueNodeType::PlayerPrompt; }

	/** What type-group is this dialogue node? Different type groups can have different playback settings, and be handled by different registered listeners. */
	const FGameplayTag& GetTypeGroupTag() const { return TypeGroup; }
	
	/** Does this node use title text? */
	bool UsesTitleText() const;

	/** How many times has this dialogue node successfully ran? */
	int32 GetNodeActivationCount() const { return NodeActivationCount; }

	/** How many times is this dialogue node allowed to successfully run? */
	int32 GetNodeActivationLimit() const { return NodeActivationLimit; }

	const FYapFragment& GetFragment(uint8 FragmentIndex) const;
	
	/** Dialogue fragments getter. */
	const TArray<FYapFragment>& GetFragments() const { return Fragments; }

	/** Simple helper function. */
	uint8 GetNumFragments() const { return Fragments.Num(); }

	/** Is dialogue from this node skippable by default? */
	bool GetSkippable() const;

	bool GetNodeAutoAdvance() const;
	
	bool GetFragmentAutoAdvance(uint8 FragmentIndex) const;

	int32 GetRunningFragmentIndex() const { return RunningFragmentIndex; }
	
	// TODO this sucks can I register the fragments some other way instead
	/** Finds the first fragment on this dialogue containing a tag. */
	FYapFragment* FindTaggedFragment(const FGameplayTag& Tag);

protected:
	bool CanSkip(FYapSpeechHandle Handle) const;

public:
	FString GetAudioID() const { return AudioID; }

protected:
	bool CheckActivationLimits() const;

#if WITH_EDITOR
private:
	TOptional<bool> GetSkippableSetting() const;
	
	void InvalidateFragmentTags();

	const TArray<UYapCondition*>& GetConditions() const { return Conditions; }
	
	TArray<UYapCondition*>& GetConditionsMutable() { return MutableView(Conditions); }

	void ToggleNodeType();
#endif
	
	// ============================================================================================
	// OVERRIDES
	// ============================================================================================

protected:
	/** UFlowNodeBase override */
	void InitializeInstance() override;

	/** UFlowNodeBase override */
	void ExecuteInput(const FName& PinName) override;

	/** UFlowNode override */
	void OnPassThrough_Implementation() override;

	// ============================================================================================
	// INTERNAL API
	// ============================================================================================
	
protected:
	bool CanEnterNode();

	bool CheckConditions();

	bool TryBroadcastPrompts();

	void RunPrompt(uint8 Uint8);
	
	bool TryStartFragments();

	bool RunFragment(uint8 FragmentIndex);

	void OnSpeechComplete(uint8 FragmentIndex);

	void OnProgressionComplete(uint8 FragmentIndex);

	void AdvanceFromFragment(uint8 FragmentIndex);
	
	bool IsBypassPinRequired() const;

	bool IsOutputConnectedToPromptNode() const;
	
	int16 FindFragmentIndex(const FGuid& InFragmentGuid) const;

protected:
	bool FragmentCanRun(uint8 FragmentIndex);
	
	const FYapFragment& GetFragmentByIndex(uint8 Index) const;
	
	UFUNCTION()
	void OnPromptChosen(UObject* Instigator, FYapPromptHandle Handle);
	
	UFUNCTION()
	void OnSkipAction(UObject* Instigator, FYapSpeechHandle Handle);
	
#if WITH_EDITOR
public:
	TArray<FYapFragment>& GetFragmentsMutable();
	
private:
	FYapFragment& GetFragmentByIndexMutable(uint8 Index);
	
	void RemoveFragment(int32 Index);

	FText GetNodeTitle() const override;
	
	bool CanUserAddInput() const override { return false; }

	bool CanUserAddOutput() const override { return false; }

	bool SupportsContextPins() const override;
	
	bool GetUsesMultipleInputs();
	
	bool GetUsesMultipleOutputs();

	EYapDialogueTalkSequencing GetMultipleFragmentSequencing() const;
	
	TArray<FFlowPin> GetContextOutputs() const override;

	void SetNodeActivationLimit(int32 NewValue);

	void CycleFragmentSequencingMode();
	
	void DeleteFragmentByIndex(int16 DeleteIndex);
	
	void UpdateFragmentIndices();

	void SwapFragments(uint8 IndexA, uint8 IndexB);
	
public:
	FString GetNodeDescription() const override;

	const FGameplayTag& GetDialogueTag() const { return DialogueTag; }
	
	void OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const;
	
	void ForceReconstruction();

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	virtual void PostEditImport() override;
	
	virtual bool CanRefreshContextPinsDuringLoad() const { return true; }
	
	FText GetNodeToolTip() const override { return FText::GetEmpty(); };

	void PostLoad() override;
	
	void PreloadContent() override;
#endif // WITH_EDITOR
};
