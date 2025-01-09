// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"
#include "Yap/YapTimeMode.h"
#include "Yap/Enums/YapMaturitySetting.h"

class UYapCharacter;
class SYapConditionsScrollBox;
class UYapCondition;
class SObjectPropertyEntryBox;
class SMultiLineEditableText;
class SFlowGraphNode_YapDialogueWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;
class SMultiLineEditableTextBox;

struct FYapBit;
struct FFlowPin;
struct FYapBitReplacement;
struct FYapFragment;
struct FGameplayTag;

enum class EYapTimeMode : uint8;
enum class EYapMissingAudioBehavior : uint8;
enum class EYapErrorLevel : uint8;
enum class EYapMaturitySetting : uint8;

#define LOCTEXT_NAMESPACE "YapEditor"

enum class EYapFragmentControlsDirection : uint8
{
	Up,
	Down,
};

class SFlowGraphNode_YapFragmentWidget : public SCompoundWidget
{
	// ------------------------------------------
	// SETTINGS
	TMap<EYapTimeMode, FLinearColor> TimeModeButtonColors;
	
	// ------------------------------------------
	// STATE
protected:
	SFlowGraphNode_YapDialogueWidget* Owner = nullptr;

	TSharedPtr<SEditableTextBox> TitleTextBox;

	TSharedPtr<SWidget> SpeakerWidget;

	TSharedPtr<SWidget> DirectedAtWidget;
	
	bool bCursorContained = false;
	bool MoodKeySelectorMenuOpen = false;

	uint8 FragmentIndex = 0;

	bool bCtrlPressed = false;

	float Opacity = 0;

	double InitTime = -1;
	
	uint64 LastBitReplacementCacheFrame = 0;
	FYapBitReplacement* CachedBitReplacement = nullptr;

	bool bShowAudioSettings = false;
	float ExpandedTextEditorWidget_StartOffset = 0.f;
	float ExpandedTextEditorWidget_Offset = 0.f;
	float ExpandedTextEditorWidget_OffsetAlpha = 0.f;

	bool bEditingChildSafeSettings = false;
	bool ContainsChildSafeSettings() const;
	bool HasAnyChildSafeData() const;
	bool HasCompleteChildSafeData() const;
	
	EYapMaturitySetting DisplayingChildSafeData() const { return bEditingChildSafeSettings ? EYapMaturitySetting::ChildSafe : EYapMaturitySetting::Mature; }
	
	TSharedPtr<SBox> CentreBox;
	TSharedPtr<SOverlay> FragmentWidgetOverlay;
	TSharedPtr<SWidget> MoveFragmentControls = nullptr;
	TSharedPtr<SWidget> CentreDialogueWidget;
	TSharedPtr<SWidget> CenterSettingsWidget;
	FReply OnClicked_DialogueCornerButton();
	TSharedPtr<SWidget> CreateCentreTextDisplayWidget();
	TSharedPtr<SWidget> CreateCenterSettingsWidget();

	bool bTextEditorExpanded = false;
	TSharedPtr<SBox> ExpandedTextEditorWidget;
	TSharedPtr<SOverlay> FragmentOverlay;

	TSharedPtr<SButton> TitleTextEditButtonWidget;
	TSharedPtr<SButton> DialogueEditButtonWidget;
	TSharedPtr<SYapConditionsScrollBox> ConditionsScrollBox;

	TSharedPtr<SButton> TextExpanderButton;
public:
	TSharedPtr<SYapConditionsScrollBox> GetConditionsScrollBox() { return ConditionsScrollBox; }
	
	// ------------------------------------------
	// CONSTRUCTION
public:
	SLATE_USER_ARGS(SFlowGraphNode_YapFragmentWidget){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex); // non-virtual override
	bool IsBeingEdited();

	// ------------------------------------------
	// WIDGETS
protected:
	int32 GetFragmentActivationCount() const;
	int32 GetFragmentActivationLimit() const;
	EVisibility Visibility_FragmentControlsWidget() const;
	EVisibility Visibility_FragmentShiftWidget(EYapFragmentControlsDirection YapFragmentControlsDirection) const;
	FReply OnClicked_FragmentShift(EYapFragmentControlsDirection YapFragmentControlsDirection);
	FReply OnClicked_FragmentDelete();
	TSharedRef<SWidget> CreateFragmentControlsWidget();
	bool Enabled_AudioPreviewButton() const;
	TSharedRef<SWidget> CreateAudioPreviewWidget(const TSoftObjectPtr<UObject>* AudioAsset, TAttribute<EVisibility> Attribute);

	TSharedRef<SWidget> CreateFragmentHighlightWidget();
	void OnTextCommitted_FragmentActivationLimit(const FText& Text, ETextCommit::Type Arg);
	EVisibility Visibility_FragmentRowNormalControls() const;

	TSharedRef<SWidget> CreateUpperFragmentBar();
	EVisibility Visibility_FragmentTagWidget() const;
	
	ECheckBoxState IsChecked_SkippableToggle() const;
	FSlateColor ColorAndOpacity_SkippableToggleIcon() const;
	EVisibility Visibility_SkippableToggleIconOff() const;
	void OnCheckStateChanged_SkippableToggle(ECheckBoxState CheckBoxState);

	ECheckBoxState		IsChecked_MaturitySettings() const;
	void				OnCheckStateChanged_MaturitySettings(ECheckBoxState CheckBoxState);
	FSlateColor			ColorAndOpacity_ChildSafeSettingsCheckBox() const;
	
	FSlateColor BorderBackgroundColor_DirectedAtImage() const;
	void OnAssetsDropped_DirectedAtWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	bool OnAreAssetsAcceptableForDrop_DirectedAtWidget(TArrayView<FAssetData> AssetDatas) const;
	FReply OnClicked_DirectedAtWidget();
	const FSlateBrush* Image_DirectedAtWidget() const;
	TSharedRef<SWidget> CreateDirectedAtWidget();
	// ------------------------------------------
	TSharedRef<SWidget> CreateFragmentWidget();

	EVisibility Visibility_DialogueEdit() const;
	EVisibility Visibility_EmptyTextIndicator(const FText* Text) const;
	TOptional<float> Value_TimeSetting_Default() const;
	TOptional<float> Value_TimeSetting_AudioTime() const;
	TOptional<float> Value_TimeSetting_TextTime() const;
	TOptional<float> Value_TimeSetting_ManualTime() const;

	TSharedRef<SWidget>	MakeTimeSettingRow(EYapTimeMode TimeMode);

	TSharedRef<SWidget> CreateTimeSettingsWidget();
	FLinearColor ButtonColor_TimeSettingButton() const;
	// ------------------------------------------
	TSharedRef<SWidget>	CreateDialogueDisplayWidget();

	FVector2D			DialogueScrollBar_Thickness() const;
	FOptionalSize		Dialogue_MaxDesiredHeight() const;
	FText				Text_TextDisplayWidget(const FText* MatureText, const FText* SafeText) const;
	
	EVisibility			Visibility_DialogueBackground() const;
	FSlateColor			BorderBackgroundColor_Dialogue() const;
		
	// ------------------------------------------

	FText				FragmentTagPreview_Text() const;
	// ---------------------------------------------------
	TOptional<float>	FragmentTimePadding_Percent() const;
	TOptional<float> FragmentTime_Percent() const;

	float				Value_FragmentTimePadding() const;
	void				OnValueChanged_FragmentTimePadding(float X);
	FSlateColor			FillColorAndOpacity_FragmentTimePadding() const;
	FText				ToolTipText_FragmentTimePadding() const;

	FSlateColor BorderBackgroundColor_CharacterImage() const;
	void OnSetNewSpeakerAsset(const FAssetData& AssetData);
	void OnSetNewDirectedAtAsset(const FAssetData& AssetData);
	
	FReply OnClicked_SpeakerWidget(TSoftObjectPtr<UYapCharacter>* CharacterAsset, const UYapCharacter* Character);

	FText Text_SpeakerWidget() const;
	bool OnAreAssetsAcceptableForDrop_SpeakerWidget(TArrayView<FAssetData> AssetDatas) const;
	void OnAssetsDropped_SpeakerWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	
	// ------------------------------------------
	TSharedRef<SOverlay>	CreateSpeakerWidget();

	EVisibility			Visibility_PortraitImage() const;
	const FSlateBrush*	Image_SpeakerImage() const;
	EVisibility			Visibility_MissingPortraitWarning() const;
	EVisibility			Visibility_CharacterSelect() const;
	FString				ObjectPath_CharacterSelect() const;
	void				OnObjectChanged_CharacterSelect(const FAssetData& InAssetData);

	FText ToolTipText_MoodKeySelector() const;
	FSlateColor ForegroundColor_MoodKeySelectorWidget() const;
	// ------------------------------------------
	TSharedRef<SWidget>	CreateMoodKeySelectorWidget();

	EVisibility			Visibility_MoodKeySelector() const;
	void				OnMenuOpenChanged_MoodKeySelector(bool bMenuOpen);
	const FSlateBrush*	Image_MoodKeySelector() const;
	FGameplayTag		GetCurrentMoodKey() const;
	
	// ------------------------------------------
	TSharedRef<SWidget> CreateMoodKeyMenuEntryWidget(FGameplayTag InIconName, bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	FReply				OnClicked_MoodKeyMenuEntry(FGameplayTag NewValue);

	FText Text_EditedText(FText* Text) const;
	void OnTextCommitted_EditedText(const FText& NewValue, ETextCommit::Type CommitType, void (FYapBit::*Func)(FText* TextToSet, const FText& NewValue), FText* TextToSet);
	FReply OnClicked_TextDisplayWidget();

	FText ToolTipText_TextDisplayWidget(FText Label, const FText* MatureText, const FText* SafeText) const;
	FSlateColor ColorAndOpacity_TextDisplayWidget(FLinearColor BaseColor, const FText* MatureText, const FText* SafeText) const;
	
	// ------------------------------------------
	TSharedRef<SWidget> CreateTitleTextDisplayWidget();

	EVisibility			Visibility_TitleText() const;

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentTagWidget();
	
	FGameplayTag		Value_FragmentTag() const;
	void				OnTagChanged_FragmentTag(FGameplayTag GameplayTag);

	// ------------------------------------------

	FReply				OnClicked_SetTimeModeButton(EYapTimeMode TimeMode);

	void				OnValueCommitted_ManualTimeEntryBox(float NewValue, ETextCommit::Type CommitType);
	FSlateColor			ButtonColorAndOpacity_UseTimeMode(EYapTimeMode TimeMode, FLinearColor ColorTint) const;
	FSlateColor			ForegroundColor_TimeSettingButton(EYapTimeMode TimeMode, FLinearColor ColorTint) const;

	bool OnShouldFilterAsset_AudioAssetWidget(const FAssetData& AssetData) const;
	// ------------------------------------------
	TSharedRef<SWidget> CreateAudioAssetWidget(const TSoftObjectPtr<UObject>& Asset);

	FText				ObjectPathText_AudioAsset() const;
	FString				ObjectPath_AudioAsset() const;
	void				OnObjectChanged_AudioAsset(const FAssetData& InAssetData);
	EVisibility			Visibility_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const;

	FSlateColor			ColorAndOpacity_AudioSettingsButton() const;
	EYapErrorLevel		GetFragmentAudioErrorLevel() const;

	FSlateColor			ColorAndOpacity_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const;
	EYapErrorLevel		GetAudioAssetErrorLevel(const TSoftObjectPtr<UObject>& Asset) const;
	
	EVisibility			Visibility_AudioButton() const;

	// ------------------------------------------
	// HELPERS
protected:
	const UFlowNode_YapDialogue* GetDialogueNode() const;

	UFlowNode_YapDialogue* GetDialogueNode();

	const FYapFragment& GetFragment() const;
	
	FYapFragment& GetFragment();

	FYapFragment& GetFragmentMutable() const;

	const FYapBit& GetBit() const;
	
	FYapBit& GetBit();

	const FYapBit& GetBitConst();

	FYapBit& GetBitMutable() const;

	bool IsFragmentFocused() const;

	EVisibility			Visibility_RowHighlight() const;
	FSlateColor			BorderBackgroundColor_RowHighlight() const;

	// ------------------------------------------
	// OVERRIDES
public:
	FSlateColor GetNodeTitleColor() const; // non-virtual override

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	TSharedRef<SWidget>	CreateRightFragmentPane();

	TSharedPtr<SBox>	EndPinBox;
	TSharedPtr<SBox>	StartPinBox;
	TSharedPtr<SBox>	PromptOutPinBox;
	
	TSharedPtr<SBox> GetPinContainer(const FFlowPin& Pin);
	
	EVisibility			Visibility_EnableOnStartPinButton() const;
	EVisibility			Visibility_EnableOnEndPinButton() const;
	
	FReply				OnClicked_EnableOnStartPinButton();
	FReply				OnClicked_EnableOnEndPinButton();
};

#undef LOCTEXT_NAMESPACE