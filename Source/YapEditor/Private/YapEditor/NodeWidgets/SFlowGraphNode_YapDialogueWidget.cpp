// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowEditorStyle.h"
#include "ILiveCodingModule.h"
#include "NodeFactory.h"
#include "PropertyCustomizationHelpers.h"
#include "SGraphPanel.h"
#include "SLevelOfDetailBranchNode.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphSettings.h"
#include "Graph/FlowGraphUtils.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Yap/YapBit.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "Yap/YapFragment.h"
#include "YapEditor/YapInputTracker.h"
#include "YapEditor/YapLogEditor.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapEditorEvents.h"
#include "YapEditor/NodeWidgets/SActivationCounterWidget.h"
#include "YapEditor/NodeWidgets/SYapConditionDetailsViewWidget.h"
#include "YapEditor/NodeWidgets/SYapConditionsScrollBox.h"
#include "YapEditor/NodeWidgets/SSkippableCheckBox.h"
#include "YapEditor/NodeWidgets/SYapGraphPinExec.h"
#include "YapEditor/SlateWidgets/SGameplayTagComboFiltered.h"

#define LOCTEXT_NAMESPACE "YapEditor"

constexpr int32 YAP_MIN_NODE_WIDTH = 275;
constexpr int32 YAP_DEFAULT_NODE_WIDTH = 400;

// TODO move to a proper style
FButtonStyle SFlowGraphNode_YapDialogueWidget::MoveFragmentButtonStyle;
bool SFlowGraphNode_YapDialogueWidget::bStylesInitialized = false;

void SFlowGraphNode_YapDialogueWidget::AddOverlayWidget(TSharedPtr<SWidget> ParentWidget, TSharedPtr<SWidget> OverlayWidget, bool bClearExisting)
{
	if (bClearExisting)
	{
		OverlayWidgets.Empty();
	}
	
	OverlayWidgets.Emplace(FYapWidgetOverlay(ParentWidget, OverlayWidget));

	SetNodeSelected();
}

void SFlowGraphNode_YapDialogueWidget::RemoveOverlayWidget(TSharedPtr<SWidget> OverlayWidget)
{
	OverlayWidgets.RemoveAll( [OverlayWidget] (FYapWidgetOverlay& X) { return X.Overlay == OverlayWidget;} );
}

void SFlowGraphNode_YapDialogueWidget::ClearOverlayWidgets()
{
	OverlayWidgets.Empty();
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	PreConstruct(InArgs, InNode);
	
	SFlowGraphNode::Construct(InArgs, InNode);
	
	PostConstruct(InArgs, InNode);
}

// ------------------------------------------
// CONSTRUCTION
// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::PreConstruct(const FArguments& InArgs, UFlowGraphNode* InNode)
{	
	FlowGraphNode_YapDialogue = Cast<UFlowGraphNode_YapDialogue>(InNode);

	DialogueButtonsColor = YapColor::DarkGray;

	ConnectedBypassPinColor = YapColor::LightBlue;
	DisconnectedBypassPinColor = YapColor::Red;
	
	ConnectedFragmentPinColor = YapColor::White;
	DisconnectedFragmentPinColor = YapColor::Red;
	
	bDragMarkerVisible = false;
	
	FocusedFragmentIndex.Reset();
	
	if (!bStylesInitialized)
	{
		MoveFragmentButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("PropertyEditor.AssetComboStyle");

		// Button colors
		MoveFragmentButtonStyle.Normal.TintColor = YapColor::Noir_Trans;
		MoveFragmentButtonStyle.Hovered.TintColor = YapColor::DarkGray_Trans;
		MoveFragmentButtonStyle.Pressed.TintColor = YapColor::DarkGrayPressed_Trans;

		// Text colors
		MoveFragmentButtonStyle.NormalForeground = YapColor::LightGray;
		MoveFragmentButtonStyle.HoveredForeground = YapColor::White;
		MoveFragmentButtonStyle.PressedForeground = YapColor::LightGrayPressed;

		bStylesInitialized = true;
	}
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::PostConstruct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	
}

// ------------------------------------------------------------------------------------------------
int32 SFlowGraphNode_YapDialogueWidget::GetDialogueActivationCount() const
{
	return GetFlowYapDialogueNode()->GetNodeActivationCount();
}

// ------------------------------------------------------------------------------------------------
int32 SFlowGraphNode_YapDialogueWidget::GetDialogueActivationLimit() const
{
	return GetFlowYapDialogueNode()->GetNodeActivationLimit();
}

// ------------------------------------------------------------------------------------------------
EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_SkippableToggleIconOff() const
{
	switch (GetFlowYapDialogueNode()->GetSkippableSetting())
	{
		case EYapDialogueSkippable::Default:
		{
			return UYapProjectSettings::GetDialogueSkippableByDefault() ? EVisibility::Collapsed : EVisibility::Visible;
		}
		case EYapDialogueSkippable::Skippable:
		{
			return EVisibility::Collapsed;
		}
		case EYapDialogueSkippable::NotSkippable:
		{
			return EVisibility::Visible;
		}
		default:
		{
			check(false);
		}
	}
	
	return (GetFlowYapDialogueNode()->Skippable == EYapDialogueSkippable::NotSkippable) ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnTextCommitted_DialogueActivationLimit(const FText& Text, ETextCommit::Type Arg)
{
	FYapTransactions::BeginModify(LOCTEXT("ChangeActivationLimit", "Change activation limit"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->SetNodeActivationLimit(FCString::Atoi(*Text.ToString()));

	FYapTransactions::EndModify();
}

// ------------------------------------------------------------------------------------------------
FGameplayTag SFlowGraphNode_YapDialogueWidget::Value_DialogueTag() const
{
	return GetFlowYapDialogueNode()->GetDialogueTag();
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnTagChanged_DialogueTag(FGameplayTag GameplayTag)
{
	if (GetFlowYapDialogueNodeMutable()->DialogueTag == GameplayTag)
	{
		return;
	}
	
	FYapTransactions::BeginModify(LOCTEXT("ChangeFragmentTag", "Change fragment tag"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->DialogueTag = GameplayTag;

	GetFlowYapDialogueNodeMutable()->InvalidateFragmentTags();

	FYapTransactions::EndModify();

	UpdateGraphNode();
}

// ------------------------------------------------------------------------------------------------
FOptionalSize SFlowGraphNode_YapDialogueWidget::GetMaxNodeWidth() const
{
	const float GraphGridSize = 16;
	return FMath::Max(YAP_MIN_NODE_WIDTH + UYapProjectSettings::GetPortraitSize(), YAP_DEFAULT_NODE_WIDTH + GraphGridSize * UYapProjectSettings::GetDialogueWidthAdjustment());
}

// ------------------------------------------------------------------------------------------------
FOptionalSize SFlowGraphNode_YapDialogueWidget::GetMaxTitleWidth() const
{
	const int32 TITLE_LEFT_RIGHT_EXTRA_WIDTH = 44;

	return GetMaxNodeWidth().Get() - TITLE_LEFT_RIGHT_EXTRA_WIDTH;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnClick_NewConditionButton(int32 FragmentIndex)
{
	FYapTransactions::BeginModify(LOCTEXT("AddCondition", "Add condition"), GetFlowYapDialogueNodeMutable());

	if (FragmentIndex == INDEX_NONE)
	{
		GetFlowYapDialogueNodeMutable()->GetConditionsMutable().Add(nullptr);
	}
	else
	{
		GetFragmentMutable(FragmentIndex).GetConditionsMutable().Add(nullptr);
	}

	FYapTransactions::EndModify();

	GetFlowYapDialogueNodeMutable()->ForceReconstruction();
}

// ------------------------------------------
// WIDGETS

// ================================================================================================
// TITLE WIDGET
// ------------------------------------------------------------------------------------------------

void SFlowGraphNode_YapDialogueWidget::OnConditionsArrayChanged()
{
	GraphNode->ReconstructNode();

	ClearOverlayWidgets();
	
	UpdateGraphNode();
}

void SFlowGraphNode_YapDialogueWidget::OnConditionDetailsViewBuilt(TSharedPtr<SYapConditionDetailsViewWidget> ConditionWidget, TSharedPtr<SWidget> ButtonWidget)
{
	AddOverlayWidget(ButtonWidget, ConditionWidget);
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle)
{
	TSharedPtr<SCheckBox> SkippableCheckBox;
	
	TSharedRef<SWidget> Widget = SNew(SBox)
	.Visibility_Lambda([]() { return GEditor->PlayWorld == nullptr ? EVisibility::Visible : EVisibility::HitTestInvisible; })
	.MaxDesiredWidth(this, &SFlowGraphNode_YapDialogueWidget::GetMaxTitleWidth)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(-10, -5, 14, -7)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
			.HighDetail()
			[
				SNew(SActivationCounterWidget, FOnTextCommitted::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::OnTextCommitted_DialogueActivationLimit))
				.ActivationCount(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueActivationCount)
				.ActivationLimit(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueActivationLimit)
				.FontHeight(10)
			]
			.LowDetail()
			[
				SNew(SSpacer)
				.Size(20)
			]
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.Padding(-10,0,2,0)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
			.HighDetail()
			[
				SAssignNew(DialogueConditionsScrollBox, SYapConditionsScrollBox)
				.DialogueNode(GetFlowYapDialogueNodeMutable())
				.ConditionsArrayProperty(FindFProperty<FArrayProperty>(UFlowNode_YapDialogue::StaticClass(), GET_MEMBER_NAME_CHECKED(UFlowNode_YapDialogue, Conditions)))
				.ConditionsContainer(GetFlowYapDialogueNodeMutable())
				.OnConditionsArrayChanged(this, &SFlowGraphNode_YapDialogueWidget::OnConditionsArrayChanged)
				.OnConditionDetailsViewBuilt(this, &SFlowGraphNode_YapDialogueWidget::OnConditionDetailsViewBuilt)
			]
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.Padding(2,0,7,0)
		.AutoWidth()
		.VAlign(VAlign_Fill)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
			.HighDetail()
			[
				SNew(SGameplayTagComboFiltered)
				.Tag(TAttribute<FGameplayTag>::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::Value_DialogueTag))
				.Filter(UYapProjectSettings::GetDialogueTagsParent().ToString())
				.OnTagChanged(TDelegate<void(const FGameplayTag)>::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::OnTagChanged_DialogueTag))
				.ToolTipText(LOCTEXT("DialogueTag", "Dialogue tag"))	
			]
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		.Padding(2, -2, -25, -2)
		[
			SNew(SBox)
			.WidthOverride(20)
			.HAlign(HAlign_Center)
			[
				SNew(SYapSkippableCheckBox)
				.IsSkippable_Lambda( [this] () { return GetFlowYapDialogueNode()->GetSkippable(); } )
				.SkippableSetting_Lambda( [this] () { return GetFlowYapDialogueNode()->Skippable; })
				.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::OnCheckStateChanged_SkippableToggle)
			]
		]
	];
	
	return Widget;
}

// ------------------------------------------------------------------------------------------------
ECheckBoxState SFlowGraphNode_YapDialogueWidget::IsChecked_SkippableToggle() const
{
	switch (GetFlowYapDialogueNode()->Skippable)
	{
		case EYapDialogueSkippable::Default:
		{
			return ECheckBoxState::Undetermined;
		}
		case EYapDialogueSkippable::NotSkippable:
		{
			return ECheckBoxState::Unchecked;
		}
		case EYapDialogueSkippable::Skippable:
		{
			return ECheckBoxState::Checked;
		}
		default:
		{
			check(false);
		}
	}
	return ECheckBoxState::Undetermined;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnCheckStateChanged_SkippableToggle(ECheckBoxState CheckBoxState)
{
	FYapTransactions::BeginModify(LOCTEXT("ToggleSkippable", "Toggle skippable"), GetFlowYapDialogueNodeMutable());

	if (GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed())
	{
		GetFlowYapDialogueNodeMutable()->Skippable = EYapDialogueSkippable::Default;
	}
	else if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFlowYapDialogueNodeMutable()->Skippable = EYapDialogueSkippable::Skippable;
	}
	else
	{
		GetFlowYapDialogueNodeMutable()->Skippable = EYapDialogueSkippable::NotSkippable;
	}

	FYapTransactions::EndModify();
}

// ------------------------------------------------------------------------------------------------
FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_SkippableToggleIcon() const
{
	if (GetFlowYapDialogueNode()->Skippable == EYapDialogueSkippable::NotSkippable)
	{
		return YapColor::LightYellow;
	}
	else if (GetFlowYapDialogueNode()->Skippable == EYapDialogueSkippable::Skippable)
	{
		return YapColor::LightGreen;
	}
	else
	{
		return YapColor::DarkGray;
	}
}

// ================================================================================================
// NODE CONTENT WIDGET
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateNodeContentArea()
{
	TSharedPtr<SVerticalBox> Content; 
	
	return SNew(SBox)
	.WidthOverride(this, &SFlowGraphNode_YapDialogueWidget::GetMaxNodeWidth)
	.Visibility_Lambda([]() { return GEditor->PlayWorld == nullptr ? EVisibility::Visible : EVisibility::HitTestInvisible; })
	[
		SAssignNew(Content, SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 3, 0, 4)
		[
			CreateContentHeader()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateFragmentBoxes()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateContentFooter()
		]
	];
}

// ------------------------------------------------------------------------------------------------
FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeaderButton() const
{
	if (GetFlowYapDialogueNode()->ActivationLimitsMet() && GetFlowYapDialogueNode()->GetActivationState() != EFlowNodeState::Active)
	{
		return YapColor::Red;
	}

	return YapColor::DarkGray;
}

// ------------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapDialogueWidget::Text_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EYapDialogueTalkSequencing::RunAll:
		{
			return LOCTEXT("RunAll", "Run All");
		}
		case EYapDialogueTalkSequencing::RunUntilFailure:
		{
			return LOCTEXT("RunTilFailure", "Run til failure");
		}
		case EYapDialogueTalkSequencing::SelectOne:
		{
			return LOCTEXT("SelectOne", "Select one");
		}
		default:
		{
			return LOCTEXT("Error", "Error");
		}
	}
}

// ------------------------------------------------------------------------------------------------
FReply SFlowGraphNode_YapDialogueWidget::OnClicked_TogglePlayerPrompt()
{
	{
		FYapScopedTransaction T("TODO", LOCTEXT("TogglePlayerPrompt", "Toggle Player Prompt"), GetFlowYapDialogueNodeMutable());

		GetFlowYapDialogueNodeMutable()->ToggleNodeType();
		GetFlowYapDialogueNodeMutable()->ForceReconstruction();

		NodeHeaderButtonToolTip->SetText(Text_NodeHeader());
	}
	
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateContentHeader()
{
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(4, 0, 0, 0)
	[
		SAssignNew(DialogueInputBoxArea, SBox)
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(-2, 0, 0, 0)
	[
		SAssignNew(NodeHeaderButton, SButton)
		.Cursor(EMouseCursor::Default)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HeaderButton)
		.ContentPadding(FMargin(4, 0, 4, 0))
		.ButtonColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeaderButton)
		.ForegroundColor(YapColor::White)
		.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_TogglePlayerPrompt)
		.ToolTipText(LOCTEXT("ToggleDialogueModeToolTip", "Toggle between player prompt or normal speech"))
		[
			SAssignNew(NodeHeaderButtonToolTip, STextBlock)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_NodeHeader)
			.Text(Text_NodeHeader())
			.ColorAndOpacity(FSlateColor::UseForeground())
		]
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(8, 0, 8, 0)
	.VAlign(VAlign_Fill)
	[
		SAssignNew(FragmentSequencingButton_Box, SBox)
		.Visibility(Visibility_FragmentSequencingButton())
		.VAlign(VAlign_Fill)
		.WidthOverride(110)
		.Padding(0)
		[
			SAssignNew(FragmentSequencingButton_Button, SButton)
			.Cursor(EMouseCursor::Default)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ContentPadding(FMargin(2, 1, 2, 1))
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSequencingButton)
			.ToolTipText(ToolTipText_FragmentSequencingButton())
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0, 0, 4, 0)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SAssignNew(FragmentSequencingButton_Image, SImage)
					.ColorAndOpacity(ColorAndOpacity_FragmentSequencingButton())
					.DesiredSizeOverride(FVector2D(16, 16))
					.Image(Image_FragmentSequencingButton())
				]
				+ SHorizontalBox::Slot()
				.Padding(4, 0, 0, 0)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SAssignNew(FragmentSequencingButton_Text, STextBlock)
					.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_NodeSequencing)
					.Text(Text_FragmentSequencingButton())
					.Justification(ETextJustify::Left)
					.ColorAndOpacity(ColorAndOpacity_FragmentSequencingButton())
				]
			]
		]
	]
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Fill)
	[
		SNew(SSpacer)
	]
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Right)
	.AutoWidth()
	.Padding(0, 0, 4, 0)
	[
		SAssignNew(DialogueOutputBoxArea, SBox)
	];

	return Box;
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentBoxes()
{
	bool bFirstFragment = true;

	TSharedRef<SVerticalBox> FragmentBoxes = SNew(SVerticalBox);

	FragmentWidgets.Empty();
	
	for (uint8 FragmentIndex = 0; FragmentIndex < GetFlowYapDialogueNode()->GetNumFragments(); ++FragmentIndex)
	{
		FragmentBoxes->AddSlot()
		.AutoHeight()
		.Padding(0, bFirstFragment ? 0 : 13, 0, bFirstFragment ? 8 : 10)
		[
			CreateFragmentSeparatorWidget(FragmentIndex)
		];
		
		FragmentBoxes->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			CreateFragmentRowWidget(FragmentIndex)
		];
		
		bFirstFragment = false;
	};

	return FragmentBoxes;
}

// ------------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapDialogueWidget::Text_NodeHeader() const
{
	if (GetFlowYapDialogueNode()->IsPlayerPrompt())
	{
		return LOCTEXT("DialogueModeLabel_PlayerPrompt", "PLAYER PROMPT");
	}
	else
	{
		return LOCTEXT("DialogueModeLabel_Talk", "TALK");
	}
}

// ------------------------------------------------------------------------------------------------
// TODO UNUSED?
EVisibility SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_Visibility(uint8 f) const
{
	if (FlashFragmentIndex == f /*|| (GetFlowYapDialogueNode()->GetRunningFragmentIndex() == f && GetFlowYapDialogueNode()->FragmentStartedTime > GetFlowYapDialogueNode()->FragmentEndedTime)*/ /* TODO */ )
	{
		return EVisibility::HitTestInvisible;
	}

	return EVisibility::Collapsed;
}

// ------------------------------------------------------------------------------------------------
// TODO UNUSED?
FSlateColor SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_BorderBackgroundColor(uint8 f) const
{
	if ( /*GetFlowYapDialogueNode()->GetRunningFragmentIndex() == f*/ false)
	{
		return YapColor::White_Glass;
	}
	
	if (FlashFragmentIndex == f)
	{
		return FlashHighlight * YapColor::White_Trans;
	}

	return YapColor::Transparent;
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentSeparatorWidget(uint8 FragmentIndex)
{
	return SNew(SButton)
	.Cursor(EMouseCursor::Default)
	.ContentPadding(2)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HeaderButton)
	.ButtonColorAndOpacity(YapColor::DarkGray)
	.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSeparator, FragmentIndex)
	[
		SNew(SSeparator)
		.Thickness(2)	
	];
}

// ------------------------------------------------------------------------------------------------
EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_FragmentSeparator() const
{
	return GetIsSelected() ? EVisibility::Visible : EVisibility::Hidden;
}

// ------------------------------------------------------------------------------------------------
FReply SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSeparator(uint8 Index)
{
	FYapTransactions::BeginModify(LOCTEXT("AddFragment", "Add fragment"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->AddFragment(Index);

	UpdateGraphNode();

	FYapTransactions::EndModify();

	SetNodeSelected();
	
	return FReply::Handled();
}

// ================================================================================================
// FRAGMENT ROW
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentRowWidget(uint8 FragmentIndex)
{
	TSharedPtr<SFlowGraphNode_YapFragmentWidget> FragmentWidget = SNew(SFlowGraphNode_YapFragmentWidget, this, FragmentIndex);

	FragmentWidgets.Add(FragmentWidget);
	
	return FragmentWidget.ToSharedRef();
}

// ================================================================================================
// LEFT SIDE PANE
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateLeftFragmentPane(uint8 FragmentIndex)
{
	return SNew(SBox)
	.WidthOverride(32)
	[
		SNew(SOverlay)
		/*
		+ SOverlay::Slot()
		.Padding(-72, 0, 0, 0)
		[
			CreateFragmentControlsWidget(FragmentIndex)
		]
		*/
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			//.AutoHeight()
			.HAlign(HAlign_Center)
			[
				CreateLeftSideNodeBox()
			]
		]
	];
}

// ================================================================================================
// INPUT NODE BOX (UPPER HALF OF LEFT SIDE PANE)
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateLeftSideNodeBox()
{
	TSharedRef<SVerticalBox> LeftSideNodeBox = SNew(SVerticalBox);

	return SNew(SBox)
	.MinDesiredHeight(16)
	.IsEnabled_Lambda([]() { return GEditor->PlayWorld == nullptr; })
	[
		LeftSideNodeBox
	];
}

// ================================================================================================
// RIGHT PANE OF FRAGMENT ROW
// ------------------------------------------------------------------------------------------------

// ================================================================================================
// BOTTOM BAR
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateContentFooter()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		.IsEnabled_Lambda([]() { return GEditor->PlayWorld == nullptr; })
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(31, 4, 7, 4)
		[
			SNew(SBox)
			.Visibility(this, &SFlowGraphNode_YapDialogueWidget::Visibility_BottomAddFragmentButton)
			.HeightOverride(14)
			.VAlign(VAlign_Center)
			//.Padding(0, 0, 0, 2)
			[
				SNew(SButton)
				.Cursor(EMouseCursor::Default)
				.HAlign(HAlign_Center)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("DialogueAddFragment_Tooltip", "Add Fragment"))
				.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_BottomAddFragmentButton)
				.ContentPadding(0)
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_PlusSign))
						.DesiredSizeOverride(FVector2D(12, 12))
						.ColorAndOpacity(YapColor::Noir)
					]
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(0, 2, 1, 2)
		[
			SAssignNew(BypassOutputBox, SBox)
			.HAlign(HAlign_Center)
			.WidthOverride(24)
			.HeightOverride(24)
			.Padding(0)
		]
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(1)
	[
		SNew(SSeparator)
		.Visibility(this, &SFlowGraphNode_YapDialogueWidget::Visibility_AddonsSeparator)
		.Thickness(1)
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.FillWidth(1.0f)
		[
			SAssignNew(LeftNodeBox, SVerticalBox)
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		[
			SAssignNew(RightNodeBox, SVerticalBox)
		]
	];
}

// ------------------------------------------------------------------------------------------------
EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_FragmentSequencingButton() const
{
	if (GetFlowYapDialogueNode()->IsPlayerPrompt())
	{
		return EVisibility::Hidden; // Should be Collapsed but that destroys the parent widget layout for some reason
	}
	
	return (GetFlowYapDialogueNode()->GetNumFragments() > 1) ? EVisibility::Visible : EVisibility::Hidden;
}

// ------------------------------------------------------------------------------------------------
FReply SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSequencingButton()
{
	{
		//FYapScopedTransaction(LOCTEXT("ChangeDialogueNodeSequencing", "Change dialogue node sequencing mode"), FlowGraphNode_YapDialogue, YapEditor::Events::DialogueNode::Test, true);
	
		FYapTransactions::BeginModify(LOCTEXT("ChangeSequencingSetting", "Change sequencing setting"), GetFlowYapDialogueNodeMutable());

		GetFlowYapDialogueNodeMutable()->CycleFragmentSequencingMode();

		//FragmentSequencingButton_Box->SetVisibility(Visibility_FragmentSequencingButton());

		FragmentSequencingButton_Button->SetToolTipText(ToolTipText_FragmentSequencingButton());
	
		FragmentSequencingButton_Image->SetImage(Image_FragmentSequencingButton());
		FragmentSequencingButton_Image->SetColorAndOpacity(ColorAndOpacity_FragmentSequencingButton());

		FragmentSequencingButton_Text->SetText(Text_FragmentSequencingButton());
		FragmentSequencingButton_Text->SetColorAndOpacity(ColorAndOpacity_FragmentSequencingButton());

		//FlowGraphNode->ReconstructNode();
		//UpdateGraphNode();
	
		FYapTransactions::EndModify();
	}
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------
const FSlateBrush* SFlowGraphNode_YapDialogueWidget::Image_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EYapDialogueTalkSequencing::RunAll:
		{
			return FAppStyle::Get().GetBrush("Icons.SortDown"); 
		}
		case EYapDialogueTalkSequencing::RunUntilFailure:
		{
			return FAppStyle::Get().GetBrush("Icons.SortDown"); 
		}
		case EYapDialogueTalkSequencing::SelectOne:
		{
			return FAppStyle::Get().GetBrush("LevelEditor.Profile"); 
		}
	}

	return FAppStyle::Get().GetBrush("Icons.Error"); 
}

// ------------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapDialogueWidget::ToolTipText_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EYapDialogueTalkSequencing::RunAll:
		{
			return LOCTEXT("DialogueNodeSequence", "Starting from the top, attempt to run all fragments");
		}
		case EYapDialogueTalkSequencing::RunUntilFailure:
		{
			return LOCTEXT("DialogueNodeSequence", "Starting from the top, attempt to run all fragments, stopping if one fails"); 
		}
		case EYapDialogueTalkSequencing::SelectOne:
		{
			return LOCTEXT("DialogueNodeSequence", "Starting from the top, attempt to run all fragments, stopping if one succeeds");
		}
		default:
		{
			return LOCTEXT("DialogueNodeSequence", "ERROR");
		}
	}
}

// ------------------------------------------------------------------------------------------------
FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EYapDialogueTalkSequencing::RunAll:
		{
			return YapColor::LightBlue;
		}
		case EYapDialogueTalkSequencing::RunUntilFailure:
		{
			return YapColor::LightGreen;
		}
		case EYapDialogueTalkSequencing::SelectOne:
		{
			return YapColor::LightOrange;
		}
		default:
		{
			return YapColor::White;
		}
	}
}

// ------------------------------------------------------------------------------------------------
EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_BottomAddFragmentButton() const
{
	if (GEditor->PlayWorld)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

// ------------------------------------------------------------------------------------------------
FReply SFlowGraphNode_YapDialogueWidget::OnClicked_BottomAddFragmentButton()
{
	FYapTransactions::BeginModify(LOCTEXT("AddFragment", "Add fragment"), GetFlowYapDialogueNodeMutable());
	
	GetFlowYapDialogueNodeMutable()->AddFragment();

	UpdateGraphNode();

	FYapTransactions::EndModify();

	SetNodeSelected();
	
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------
EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_AddonsSeparator() const
{
	return GetFlowYapDialogueNode()->AddOns.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnClick_DeleteConditionButton(int32 FragmentIndex, int32 ConditionIndex)
{
	FYapTransactions::BeginModify(LOCTEXT("DeleteCondition", "Delete condition"), GetFlowYapDialogueNodeMutable());

	if (FragmentIndex == INDEX_NONE)
	{
		GetFlowYapDialogueNodeMutable()->GetConditionsMutable().RemoveAt(ConditionIndex);
	}
	else
	{
		GetFragmentMutable(FragmentIndex).GetConditionsMutable().RemoveAt(ConditionIndex);
	}

	FYapTransactions::EndModify();

	GetFlowYapDialogueNodeMutable()->ForceReconstruction();
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnEditedConditionChanged(int32 FragmentIndex, int32 ConditionIndex)
{
}

// ------------------------------------------------------------------------------------------------
bool SFlowGraphNode_YapDialogueWidget::IsEnabled_ConditionWidgetsScrollBox() const
{
	return true; //(SYapConditionsScrollBox::EditedConditionDetailsWidget.IsValid());
}

TArray<FOverlayWidgetInfo> SFlowGraphNode_YapDialogueWidget::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	TArray<FOverlayWidgetInfo> Widgets;

	/*
	if (FocusedConditionWidget.IsValid())
	{
		if (FocusedConditionWidgetStartTime < 0)
		{
			FocusedConditionWidget->SetRenderOpacity(0.0);
		}
		else
		{
			const float Delta = 0.2;
			float Opacity = FMath::Lerp(0.0, 1.0, (FPlatformTime::Seconds() - FocusedConditionWidgetStartTime) / Delta);
			FocusedConditionWidget->SetRenderOpacity(Opacity);
		}
		TSharedPtr<SYapConditionsScrollBox> ScrollBox = (FocusedConditionWidget->FragmentIndex == INDEX_NONE) ? DialogueConditionsScrollBox : FragmentWidgets[FocusedConditionWidget->FragmentIndex]->GetConditionsScrollBox();

		FVector2D OwnerLTA = GetPaintSpaceGeometry().LocalToAbsolute(FVector2D(0, 0));
		
		TSharedPtr<SWidget> EditedButton = ScrollBox->GetEditedButton(FocusedConditionWidget->ConditionIndex);

		const FGeometry& ButtonGeo = EditedButton->GetPaintSpaceGeometry();
	
		FVector2D ConditionDetailsPaneOffset = ButtonGeo.LocalToAbsolute(FVector2D(0, 0)) - OwnerLTA;

		ConditionDetailsPaneOffset *= 1.0 / OwnerGraphPanelPtr.Pin()->GetZoomAmount();
	
		FOverlayWidgetInfo Info;
		Info.OverlayOffset = ConditionDetailsPaneOffset + FVector2D(0, 20);
		Info.Widget = FocusedConditionWidget;

		Widgets.Add(Info);
	}
*/

	for (const FYapWidgetOverlay& WidgetOverlay : OverlayWidgets)
	{
		FVector2D OwnerLTA = GetPaintSpaceGeometry().LocalToAbsolute(FVector2D(0, 0));

		const FGeometry& ParentGeo = WidgetOverlay.Parent->GetPaintSpaceGeometry();
	
		FVector2D WidgetOfsset = ParentGeo.LocalToAbsolute(FVector2D(0, 0)) - OwnerLTA;

		WidgetOfsset *= 1.0 / OwnerGraphPanelPtr.Pin()->GetZoomAmount();
	
		FOverlayWidgetInfo Info;
		Info.OverlayOffset = WidgetOfsset + FVector2D(0, ParentGeo.Size.Y);
		Info.Widget = WidgetOverlay.Overlay;

		Widgets.Add(Info);

		WidgetOverlay.Overlay->SetRenderOpacity(WidgetOverlay.Opacity);
	}

	return Widgets;
}

// ------------------------------------------------------------------------------------------------
// PUBLIC API & THEIR HELPERS

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::SetNodeSelected()
{
	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());

	if (!GraphEditor)
	{
		return;
	}

	GraphEditor->SelectSingleNode(GraphNode);
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::SetFocusedFragmentIndex(uint8 InFragment)
{
	if (FocusedFragmentIndex != InFragment)
	{
		TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());
		GraphEditor->SetNodeSelection(FlowGraphNode, true);
		
		FocusedFragmentIndex = InFragment;
	}

	SetTypingFocus();
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::ClearFocusedFragmentIndex(uint8 FragmentIndex)
{
	if (FocusedFragmentIndex == FragmentIndex)
	{
		FocusedFragmentIndex.Reset();
	}
}

// ------------------------------------------------------------------------------------------------
const bool SFlowGraphNode_YapDialogueWidget::GetFocusedFragmentIndex(uint8& OutFragmentIndex) const
{
	if (FocusedFragmentIndex.IsSet())
	{
		OutFragmentIndex = FocusedFragmentIndex.GetValue();
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::SetTypingFocus()
{
	bKeyboardFocused = true;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::ClearTypingFocus()
{
	bKeyboardFocused = false;
}

// ------------------------------------------------------------------------------------------------
UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowYapDialogueNodeMutable()
{
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNodeBase());
}

// ------------------------------------------------------------------------------------------------
const UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowYapDialogueNode() const
{
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNodeBase());
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::SetFlashFragment(uint8 FragmentIndex)
{
	FlashFragmentIndex = FragmentIndex;
	FlashHighlight = 1.0;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnDialogueEnd(uint8 FragmentIndex)
{
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnDialogueStart(uint8 FragmentIndex)
{
	SetFlashFragment(FragmentIndex);
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnDialogueSkipped(uint8 FragmentIndex)
{
	SetFlashFragment(FragmentIndex);
}

// ------------------------------------------------------------------------------------------------
// OVERRIDES & THEIR HELPERS

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SFlowGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());

	if (!GraphEditor)
	{
		return;
	}
	
	// TODO cleanup
	bIsSelected = GraphEditor->GetSelectedFlowNodes().Contains(FlowGraphNode);

	bShiftPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetShiftPressed();
	bCtrlPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();
	
	if (bIsSelected && bShiftPressed && !bKeyboardFocused)
	{
		bShiftHooked = true;
	}

	if (bIsSelected)
	{
		if (FocusedConditionWidget.IsValid() && FocusedConditionWidgetStartTime < 0)
		{
			FocusedConditionWidgetStartTime = FPlatformTime::Seconds();
		}
	}
	else
	{
		bShiftHooked = false;
		FocusedFragmentIndex.Reset();
		bKeyboardFocused = false;

		FocusedConditionWidget = nullptr;
		FocusedConditionWidgetStartTime = -1;

		OverlayWidgets.Empty();
	}

	FlashHighlight = FMath::Max(FlashHighlight, FlashHighlight -= 2.0 * InDeltaTime);

	if (FlashHighlight <= 0)
	{
		FlashFragmentIndex.Reset();
	}

	for (FYapWidgetOverlay& Overlay : OverlayWidgets)
	{
		float NewValue = Overlay.Opacity += 7.0 * InDeltaTime;
		Overlay.Opacity = FMath::Clamp(NewValue, 0.0, 1.0);
	}
}

static const FName OutPinName = FName("Out");
static const FName BypassPinName = FName("Bypass");
static const FName InputPinName = FName("In");

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::CreatePinWidgets()
{	
	TArray<TSet<FFlowPin>> FragmentPins;
	FragmentPins.SetNum(GetFlowYapDialogueNode()->GetFragments().Num());

	TSet<FFlowPin> OptionalPins;
	
	TMap<FFlowPin, int32> FragmentPinsFragmentIndex;

	TSet<FFlowPin> PromptOutPins;
	
	for (int32 i = 0; i < GetFlowYapDialogueNode()->GetFragments().Num(); ++i)
	{
		const FYapFragment& Fragment = GetFlowYapDialogueNode()->GetFragments()[i];

		if (Fragment.UsesStartPin())
		{
			FFlowPin StartPin = Fragment.GetStartPin();
			
			FragmentPins[i].Add(StartPin);
			FragmentPinsFragmentIndex.Add(StartPin, i);
			OptionalPins.Add(StartPin);
		}

		if (Fragment.UsesEndPin())
		{
			FFlowPin EndPin = Fragment.GetEndPin();

			FragmentPins[i].Add(EndPin);
			FragmentPinsFragmentIndex.Add(EndPin, i);
			OptionalPins.Add(EndPin);
		}

		// We store all potential prompt pin names regardless of whether this is a Player Prompt node or not - this helps deal with orphaned pins easier if the user switches the dialogue node type
		FFlowPin PromptPin = Fragment.GetPromptPin();

		FragmentPins[i].Add(PromptPin);
		FragmentPinsFragmentIndex.Add(PromptPin, i);
		PromptOutPins.Add(PromptPin);
	}
	
	// Create Pin widgets for each of the pins.
	for (int32 PinIndex = 0; PinIndex < GraphNode->Pins.Num(); ++PinIndex)
	{
		UEdGraphPin* Pin = GraphNode->Pins[PinIndex];

		if ( !ensureMsgf(Pin->GetOuter() == GraphNode
			, TEXT("Graph node ('%s' - %s) has an invalid %s pin: '%s'; (with a bad %s outer: '%s'); skiping creation of a widget for this pin.")
			, *GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString()
			, *GraphNode->GetPathName()
			, (Pin->Direction == EEdGraphPinDirection::EGPD_Input) ? TEXT("input") : TEXT("output")
			,  Pin->PinFriendlyName.IsEmpty() ? *Pin->PinName.ToString() : *Pin->PinFriendlyName.ToString()
			,  Pin->GetOuter() ? *Pin->GetOuter()->GetClass()->GetName() : TEXT("UNKNOWN")
			,  Pin->GetOuter() ? *Pin->GetOuter()->GetPathName() : TEXT("NULL")) )
		{
			continue;
		}

		const TSharedRef<SGraphPin> NewPinRef = OptionalPins.Contains(Pin->GetFName()) ? SNew(SYapGraphPinExec, Pin) : FNodeFactory::CreatePinWidget(Pin).ToSharedRef();

		NewPinRef->SetOwner(SharedThis(this));
		NewPinRef->SetShowLabel(false);
		NewPinRef->SetPadding(FMargin(-4, -2, 2, -2));
		NewPinRef->SetColorAndOpacity(YapColor::White);

		FString PinToolTIpText = Pin->GetName();

		int32 UnderscoreIndex;
		if (PinToolTIpText.FindLastChar('_', UnderscoreIndex))
		{
			PinToolTIpText = PinToolTIpText.Left(UnderscoreIndex);
		}
		
		NewPinRef->SetToolTipText(FText::FromString(PinToolTIpText));
		
		if (OptionalPins.Contains(Pin->GetFName()))
		{
			NewPinRef->SetPadding(FMargin(-4, -2, 16, -2));
		}

		NewPinRef->SetHAlign(HAlign_Right);

		const bool bAdvancedParameter = Pin && Pin->bAdvancedView;
		if (bAdvancedParameter)
		{
			NewPinRef->SetVisibility(TAttribute<EVisibility>(NewPinRef, &SGraphPin::IsPinVisibleAsAdvanced));
		}

		TSharedPtr<SBox> PinBox = nullptr;

		if (Pin->GetFName() == OutPinName)
		{
			PinBox = DialogueOutputBoxArea;
			NewPinRef->SetColorAndOpacity(YapColor::White);
			NewPinRef->SetPadding(FMargin(-4, -2, 2, -2));
		}
		else if (Pin->GetFName() == BypassPinName)
		{
			PinBox = BypassOutputBox;
			NewPinRef->SetColorAndOpacity(NewPinRef->IsConnected() ? ConnectedBypassPinColor : DisconnectedBypassPinColor);
			NewPinRef->SetPadding(FMargin(-4, -2, 2, -2));
		}
		else if (int32* FragmentIndex = FragmentPinsFragmentIndex.Find(Pin->GetFName()))
		{
			PinBox = FragmentWidgets[*FragmentIndex]->GetPinContainer(Pin->GetFName());

			FLinearColor PinColor = NewPinRef->IsConnected() ? ConnectedFragmentPinColor : DisconnectedFragmentPinColor;
			
			NewPinRef->SetColorAndOpacity(PinColor);
		}
		else if (Pin->GetFName() == InputPinName)
		{
			PinBox = DialogueInputBoxArea;
			NewPinRef->SetPadding(FMargin(4, -2, 0, -2));			
		}
		else
		{
			NewPinRef->SetShowLabel(true);
			
			if (bAdvancedParameter)
			{
				NewPinRef->SetVisibility(TAttribute<EVisibility>(NewPinRef, &SGraphPin::IsPinVisibleAsAdvanced));
			}

			if (NewPinRef->GetDirection() == EEdGraphPinDirection::EGPD_Input)
			{
				LeftNodeBox->AddSlot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					.Padding(Settings->GetInputPinPadding())
					[
						NewPinRef
					];
				InputPins.Add(NewPinRef);
			}
			else // Direction == EEdGraphPinDirection::EGPD_Output
			{
				RightNodeBox->AddSlot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					.Padding(Settings->GetOutputPinPadding())
					[
						NewPinRef
					];
				OutputPins.Add(NewPinRef);
			}
		}

		if (PinBox.IsValid())
		{
			PinBox->SetContent(NewPinRef);

			if (NewPinRef->GetDirection() == EEdGraphPinDirection::EGPD_Input)
			{
				InputPins.Add(NewPinRef);
			}
			else
			{
				OutputPins.Add(NewPinRef);
			}
		}
		else
		{
			// TODO error handling
			UE_LOG(LogYapEditor, Warning, TEXT("Could not find pin box for pin %s"), *Pin->GetFName().ToString());
		}
	}
}

// ------------------------------------------------------------------------------------------------
const FYapFragment& SFlowGraphNode_YapDialogueWidget::GetFragment(uint8 FragmentIndex) const
{
	return GetFlowYapDialogueNode()->GetFragmentByIndex(FragmentIndex);
}

// ------------------------------------------------------------------------------------------------
FYapFragment& SFlowGraphNode_YapDialogueWidget::GetFragmentMutable(uint8 FragmentIndex)
{
	return GetFlowYapDialogueNodeMutable()->GetFragmentByIndexMutable(FragmentIndex);
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::CreateStandardPinWidget(UEdGraphPin* Pin)
{
	const bool bShowPin = ShouldPinBeHidden(Pin);

	if (bShowPin)
	{
		TSharedPtr<SGraphPin> NewPin = FNodeFactory::CreatePinWidget(Pin);
		check(NewPin.IsValid());

		this->AddPin(NewPin.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE
