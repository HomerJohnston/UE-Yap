// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

class IPropertyHandle;
class FDetailWidgetRow;
class IDetailChildrenBuilder;
class IDetailCategoryBuilder;
class IDetailGroup;
struct FGameplayTag;
struct FYapGroupSettings;
class FPropertyCustomization_YapGroupSettings : public IPropertyTypeCustomization
{
    // CONSTRUCTION -------------------------------------------------------------------------------

public:
    
    static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FPropertyCustomization_YapGroupSettings>(); }

    // STATE --------------------------------------------------------------------------------------
    
protected:

    FYapGroupSettings* Settings = nullptr;
        
    TArray<TSharedPtr<IPropertyHandle>> IndexedPropertyHandles;

    TMap<FName, TSharedPtr<IPropertyHandle>> PropertyBoolControlHandles;
    
    static TMap<FName, TSharedPtr<IPropertyHandle>> DefaultPropertyHandles;

    TMap<FString, TArray<TSharedPtr<IPropertyHandle>>> PropertyGroups;

    TMap<FName, int32> GroupOverridenCounts;

    int32 TotalOverrides = 0;

    TSharedPtr<IPropertyHandle> GroupColorPropertyHandle;

    TSharedPtr<SBox> HeaderColorPropertyHolder;
    
    TMap<FName, bool> CachedGameplayTagsPreviewTextsDirty;
    
    TMap<FName, FText> CachedGameplayTagsPreviewTexts;
    
    // OVERRIDES ----------------------------------------------------------------------------------

protected:
    
    void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    // METHODS ------------------------------------------------------------------------------------

protected:

    // Initial Setup
    
    void GrabOriginalStructPtr(TSharedRef<IPropertyHandle> StructPropertyHandle);
    
    void IndexChildrenProperties(TSharedRef<class IPropertyHandle> StructPropertyHandle);

    void UpdateOverriddenCounts();

    void HookUpPropertyChangeDelegates();
    
    bool IsDefault() const;

    bool IsOverridden(FName PropertyName) const;

    FLinearColor GetGroupColor();
    
    void GroupProperties();

    void GatherOverrides();

    void SortGroups();

    // Draw
    void DrawGroup(IDetailChildrenBuilder& StructBuilder, FString CategoryString, TArray<TSharedPtr<IPropertyHandle>>& PropertyHandles);

    void DrawProperty(IDetailChildrenBuilder& StructBuilder, IDetailGroup& Group, TSharedPtr<IPropertyHandle>& Property);

    void DrawDefaultProperty(IDetailChildrenBuilder& StructBuilder, IDetailGroup& Group, TSharedPtr<IPropertyHandle>& Property);

    void DrawNamedGroupProperty(IDetailChildrenBuilder& StructBuilder, IDetailGroup& Group, TSharedPtr<IPropertyHandle>& Property);
    
    void DrawTagExtraControls(IDetailGroup& Group, TSharedPtr<IPropertyHandle> ParentTagPropertyHandle, FText TagEditorTitle);
    
    void DrawDialogueTagsExtraControls(IDetailGroup& Group, TSharedPtr<IPropertyHandle> DialogueTagsParentProperty);

    void DrawExtraPanelContent(IDetailGroup& Group, TSharedPtr<IPropertyHandle> Property);
    
    const FSlateBrush* BorderImage() const;
    
    FText GetChildTagsAsText(TSharedPtr<IPropertyHandle> ParentTagProperty) const;

    bool IsTagPropertySet(TSharedPtr<IPropertyHandle> TagPropertyHandle) const;

    FGameplayTag& GetTagPropertyFromHandle(TSharedPtr<IPropertyHandle> TagPropertyHandle) const;
    
    FReply OnClicked_OpenTagsManager(FGameplayTag ParentTag, FText Title);
    
    FReply OnClicked_CleanupDialogueTags();

    FText GetDeletedTagsText(const TArray<FName>& TagNamesToDelete);
    
    void RecacheTagListText(TSharedPtr<IPropertyHandle> ParentTagProperty);
};