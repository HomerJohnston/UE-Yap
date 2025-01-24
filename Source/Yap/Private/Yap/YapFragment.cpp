// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapFragment.h"

#include "GameplayTagsManager.h"
#include "Yap/YapCondition.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

FYapFragment::FYapFragment()
{
	Guid = FGuid::NewGuid();
}

bool FYapFragment::CheckConditions() const
{
	for (TObjectPtr<UYapCondition> Condition : Conditions)
	{
		if (!IsValid(Condition))
		{
			UE_LOG(LogYap, Warning, TEXT("Ignoring null condition. Clean this up!")); // TODO more info
			continue;
		}
		if (!Condition->EvaluateCondition())
		{
			return false;
		}
	}
	
	return true;
}

void FYapFragment::ResetOptionalPins()
{
	bShowOnStartPin = false;
	bShowOnEndPin = false;
}

void FYapFragment::PreloadContent(UFlowNode_YapDialogue* OwningContext)
{
	Bit.PreloadContent(OwningContext);
}

float FYapFragment::GetPaddingToNextFragment() const
{
	if (GetBit().IsTimeModeNone())
	{
		return 0;
	}
	
	if (PaddingToNextFragment < 0)
	{
		return UYapProjectSettings::GetDefaultFragmentPaddingTime();
	}
	
	return FMath::Max(PaddingToNextFragment, 0);
}

void FYapFragment::IncrementActivations()
{
	ActivationCount++;
}

void FYapFragment::ReplaceBit(const FYapBitReplacement& ReplacementBit)
{
	Bit = ReplacementBit;
}

FFlowPin FYapFragment::GetPromptPin() const
{
	if (!PromptPin.IsValid())
	{
		FYapFragment* MutableThis = const_cast<FYapFragment*>(this);
		MutableThis->PromptPin = FName("Prompt_" + Guid.ToString());
		MutableThis->PromptPin.PinToolTip = "Out";
	}
	
	return PromptPin;
}

FFlowPin FYapFragment::GetEndPin() const
{
	if (!EndPin.IsValid())
	{
		FYapFragment* MutableThis = const_cast<FYapFragment*>(this);
		MutableThis->EndPin = FName("End_" + Guid.ToString());
		MutableThis->PromptPin.PinToolTip = "Runs before end-padding time begins";
	}
	
	return EndPin;
}

FFlowPin FYapFragment::GetStartPin() const
{
	if (!StartPin.IsValid())
	{
		FYapFragment* MutableThis = const_cast<FYapFragment*>(this);
		MutableThis->StartPin = FName("Start_" + Guid.ToString());
		MutableThis->StartPin.PinToolTip = "Runs when fragment starts playback";
	}
	
	return StartPin;
}

#if WITH_EDITOR
void FYapFragment::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString)
{
	if (!PropertyHandle || PropertyHandle->GetProperty()->GetFName() != GET_MEMBER_NAME_CHECKED(FYapFragment, FragmentTag))
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* Object : OuterObjects)
	{
		const UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(Object);

		if (!DialogueNode)
		{
			continue;
		}
		
		if (DialogueNode->IsPlayerPrompt())
		{
			MetaString = DialogueNode->GetDialogueTag().ToString();
		}
	}
}

TArray<FFlowPin> FYapFragment::GetOutputPins() const
{
	TArray<FFlowPin> OutPins;
	
	if (Owner.IsValid() && Owner->IsPlayerPrompt())
	{
		OutPins.Add(PromptPin);
	}
	
	if (UsesEndPin())
	{
		OutPins.Add(EndPin);
	}
	
	if (UsesStartPin())
	{
		OutPins.Add(StartPin);
	}
	
	return OutPins;
}

void FYapFragment::InvalidateFragmentTag()
{
	FragmentTag = FGameplayTag::EmptyTag;
}
#endif

#undef LOCTEXT_NAMESPACE