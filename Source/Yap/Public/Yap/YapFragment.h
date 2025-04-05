// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "YapBit.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "Nodes/FlowPin.h"

#include "YapFragment.generated.h"

enum class EYapLoadContext : uint8;
class UYapCharacter;
class UYapCondition;
class UFlowNode_YapDialogue;
struct FFlowPin;
enum class EYapMaturitySetting : uint8;

// ================================================================================================

UENUM()
enum class EYapFragmentRunState : uint8
{
	Idle		= 0,
	Running		= 1,
	InPadding	= 2,
};

// ================================================================================================

UENUM()
enum class EYapFragmentEntryStateFlags : uint8
{
	NeverRan =	0,
	Failed =	1 << 0,
	Success =	1 << 1,
	Skipped =	1 << 2,
};

inline EYapFragmentEntryStateFlags operator|(EYapFragmentEntryStateFlags Left, EYapFragmentEntryStateFlags Right)
{
	return static_cast<EYapFragmentEntryStateFlags>(static_cast<uint8>(Left) | static_cast<uint8>(Right));
}

// ================================================================================================

/**
 * Fragments contain all of the actual data and settings required for a segment of speech to run.
 * 
 * Fragment settings override any defaults provided by the parent node.
 */
USTRUCT(NotBlueprintType)
struct YAP_API FYapFragment
{
	GENERATED_BODY()

public:
	FYapFragment();

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class UFlowGraphNode_YapDialogue;
	friend class SYapDialogueEditor;
#endif
	
	// ==========================================
	// SETTINGS
protected:
	UPROPERTY()
	TArray<TObjectPtr<UYapCondition>> Conditions;
	
	/**  */
	UPROPERTY()
	TSoftObjectPtr<UYapCharacter> SpeakerAsset;

	/**  */
	UPROPERTY()
	TSoftObjectPtr<UYapCharacter> DirectedAtAsset;
	
	UPROPERTY()
	FYapBit MatureBit;

	UPROPERTY()
	FYapBit ChildSafeBit;
	
	/** How many times is this fragment allowed to broadcast? This count persists only within this flow asset's lifespan (resets every Start). */
	UPROPERTY()
	int32 ActivationLimit = 0;
	
	UPROPERTY()
	FGameplayTag FragmentTag;

	/** Padding is idle time to wait after the fragment finishes running. An unset value will use project defaults. */
	UPROPERTY()
	TOptional<float> Padding;
	
	/**  */
	UPROPERTY()
	TOptional<bool> Skippable;
	
	/**  */
	UPROPERTY()
	TOptional<bool> AutoAdvance;
	
	/** Indicates whether child-safe data is available in this bit or not */
	UPROPERTY()
	bool bEnableChildSafe = false;
	
	UPROPERTY()
	bool bShowOnStartPin = false;

	UPROPERTY()
	bool bShowOnEndPin = false;
	
	/**  */
	UPROPERTY()
	FGameplayTag MoodTag;

	UPROPERTY(EditAnywhere)
	TArray<FInstancedStruct> Data;
	
	/**  */
	UPROPERTY()
	EYapTimeMode TimeMode;
	
	// ==========================================
	// STATE
protected:

	UPROPERTY(VisibleAnywhere, meta=(IgnoreForMemberInitializationTest))
	FGuid Guid;
	
	// TODO should this be serialized or transient
	UPROPERTY(Transient)
	uint8 IndexInDialogue = 0; 

	UPROPERTY(Transient)
	int32 ActivationCount = 0;
	
	UPROPERTY()
	FFlowPin PromptPin;

	UPROPERTY()
	FFlowPin StartPin;

	UPROPERTY()
	FFlowPin EndPin;

	UPROPERTY(Transient)
	EYapFragmentRunState RunState = EYapFragmentRunState::Idle;

	UPROPERTY(Transient)
	EYapFragmentEntryStateFlags LastEntryState = EYapFragmentEntryStateFlags::NeverRan;

	/** When was the current running fragment started? */
	UPROPERTY(Transient)
	double StartTime = -1;

	/** When did the most recently ran fragment finish? */
	UPROPERTY(Transient)
	double EndTime = -1;

	/**  */
	UPROPERTY(Transient)
	bool bRunning = false;
	
	/**  */
	UPROPERTY(Transient)
	bool bFragmentAwaitingManualAdvance = false;

public:
	/**  */
	UPROPERTY(Transient)
	FTimerHandle SpeechTimerHandle;

	/**  */
	UPROPERTY(Transient)
	FTimerHandle ProgressionTimerHandle;
	
	// ASSET LOADING
protected:
	
	TSharedPtr<FStreamableHandle> SpeakerHandle;
	
	TSharedPtr<FStreamableHandle> DirectedAtHandle;

	// EDITOR
#if WITH_EDITOR
protected:
#endif
	
	// ==========================================
	// API
public:
	bool CanRun() const;
	
	bool CheckConditions() const;
	
	void ResetOptionalPins();
	
	void PreloadContent(EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext);
	
	const UYapCharacter* GetSpeaker(EYapLoadContext LoadContext); // Non-const because of async loading handle

	const UYapCharacter* GetDirectedAt(EYapLoadContext LoadContext); // Non-const because of async loading handle
	
private:
	const UYapCharacter* GetCharacter_Internal(const TSoftObjectPtr<UYapCharacter>& CharacterAsset, TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext);

public:
	// TODO I don't think fragments should know where their position is!
	uint8 GetIndexInDialogue() const { return IndexInDialogue; }
	
	int32 GetActivationCount() const { return ActivationCount; }

	void SetRunState(EYapFragmentRunState NewState) { RunState = NewState; }
	
	EYapFragmentRunState GetRunState() const { return RunState; }

	void SetEntryState(EYapFragmentEntryStateFlags NewStateFlags) { LastEntryState = (EYapFragmentEntryStateFlags)NewStateFlags; }
	
	EYapFragmentEntryStateFlags GetLastEntryState() const { return LastEntryState; }
	
	int32 GetActivationLimit() const { return ActivationLimit; }

	bool CheckActivationLimit() const { if (ActivationLimit <= 0) return true; return ActivationCount < ActivationLimit; }

	bool IsActivationLimitMet() const { if (ActivationLimit <= 0) return false; return ActivationCount >= ActivationLimit; }

	const FText& GetDialogueText(EYapMaturitySetting MaturitySetting) const;
	
	const FText& GetTitleText(EYapMaturitySetting MaturitySetting) const;
	
	const UObject* GetAudioAsset(EYapMaturitySetting MaturitySetting) const;
	
	const FYapBit& GetBit() const;

	const FYapBit& GetBit(EYapMaturitySetting MaturitySetting) const;

	const FYapBit& GetMatureBit() const { return MatureBit; }

	const FYapBit& GetChildSafeBit() const { return ChildSafeBit; }

	FYapBit& GetMatureBitMutable() { return MatureBit; }

	FYapBit& GetChildSafeBitMutable() { return ChildSafeBit; }

	TOptional<float> GetSpeechTime(const FGameplayTag& TypeGroup) const;

	double GetStartTime() const { return StartTime; }

	void SetStartTime(double InTime) { StartTime = InTime; }

	double GetEndTime() const { return EndTime; }

	void SetEndTime(double InTime) { EndTime = InTime; }

	bool GetIsAwaitingManualAdvance() const { return bFragmentAwaitingManualAdvance; };

	void SetAwaitingManualAdvance() { bFragmentAwaitingManualAdvance = true; };

protected:
	TOptional<float> GetSpeechTime(EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext, const FGameplayTag& TypeGroup) const;

public:
	TOptional<float> GetPaddingSetting() const { return Padding; };
	
	float GetPaddingValue(const FGameplayTag& TypeGroup) const;

	float GetProgressionTime(const FGameplayTag& TypeGroup) const;
	
	void IncrementActivations();

	const FGameplayTag& GetFragmentTag() const { return FragmentTag; } 

	// TODO - want to design a better system for this.
	//void ReplaceBit(EYapMaturitySetting MaturitySetting, const FYapBitReplacement& ReplacementBit);

	const FGuid& GetGuid() const { return Guid; }

	bool UsesStartPin() const { return bShowOnStartPin; }

	bool UsesEndPin() const { return bShowOnEndPin; }

	const TArray<UYapCondition*>& GetConditions() const { return Conditions; }

	const TSoftObjectPtr<UYapCharacter>& GetSpeakerAsset() const { return SpeakerAsset; }
	
	const TSoftObjectPtr<UYapCharacter>& GetDirectedAtAsset() const { return DirectedAtAsset; }

	FFlowPin GetPromptPin() const;

	FFlowPin GetEndPin() const;

	FFlowPin GetStartPin() const;;

	void ResolveMaturitySetting(EYapMaturitySetting& MaturitySetting) const;
	
	TOptional<bool> GetSkippableSetting() const { return Skippable; }
	
	TOptional<bool>& GetSkippableSetting() { return Skippable; }
	
	TOptional<bool> GetAutoAdvanceSetting() const { return AutoAdvance; }
	
	TOptional<bool>& GetAutoAdvanceSetting() { return AutoAdvance; }
	
	/** Gets the evaluated skippable setting to be used for this fragment (incorporating project default settings and fallbacks) */
	bool GetSkippable(bool Default) const;
	
	/** Gets the evaluated time mode to be used for this bit (incorporating project default settings and fallbacks) */
	EYapTimeMode GetTimeMode(const FGameplayTag& TypeGroup) const;
	
	EYapTimeMode GetTimeMode(EYapMaturitySetting MaturitySetting, const FGameplayTag& TypeGroup) const;

	FGameplayTag GetMoodTag() const { return MoodTag; }

	const TArray<FInstancedStruct>& GetData() const { return Data; }
	
	bool IsTimeModeNone() const;

	bool HasAudio() const;

	bool HasData() const;

#if WITH_EDITOR
public:
	FYapBit& GetBitMutable(EYapMaturitySetting MaturitySetting);
		
	void SetIndexInDialogue(uint8 NewValue) { IndexInDialogue = NewValue; }

	FDelegateHandle FragmentTagChildrenFilterDelegateHandle;
	
	static void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String);
	
	void SetPaddingToNextFragment(float NewValue) { Padding = NewValue; }

	TArray<TObjectPtr<UYapCondition>>& GetConditionsMutable() { return Conditions; }

	void ResetGUID() { Guid = FGuid::NewGuid(); }
	
	FName GetPromptPinName() const { return GetPromptPin().PinName; }

	FName GetEndPinName() const { return GetEndPin().PinName; }

	FName GetStartPinName() const { return GetStartPin().PinName; }

	void ResetEndPin() { bShowOnEndPin = false; }

	void ResetStartPin() { bShowOnStartPin = false; }
	
	void InvalidateFragmentTag(UFlowNode_YapDialogue* OwnerNode);

	void SetMoodTag(const FGameplayTag& NewValue) { MoodTag = NewValue; };

	void SetTimeModeSetting(EYapTimeMode NewValue) { TimeMode = NewValue; }
	
	EYapTimeMode GetTimeModeSetting() const { return TimeMode; }
	// TODO implement this
	bool GetBitReplaced() const { return false; };
#endif

	
	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	void SetSpeaker(TSoftObjectPtr<UYapCharacter> InCharacter);
	
	void SetDirectedAt(TSoftObjectPtr<UYapCharacter> InDirectedAt);
#endif
};
