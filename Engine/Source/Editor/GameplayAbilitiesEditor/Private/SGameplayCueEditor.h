// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SCompoundWidget.h"
#include "GameplayTagContainer.h"

/** Main CollisionAnalyzer UI widget */
class SGameplayCueEditor : public SUserWidget
{
public:

	SLATE_USER_ARGS(SGameplayCueEditor) {}
	SLATE_END_ARGS()
	virtual void Construct(const FArguments& InArgs) = 0;

	virtual void OnNewGameplayCueTagCommited(const FText& InText, ETextCommit::Type InCommitType) = 0;
	virtual void OnSearchTagCommited(const FText& InText, ETextCommit::Type InCommitType) = 0;
	virtual void HandleNotify_OpenAssetInEditor(FString AssetName, int AssetType) = 0;
	virtual void HandleNotify_FindAssetInEditor(FString AssetName, int AssetType) = 0;

	virtual FReply OnNewGameplayCueButtonPressed() = 0;
	
	static FString GetPathNameForGameplayCueTag(FString Tag);

	static void CreateNewGameplayCueNotifyDialogue(FString GameplayCue, UClass* ParentClass);
	static void OpenEditorForNotify(FString NotifyFullPath);
};
