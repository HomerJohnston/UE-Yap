

// ================================================================================================

#include "Yap/YapSubsystem.h"

#include "GameplayTagsManager.h"
#include "Logging/StructuredLog.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapConversationBrokerBase.h"
#include "Yap/YapFragment.h"
#include "Yap/YapLog.h"
#include "Yap/IYapConversationListener.h"
#include "Yap/YapDialogueHandle.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapPromptHandle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

FYapActiveConversation::FYapActiveConversation()
{
	FlowAsset = nullptr;
	Conversation = FGameplayTag::EmptyTag;
}

bool FYapActiveConversation::StartConversation(UFlowAsset* InOwningAsset, const FGameplayTag& InConversation)
{
	if (Conversation != FGameplayTag::EmptyTag)
	{
		UE_LOGFMT(LogYap, Warning, "Tried to start conversation {0} but conversation {1} was already ongoing. Ignoring start request.", InConversation.ToString(), Conversation.ToString());
		return false;
	}
	
	if (InConversation == FGameplayTag::EmptyTag)
	{
		UE_LOG(LogYap, Error, TEXT("Tried to start conversation named NONE! Did you forgot to name the Start Conversation node? Ignoring start request."));
		return false;
	}

	FlowAsset = InOwningAsset;
	Conversation = InConversation;

	OnConversationStarts.ExecuteIfBound(Conversation);
	
	return true;
}

bool FYapActiveConversation::EndConversation()
{
	if (Conversation != FGameplayTag::EmptyTag)
	{
		OnConversationEnds.ExecuteIfBound(Conversation);

		FlowAsset = nullptr;
		Conversation = FGameplayTag::EmptyTag;

		return true;
	}

	return false;
}

// ================================================================================================

UYapSubsystem::UYapSubsystem()
{
}

void UYapSubsystem::RegisterConversationListener(UObject* NewListener)
{
	if (NewListener->Implements<UYapConversationListener>())
	{
		Listeners.AddUnique(NewListener);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a conversation handler, but it does not implement the FlowYapConversationHandler interface!"));
	}
}

void UYapSubsystem::UnregisterConversationListener(UObject* RemovedListener)
{
	Listeners.Remove(RemovedListener);
}

UYapCharacterComponent* UYapSubsystem::GetYapCharacter(const FGameplayTag& CharacterTag)
{
	TWeakObjectPtr<UYapCharacterComponent>* CharacterComponentPtr = YapCharacterComponents.Find(CharacterTag);

	if (CharacterComponentPtr && CharacterComponentPtr->IsValid())
	{
		return CharacterComponentPtr->Get();
	}

	return nullptr;
}

void UYapSubsystem::RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode)
{
	if (TaggedFragments.Contains(FragmentTag))
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to register tagged fragment with tag [%s] but this tag was already registered! Find and fix the duplicate tag usage."), *FragmentTag.ToString()); // TODO if I pass in the full fragment I could log the dialogue text to make this easier for designers?
		return;
	}
	
	TaggedFragments.Add(FragmentTag, DialogueNode);
}

FYapFragment* UYapSubsystem::FindTaggedFragment(const FGameplayTag& FragmentTag)
{
	UFlowNode_YapDialogue** DialoguePtr = TaggedFragments.Find(FragmentTag);

	if (DialoguePtr)
	{
		return (*DialoguePtr)->FindTaggedFragment(FragmentTag);
	}

	return nullptr;
}

bool UYapSubsystem::StartConversation(UFlowAsset* OwningAsset, const FGameplayTag& Conversation)
{
	return ActiveConversation.StartConversation(OwningAsset, Conversation);
}

void UYapSubsystem::EndCurrentConversation()
{
	if (!ActiveConversation.EndConversation())
	{
		return;
	}

	// TODO
	/*
	if (ConversationQueue.Num() > 0)
	{
		FName NextConversation = ConversationQueue[0];
		ConversationQueue.RemoveAt(0);
		StartConversation(NextConversation);
	}
	*/
}

void UYapSubsystem::BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	const FYapFragment& Fragment = Dialogue->GetFragmentByIndex(FragmentIndex);
	const FYapBit& Bit = Fragment.GetBit();

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation;
	}

	FYapPromptHandle Handle(Dialogue, FragmentIndex);

	BroadcastBrokerListenerFuncs<&UYapConversationBrokerBase::OnPromptOptionAdded, &IYapConversationListener::Execute_K2_OnPromptOptionAdded>
		(ConversationName, Handle, Bit.GetSpeaker(), Bit.GetMoodKey(), Bit.GetDialogueText(), Bit.GetTitleText());
}

void UYapSubsystem::OnFinishedBroadcastingPrompts()
{
	FGameplayTag ConversationName = ActiveConversation.IsConversationInProgress() ? ActiveConversation.Conversation : FGameplayTag::EmptyTag;

	BroadcastBrokerListenerFuncs<&UYapConversationBrokerBase::OnPromptOptionsAllAdded, &IYapConversationListener::Execute_K2_OnPromptOptionsAllAdded>
		(ConversationName);
}

void UYapSubsystem::BroadcastDialogueStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	FGuid DialogueGUID = Dialogue->GetGuid();
	const FYapFragment& Fragment = Dialogue->GetFragmentByIndex(FragmentIndex);
	const FYapBit& Bit = Fragment.GetBit();

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation;
	}

	bool bSkippable = (Bit.GetSkippable() == EYapDialogueSkippable::Default) ? Dialogue->GetSkippable() : Bit.GetSkippable() == EYapDialogueSkippable::Skippable;

	FYapDialogueHandle DialogueHandle(Dialogue, FragmentIndex, bSkippable);

	bool bUseChildSafe = ExecuteBrokerListenerFuncs<&UYapConversationBrokerBase::UseMatureDialogue, &IYapConversationListener::Execute_K2_UseMatureDialogue, bool>();
	
	BroadcastBrokerListenerFuncs<&UYapConversationBrokerBase::OnDialogueBegins, &IYapConversationListener::Execute_K2_OnDialogueBegins>
		(ConversationName, DialogueHandle, Bit.GetSpeaker(), Bit.GetMoodKey(), Bit.GetSpokenText(bUseChildSafe), Bit.GetTime(), Bit.GetDialogueAudioAsset<UObject>(), Bit.GetDirectedAt());
}

void UYapSubsystem::BroadcastDialogueEnd(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex)
{
	const FYapBit& Bit = OwnerDialogue->GetFragmentByIndex(FragmentIndex).GetBit();

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == OwnerDialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation;
	}

	FYapDialogueHandle DialogueHandle(OwnerDialogue, FragmentIndex, false);

	BroadcastBrokerListenerFuncs<&UYapConversationBrokerBase::OnDialogueEnds, &IYapConversationListener::Execute_K2_OnDialogueEnds>
		(ConversationName, DialogueHandle);
}

void UYapSubsystem::RunPrompt(const FYapPromptHandle& Handle)
{
	Handle.DialogueNode->RunPrompt(Handle.FragmentIndex);
}

void UYapSubsystem::RegisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent)
{
	AActor* Actor = YapCharacterComponent->GetOwner();

	if (RegisteredYapCharacterActors.Contains(Actor))
	{
		UE_LOG(LogYap, Error, TEXT("Multiple character components on actor, ignoring! Actor: %s"), *Actor->GetName());
		return;
	}

	YapCharacterComponents.Add(YapCharacterComponent->GetCharacterTag(), YapCharacterComponent);
	
	RegisteredYapCharacterActors.Add(Actor);
}

void UYapSubsystem::UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent)
{
	// TODO
	UE_LOG(LogYap, Error, TEXT("Unimplemented function UYapSubsystem::UnregisterCharacterComponent"));
}

// =====================================================================================================

void UYapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	ActiveConversation.OnConversationStarts.BindUObject(this, &UYapSubsystem::OnConversationStarts_Internal);
	ActiveConversation.OnConversationEnds.BindUObject(this, &UYapSubsystem::OnConversationEnds_Internal);

	// TODO handle null unset values
	TextCalculatorClass = UYapProjectSettings::Get()->GetTextCalculator().LoadSynchronous();
	DialogueAudioAssetClass = UYapProjectSettings::Get()->GetDialogueAssetClass().LoadSynchronous();
	ConversationBrokerClass = UYapProjectSettings::Get()->GetConversationBrokerClass().LoadSynchronous();

	if (ConversationBrokerClass)
	{
		ConversationBroker = NewObject<UYapConversationBrokerBase>(this, ConversationBrokerClass);
	}
}


void UYapSubsystem::OnConversationStarts_Internal(const FGameplayTag& ConversationName)
{
	BroadcastBrokerListenerFuncs<&UYapConversationBrokerBase::OnConversationBegins, &IYapConversationListener::Execute_K2_OnConversationBegins>
		(ConversationName);
}

void UYapSubsystem::OnConversationEnds_Internal(const FGameplayTag& ConversationName)
{
	BroadcastBrokerListenerFuncs<&UYapConversationBrokerBase::OnConversationEnds, &IYapConversationListener::Execute_K2_OnConversationEnds>
		(ConversationName);
}

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
