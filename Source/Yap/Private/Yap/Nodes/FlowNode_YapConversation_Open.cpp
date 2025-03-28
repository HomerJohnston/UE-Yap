// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Nodes/FlowNode_YapConversation_Open.h"

#include "FlowAsset.h"
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Nodes/FlowNode_YapConversation_Close.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

UFlowNode_YapConversation_Open::UFlowNode_YapConversation_Open()
{
#if WITH_EDITOR
	Category = TEXT("Yap");
#endif
}

void UFlowNode_YapConversation_Open::OnActivate()
{
	Super::OnActivate();
	
	FYapConversation& NewConversation = UYapSubsystem::Get()->OpenConversation(ConversationName, GetFlowAsset());

	// The subsystem will give conversation listeners a chance to set an interlock. If so, the state will be "Opening" rather than "Open".
	// When the interlock gets released, the delegate below will get called instead.
	if (NewConversation.GetState() == EYapConversationState::Open)
	{
		UE_LOG(LogYap, Verbose, TEXT("Conversation Opened: %s"), *ConversationName.GetTagName().ToString());
		FinishNode();
	}
	else
	{
		NewConversation.OnConversationOpened.AddDynamic(this, &ThisClass::FinishNode);
	}
}

void UFlowNode_YapConversation_Open::Finish()
{
	Super::Finish();

	UE_LOG(LogYap, Verbose, TEXT("    Entering conversation..."));

	UYapSubsystem::GetConversation(ConversationName).OnConversationOpened.RemoveAll(this);
}

void UFlowNode_YapConversation_Open::FinishNode()
{
	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FText UFlowNode_YapConversation_Open::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return FText::FromString("Conversation - Open");
	}

	return FText::FromString("Open Convo.");
}

void UFlowNode_YapConversation_Open::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#undef LOCTEXT_NAMESPACE