#pragma once
#include "FlowNode_YapConversationStart.h"
#include "Nodes/FlowNode.h"

#include "FlowNode_YapConversationEnd.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Convo End", Keywords = "yap"))
class FLOWYAP_API UFlowNode_YapConversationEnd : public UFlowNode
{
	GENERATED_BODY()

	UPROPERTY()
	FName ConversationName;
	
public:
	UFlowNode_YapConversationEnd();

	virtual void OnActivate() override;
};