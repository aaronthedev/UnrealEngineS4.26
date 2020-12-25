// Copyright 2017 Yaki Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Engine.h"
#include "Containers/Ticker.h"
#include "RootMotionExtractorGUI.h"


class FToolBarBuilder;
class FMenuBuilder;

class FRootMotionExtractorModule : public IModuleInterface, public FRootMotionExtractorGUI
{
	FTickerDelegate TickDelegate;
	FDelegateHandle TickDelegateHandle;
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	bool Tick(float DeltaTime);
	void Initialize();
	void StickPlugin();
	void PluginButtonClicked();

private:
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};