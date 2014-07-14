// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "K2Node_LeaderboardFlush.generated.h"

UCLASS()
class ONLINEBLUEPRINTSUPPORT_API UK2Node_LeaderboardFlush : public UK2Node_BaseAsyncTask
{
	GENERATED_UCLASS_BODY()

	// UEdGraphNode interface
	virtual FString GetTooltip() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// End of UEdGraphNode interface

	// Begin UK2Node interface
	virtual FText GetMenuCategory() const override;
	// End UK2Node interface
};
