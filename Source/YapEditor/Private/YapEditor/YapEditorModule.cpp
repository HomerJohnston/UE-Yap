﻿// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapEditorModule.h"

#include "YapEditor/AssetFactory_YapCharacter.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/DetailCustomizations/DetailCustomization_YapProjectSettings.h"
#include "YapEditor/DetailCustomizations/DetailCustomization_YapCharacter.h"
#include "YapEditor/NodeWidgets/GameplayTagFilteredStyle.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FYapEditorModule::StartupModule()
{
	AssetCategory = { "Yap", LOCTEXT("Yap", "Yap") };
	
	AssetTypeActions.Add(MakeShareable(new FAssetTypeActions_FlowYapCharacter()));
	
	DetailCustomizations.Add({UYapProjectSettings::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_YapProjectSettings::MakeInstance)});
	DetailCustomizations.Add({UYapCharacter::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_YapCharacter::MakeInstance)});
	
	//PropertyCustomizations.Add(*FFlowYapFragment::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomization_FlowYapFragment::MakeInstance));
	
	StartupModuleBase();

	// TODO fix this retardation into my style proper
	FGameplayTagFilteredStyle::Initialize();

	// Force the style to load (for some reason stuff is not initalized on the first call to this otherwise???
	FYapEditorStyle::Get();
}

void FYapEditorModule::ShutdownModule()
{
	ShutdownModuleBase();
}

IMPLEMENT_MODULE(FYapEditorModule, FlowYapEditor)

#undef LOCTEXT_NAMESPACE