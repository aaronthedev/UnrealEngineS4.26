// Copyright2017 Yaki Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "RootMotionExtractorStyle.h"

class FRootMotionExtractorCommands : public TCommands<FRootMotionExtractorCommands>
{
public:

	FRootMotionExtractorCommands()
		: TCommands<FRootMotionExtractorCommands>(TEXT("RootMotionExtractor"), NSLOCTEXT("Contexts", "RootMotionExtractor", "RootMotionExtractor Plugin"), NAME_None, FRootMotionExtractorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};