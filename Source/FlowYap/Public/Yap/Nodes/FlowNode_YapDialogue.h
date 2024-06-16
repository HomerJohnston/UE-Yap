#pragma once

#include "Nodes/FlowNode.h"
#include "Yap/FlowYapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UFlowYapCharacter;

enum class EFlowYapInterruptible : uint8;

UENUM(BlueprintType)
enum class EFlowYapMultipleFragmentSequencing : uint8
{
	Sequential	,
	SelectOne	,
	COUNT		UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EFlowYapMultipleFragmentSequencing, EFlowYapMultipleFragmentSequencing::COUNT);

// TODO: you should NOT be able to set activation limits on any fragments which do not have unconnected nodes below them?

/**
 * Emits a FlowYap Dialogue Fragment
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Dialogue", Keywords = "yap", ToolTip = "Select node(s) and tap SHIFT key to show all node contents"))
class FLOWYAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()

friend class SFlowGraphNode_YapDialogueWidget;
friend class SFlowGraphNode_YapFragmentWidget;

public:
	UFlowNode_YapDialogue();

	// SETTINGS
protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	TArray<FFlowYapFragment> Fragments;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	bool bIsPlayerPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, meta = (ClampMin = 0, UIMin = 0, UIMax = 5))
	int32 NodeActivationLimit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, meta = (EditCondition = "!bIsPlayerPrompt", EditConditionHides))
	EFlowYapMultipleFragmentSequencing MultipleFragmentSequencing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	EFlowYapInterruptible Interruptible;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag PromptTag;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName ConversationName;
	
	// STATE
protected:
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly)
	int32 NodeActivationCount = 0;

	UPROPERTY()
	FTimerHandle TimerHandle;
	
	// API
public:
	void SetConversationName(FName Name);
	
	FText GetSpeakerName() const;

	const UTexture2D* GetDefaultSpeakerPortrait() const;

	const UTexture2D* GetSpeakerPortrait(const FName& RequestedMoodKey) const;

	FSlateBrush* GetSpeakerPortraitBrush(const FName& RequestedMoodKey) const;
	
	bool GetIsPlayerPrompt() const;

	int32 GetNodeActivationCount() const;

	int32 GetNodeActivationLimit() const;
	
	const TArray<FFlowYapFragment>& GetFragments();

	uint8 GetNumFragments() const;

	int16 FindFragmentIndex(FFlowYapFragment* Fragment) const;
	
	// -------------------

	void InitializeInstance() override;

	void OnActivate() override;
	
	void ExecuteInput(const FName& PinName) override;

	bool GetInterruptible() const;

protected:
	void OnTextTimeComplete(uint8 FragmentIndex);
	
	void OnPaddingTimeComplete(uint8 FragmentIndex);

	void RunFragmentsSequentiallyFrom(uint8 StartIndex);
	
#if WITH_EDITOR
public:
	const FFlowYapFragment* GetFragmentByIndex(int16 Index) const;
	
	FFlowYapFragment* GetFragmentByIndexMutable(int16 Index);
	
	TArray<FFlowYapFragment>& GetFragmentsMutable();
	
	void AddFragment();

	bool GetDynamicTitleColor(FLinearColor& OutColor) const override;

	bool CanUserAddInput() const override { return false; }

	bool CanUserAddOutput() const override { return false; }

	bool SupportsContextPins() const override;
	
	bool GetUsesMultipleInputs();
	
	bool GetUsesMultipleOutputs();

	EFlowYapMultipleFragmentSequencing GetMultipleFragmentSequencing() const;
	
	virtual TArray<FFlowPin> GetContextInputs() override;

	virtual TArray<FFlowPin> GetContextOutputs() override;
	
	void SetIsPlayerPrompt(bool NewValue);

	void SetNodeActivationLimit(int32 NewValue);

	void CycleFragmentSequencingMode();
	
	void DeleteFragmentByIndex(int16 DeleteIndex);
	
	void InsertFragment(uint8 Index);
	
	void UpdateFragmentIndices();

	void SwapFragments(uint8 IndexA, uint8 IndexB);

protected:
	TOptional<uint8> RunningFragmentIndex;

public:
	TOptional<uint8> GetRunningFragmentIndex() const { return RunningFragmentIndex; }

	FString GetNodeDescription() const override;

	const FGameplayTag& GetPromptTag() const { return PromptTag; }
	
	void OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const;

#endif // WITH_EDITOR
};