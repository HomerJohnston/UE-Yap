// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Nodes/FlowNode.h"

#include "FlowNode_YapConversation_Close.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Yap Conversation End", Keywords = "yap"))
class YAP_API UFlowNode_YapConversation_Close : public UFlowNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag Conversation;
	
public:
	UFlowNode_YapConversation_Close();

	void OnActivate() override;
	
	void Finish() override;

	UFUNCTION()
	void FinishNode();
	
#if WITH_EDITOR
	FText GetNodeTitle() const override;
#endif
};