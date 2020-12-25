// Copyright2017 Yaki Studios, Inc. All Rights Reserved.

#include "RootMotionExtractorCommands.h"

#define LOCTEXT_NAMESPACE "FRootMotionExtractorModule"

void FRootMotionExtractorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "RootMotionExtractor", "Bring up RootMotionExtractor window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
