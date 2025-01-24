// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "YapBit.h"
#include "GameplayTagContainer.h"
#include "Nodes/FlowPin.h"

#include "YapFragment.generated.h"

class UYapCondition;
class UFlowNode_YapDialogue;
struct FFlowPin;

USTRUCT(NotBlueprintType)
struct YAP_API FYapFragment
{
	GENERATED_BODY()

public:
	FYapFragment();
	
	bool CheckConditions() const;
	void ResetOptionalPins();
	void PreloadContent(UFlowNode_YapDialogue* OwningContext);

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
#endif
	
	// ==========================================
	// SETTINGS
protected:
	UPROPERTY(Instanced)
	TArray<TObjectPtr<UYapCondition>> Conditions;
	
	UPROPERTY(meta = (ShowOnlyInnerProperties))
	FYapBit Bit;

	/** How many times is this fragment allowed to broadcast? This count persists only within this flow asset's lifespan (resets every Start). */
	UPROPERTY()
	int32 ActivationLimit = 0;
	
	UPROPERTY()
	FGameplayTag FragmentTag;

	/** Padding is idle time to wait after the fragment finishes running. A value of -1 will use project defaults. */
	UPROPERTY()
	float PaddingToNextFragment = -1;
	
	UPROPERTY()
	bool bShowOnStartPin = false;

	UPROPERTY()
	bool bShowOnEndPin = false;
	
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

	// ==========================================
	// API
public:
	// TODO I don't think fragments should know where their position is!
	uint8 GetIndexInDialogue() const { return IndexInDialogue; }
	
	int32 GetActivationCount() const { return ActivationCount; }
	
	int32 GetActivationLimit() const { return ActivationLimit; }

	bool IsActivationLimitMet() const { if (ActivationLimit <= 0) return false; return (ActivationCount >= ActivationLimit); }
	
	const FYapBit& GetBit() const { return Bit; }

	float GetPaddingToNextFragment() const;

	void IncrementActivations();

	const FGameplayTag& GetFragmentTag() const { return FragmentTag; } 

	void ReplaceBit(const FYapBitReplacement& ReplacementBit);

	const FGuid& GetGuid() const { return Guid; }

	bool UsesStartPin() const { return bShowOnStartPin; }

	bool UsesEndPin() const { return bShowOnEndPin; }

	const TArray<UYapCondition*>& GetConditions() const { return Conditions; }

	FFlowPin GetPromptPin() const;

	FFlowPin GetEndPin() const;

	FFlowPin GetStartPin() const;;

#if WITH_EDITOR
public:
	FYapBit& GetBitMutable() { return Bit; }
	
	TWeakObjectPtr<UFlowNode_YapDialogue> Owner;
	
	void SetIndexInDialogue(uint8 NewValue) { IndexInDialogue = NewValue; }

	FDelegateHandle FragmentTagChildrenFilterDelegateHandle;
	
	static void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String);
	
	void SetPaddingToNextFragment(float NewValue) { PaddingToNextFragment = NewValue; }

	TArray<TObjectPtr<UYapCondition>>& GetConditionsMutable() { return Conditions; }

	void ResetGUID() { Guid = FGuid::NewGuid(); };

	TArray<FFlowPin> GetOutputPins() const;

	FName GetPromptPinName() const { return GetPromptPin().PinName; }

	FName GetEndPinName() const { return GetEndPin().PinName; }

	FName GetStartPinName() const { return GetStartPin().PinName; }

	void ResetEndPin() { bShowOnEndPin = false; }

	void ResetStartPin() { bShowOnStartPin = false; }
	
	void InvalidateFragmentTag();

	// TODO implement this
	bool GetBitReplaced() const { return false; };
#endif
};