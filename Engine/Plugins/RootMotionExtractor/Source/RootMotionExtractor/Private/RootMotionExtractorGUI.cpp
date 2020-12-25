#include "RootMotionExtractorGUI.h"
#include "PropertyEditorModule.h"
#include "PropertyCustomizationHelpers.h"
#include "Runtime/Slate/Public/Widgets/Layout/SWidgetSwitcher.h"
#include "Editor/Persona/Public/BoneSelectionWidget.h"
#include "Runtime/Slate/Public/Widgets/Input/SSearchBox.h"
#include "Editor/UnrealEd/Public/Dialogs/DlgPickPath.h"
#include "RMEUtilities.h"
#define LOCTEXT_NAMESPACE "FRootMotionExtractorGUI"

DEFINE_LOG_CATEGORY(RMELog);
void FRootMotionExtractorGUI::ShowAddBoneWindow(const TSharedPtr <FBoneTask> BoneTask) {


	FText WindowTitle;
	GUI_SelectedBoneName = FName(TEXT(""));
	if (BoneTask.IsValid()) {
		WindowTitle = FText::FromString(TEXT("Edit Bone"));
	}
	else {
		WindowTitle = FText::FromString(TEXT("Add Bone"));
	}

	if (!AddBoneWindowContainer.IsValid()) {
		SAssignNew(AddBoneWindowContainer, SWindow)
			.ClientSize(FVector2D(500, 680))
			.CreateTitleBar(true)
			.Title(WindowTitle)
			.SupportsMaximize(false)
			.SupportsMinimize(true)
			.FocusWhenFirstShown(true)
			.IsTopmostWindow(true)
			.SizingRule(ESizingRule::FixedSize);
		AddBoneWindowContainer->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FRootMotionExtractorGUI::OnAddBoneWindowClosed));
		AddBoneWindowContainer->SetContent(
			FRootMotionExtractorGUI::GenerateAddBoneWindow(BoneTask)
		);
		FSlateApplication::Get().AddWindow(AddBoneWindowContainer.ToSharedRef(), false);
		if (AddBoneWindowContainer.IsValid()) {
			AddBoneWindowContainer->ShowWindow();
		}
	}
}


void FRootMotionExtractorGUI::ShowMainWindow() {
	if (!WindowContainer.IsValid()) {
		SAssignNew(WindowContainer, SWindow)
			.ClientSize(FVector2D(500, 900 /*900*/))
			.CreateTitleBar(true)
			.Title(FText::FromString(TEXT("Root Motion Extractor © Yaki Studios")))
			.SupportsMaximize(false)
			.SupportsMinimize(true)
			.FocusWhenFirstShown(true)
			.IsTopmostWindow(true)
			.SizingRule(ESizingRule::FixedSize);
		WindowContainer->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FRootMotionExtractorGUI::OnMainWindowClosed));
		WindowContainer->SetContent(
			FRootMotionExtractorGUI::GenerateWindow()
		);
		FSlateApplication::Get().AddWindow(WindowContainer.ToSharedRef(), false);
		if (WindowContainer.IsValid()) {
			WindowContainer->ShowWindow();
		}
	}
}

/// <summary>
/// Do some cleanup when Add Bone window is closed.
/// </summary>
/// <param name="WindowBeingClosed">Plugin window refference</param>
void FRootMotionExtractorGUI::OnAddBoneWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed)
{
	//Reset all variables and clean up
	GUI_SelectedBoneName = FName(TEXT(""));
	AddBoneWindowContainer.Reset();
}

/// <summary>
/// Do some cleanup when Add Bone window is closed.
/// </summary>
/// <param name="WindowBeingClosed">Plugin window refference</param>
FReply FRootMotionExtractorGUI::AddBoneWindowCloseButton()
{
	GUI_SelectedBoneName = FName(TEXT(""));
	AddBoneWindowContainer->RequestDestroyWindow();
	AddBoneWindowContainer.Reset();
	return FReply::Handled();

}


/// <summary>
/// Do some cleanup when plugin window is closed.
/// </summary>
/// <param name="WindowBeingClosed">Plugin window refference</param>
void FRootMotionExtractorGUI::OnMainWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed)
{
	//Reset all variables and clean up
	GUI_SelectedBoneName = FName(TEXT(""));
	WindowContainer.Reset();
	if (AddBoneWindowContainer.IsValid()) {
		AddBoneWindowContainer->RequestDestroyWindow();
	}
	RMEEngine::ExtractionType = CaptureTypes[0];
	AddBoneWindowContainer.Reset();
}

const FReferenceSkeleton & FRootMotionExtractorGUI::GetSkeletonRef() {
	if (SkelFile.IsValid()) {
		USkeleton *LocalSkel = Cast<USkeleton>(SkelFile.GetAsset());
		if (LocalSkel) {
			return LocalSkel->GetReferenceSkeleton();
		}
	}
	return RefEmpty;
}


TSharedRef < SWidget > FRootMotionExtractorGUI::GenerateAddBoneWindow(const TSharedPtr <FBoneTask> BoneTask) {
	const FSlateFontInfo Header = FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 15);

	bool bIsEditingBone = false;
	FName BoneEdit;

	if (BoneTask.IsValid()) {
		GUI_SelectedBoneName = BoneTask.Get()->BoneName;
		SkelFile = BoneTask.Get()->SkeletonFile;
	}

	return
	SNew(SToolTip)
	[
		SNew(SCanvas)
		+ SCanvas::Slot()
		.Position(FVector2D(5, 5))
		.Size(FVector2D(480, 750))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(5.0f, 5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				.MaxHeight(20)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Bone Name")))
					.Font(Header)
				]
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor(48.0f, 48.0f, 48.0f, 1.0f)))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						.FillHeight(0.1f)
						[
							SNew(SBox)
							.HeightOverride(30.f)
							.WidthOverride(450.f)
							[
								SNew(SObjectPropertyEntryBox)
								.ObjectPath_Raw(this, &FRootMotionExtractorGUI::GetSkelAssetName)
								.OnObjectChanged_Raw(this, &FRootMotionExtractorGUI::OnSkelAssetSelected)
								.AllowClear(false)
								.AllowedClass(FRootMotionExtractorGUI::OnGetClassesForSkelPicker())
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SAssignNew(MenuSkelWidget, SBoneTreeMenu)
							#if ENGINE_MINOR_VERSION > 17
								.bShowSocket(false)
							#endif
							.OnGetReferenceSkeleton_Raw(this, &FRootMotionExtractorGUI::GetSkeletonRef)
							.OnBoneSelectionChanged_Raw(this, &FRootMotionExtractorGUI::OnBoneNameChanged)
							.SelectedBone(GUI_SelectedBoneName)
						]
					]
				]
			]
		]
		+ SCanvas::Slot()
		.Position(FVector2D(5, 400))
		.Size(FVector2D(480, 360))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(5.0f, 5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				.MaxHeight(20)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Capture Channels")))
					.Font(Header)
				]
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor(48.0f, 48.0f, 48.0f, 1.0f)))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_MovementSpeed, false)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_MovementSpeed, false)
					
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Movement Speed")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Right, false)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Right, false)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Right Movement")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Forward, false)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Forward, false)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Forward Movement")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Up, false)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Up, false)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Up Movement")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Yaw, false)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Yaw, false)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Yaw")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Pitch, false)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Pitch, false)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Pitch")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Roll, false)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Roll, false)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Roll")))
							]
						]
					]
				]
			]
		]
		+ SCanvas::Slot()
		.Position(FVector2D(5, 600))
		.Size(FVector2D(480, 50))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(5.0f, 5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.8f)
					[
						SNew(SBox)
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.2f)
					.HAlign(HAlign_Right)
					[
						SNew(SButton).Text(FText::FromString(" Close "))
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.OnClicked_Raw(this, &FRootMotionExtractorGUI::AddBoneWindowCloseButton)
					]
				]
			]
		]
	];
}

void FRootMotionExtractorGUI::OnBoneNameChanged(FName BoneName) {
	TSharedPtr <FBoneTask> *FoundBoneEntry;
	const FName OldBoneName = GUI_SelectedBoneName;
	if (OldBoneName.ToString() != "") {
		if (RMEEngine::IsBoneSelected(OldBoneName, FoundBoneEntry)) {
			FoundBoneEntry->Get()->SkeletonFile = SkelFile;
			FoundBoneEntry->Get()->BoneName = BoneName;
			for (auto &BoneChannel : FoundBoneEntry->Get()->BoneChannels) {
				BoneChannel.Curve.DisplayName = RMEUtilities::GenerateCurveDisplayName(BoneName, BoneChannel.CaptureChannel);
			}
			BoneListView->RequestListRefresh();
		}
	}
	GUI_SelectedBoneName = BoneName;
}

TSharedRef < SWidget > FRootMotionExtractorGUI::GenerateWindow() {
	const FSlateFontInfo Header = FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 15); 
	const bool bIsEnabled = false;

	RMEEngine::ExtractionType = CaptureTypes[0];

	return
	SNew(SToolTip)
	[
		SNew(SCanvas)
		+ SCanvas::Slot()
		.Position(FVector2D(5,5))
		.Size(FVector2D(480, 330))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(5.0f, 5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				.MaxHeight(20)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Resources")))
					.Font(Header)
				]
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor(48.0f, 48.0f, 48.0f, 1.0f)))
					[
						SNew(SWrapBox)
						+ SWrapBox::Slot()
						.Padding(2.0f, 2.0f)
						.VAlign(VAlign_Center)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.Padding(2.0f, 5.0f)
							[
								SAssignNew(CaptureTypeOptions, SComboBox< TSharedPtr<FCaptureType>>)
								.OptionsSource(&CaptureTypes)
								.OnSelectionChanged_Raw(this, &FRootMotionExtractorGUI::OnCaptureTypeChanged)
								.OnGenerateWidget_Raw(this, &FRootMotionExtractorGUI::MakeWidgetForCaptureType)
								.InitiallySelectedItem(RMEEngine::ExtractionType)
								  [
								     SNew(STextBlock)
									  .Text_Raw(this, &FRootMotionExtractorGUI::GetCurrentCaptureTypeLabel)
								   ]
							]
						]
						+ SWrapBox::Slot()
						.Padding(0,10)
						.VAlign(VAlign_Center)
						[
							SAssignNew(WidgetSwitcher, SWidgetSwitcher)
							+ SWidgetSwitcher::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.Padding(5.0f, 5.0f)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.FillHeight(0.6f)
									[
										SNew(STextBlock).Text(FText::FromString(TEXT("Source Folder:")))
									]
									+ SVerticalBox::Slot()
									.FillHeight(0.6f)
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.FillWidth(1.7f)
										.VAlign(VAlign_Center)
										[
											SNew(SBox)
											.HeightOverride(30.f)
											.WidthOverride(400.f)
											[
												SNew(SEditableTextBox).Text_Raw(this, &FRootMotionExtractorGUI::GetSourceDir).IsEnabled(bIsEnabled)
											]
										]
										+ SHorizontalBox::Slot()
										.FillWidth(0.3f)
										.VAlign(VAlign_Center)
										[
											SNew(SBox)
											.HeightOverride(30.f)
											.WidthOverride(50.f)
											[
												SNew(SButton)
												.Text(FText::FromString(TEXT("Browse")))
												.HAlign(HAlign_Center)
												.OnClicked_Raw(this, &FRootMotionExtractorGUI::SourceDirButton)
											]
										]
									]
								]
								+ SVerticalBox::Slot()
								.Padding(5.0f, 5.0f)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.FillHeight(0.6f)
									[
										SNew(STextBlock).Text(FText::FromString(TEXT("Target Folder:")))
									]
									+ SVerticalBox::Slot()
									.FillHeight(0.6f)
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.FillWidth(1.7f)
										.VAlign(VAlign_Center)
										[
											SNew(SBox)
											.HeightOverride(30.f)
											.WidthOverride(400.f)
											[
												SNew(SEditableTextBox).Text_Raw(this, &FRootMotionExtractorGUI::GetTargetDir).IsEnabled(bIsEnabled)
											]
										]
										+ SHorizontalBox::Slot().FillWidth(0.3f)
										.VAlign(VAlign_Center)
										[
											SNew(SBox)
											.HeightOverride(30.f)
											.WidthOverride(50.f)
											[
												SNew(SButton)
												.Text(FText::FromString(TEXT("Browse")))
												.HAlign(HAlign_Center)
												.OnClicked_Raw(this, &FRootMotionExtractorGUI::TargetDirButton)
											]
										]
									]
								]
							]
							+ SWidgetSwitcher::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.Padding(5.0f, 5.0f)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.FillHeight(0.6f)
									[
										SNew(STextBlock).Text(FText::FromString(TEXT("Source Animation:")))
									]
									+ SVerticalBox::Slot()
									.FillHeight(1.4f)
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.VAlign(VAlign_Center)
										[
											SNew(SBox)
											.HeightOverride(30.f)
											.WidthOverride(450.f)
											[
												SNew(SObjectPropertyEntryBox)
												.ObjectPath_Raw(this, &FRootMotionExtractorGUI::GetSourceAnimAssetName)
												.OnObjectChanged_Raw(this, &FRootMotionExtractorGUI::OnSourceAnimAssetSelected)
												.AllowClear(false)
												.AllowedClass(FRootMotionExtractorGUI::OnGetClassesForSourcePicker())
												.OnShouldFilterAsset_Raw(this, &FRootMotionExtractorGUI::FilterSourcePicker)
											]
										]
									]
								]
								+ SVerticalBox::Slot()
								.Padding(5.0f, 5.0f)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.FillHeight(0.6f)
									[
										SNew(STextBlock).Text(FText::FromString(TEXT("Target:")))
									]
									+ SVerticalBox::Slot()
									.FillHeight(1.4f)
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.FillWidth(1.5f)
										.VAlign(VAlign_Center)
										[
											SNew(SBox)
											.HeightOverride(30.f)
											.WidthOverride(450.f)
											[
												SNew(SObjectPropertyEntryBox)
												.ObjectPath_Raw(this, &FRootMotionExtractorGUI::GetTargetAssetName)
												.OnObjectChanged_Raw(this, &FRootMotionExtractorGUI::OnTargetAssetSelected)
												.AllowClear(false)
												.AllowedClass(FRootMotionExtractorGUI::OnGetClassesForTargetPicker())
												.OnShouldFilterAsset_Raw(this, &FRootMotionExtractorGUI::FilterTargetPicker)
											]
										]
									]
								]
							]
						]
					]
				]
			]
		]
		+ SCanvas::Slot()
		.Position(FVector2D(5, 200))
		.Size(FVector2D(480, 350))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(5.0f, 5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				.MaxHeight(20)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Root Motion Channels")))
					.Font(Header)
				]
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor(48.0f, 48.0f, 48.0f, 1.0f)))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_MovementSpeed, true).IsChecked(ECheckBoxState::Checked)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_MovementSpeed, true)
					
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Movement Speed")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Right, true)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Right, true)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Right Movement")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Forward, true)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Forward, true)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Forward Movement")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Up, true)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Up, true)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Up Movement")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Yaw, true)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Yaw, true)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Yaw")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Pitch, true)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Pitch, true)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Pitch")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::SetCaptureChannel, ECaptureChannel::CC_Roll, true)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::GUIIsChannelSelected, ECaptureChannel::CC_Roll, true)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Capture Roll")))
							]
						]
					]
				]
			]
		]
		+ SCanvas::Slot()
		.Position(FVector2D(5, 400))
		.Size(FVector2D(480, 250))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(5.0f, 5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				.MaxHeight(20)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Bones")))
					.Font(Header)
				]
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor(48.0f, 48.0f, 48.0f, 1.0f)))
					[

						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(0.7f)
						[
							SNew(SScrollBox)
							.Orientation(EOrientation::Orient_Vertical)
							+ SScrollBox::Slot()
							[
								SAssignNew(BoneListView, SListView< TSharedPtr<FBoneTask>>)
								.ListItemsSource(&RMEEngine::BoneTasks)
								.ItemHeight(24)
								.SelectionMode(ESelectionMode::Single)
								.OnGenerateRow_Raw(this, &FRootMotionExtractorGUI::OnGenerateRowForList)
								.OnSelectionChanged_Raw(this, &FRootMotionExtractorGUI::BoneEntrySelectionChanged)
								.OnMouseButtonDoubleClick_Raw(this, &FRootMotionExtractorGUI::BoneEntryDoubleClicked)
							]
						]
						+ SHorizontalBox::Slot()
						.Padding(2.0f, 2.0f)
						.FillWidth(0.3f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							[
								SNew(SButton).Text(FText::FromString(TEXT("+")))
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked_Raw(this, &FRootMotionExtractorGUI::AddBonePressed)
							]
							+ SVerticalBox::Slot()
							[
								SNew(SButton).Text(FText::FromString(TEXT("-")))
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked_Raw(this, &FRootMotionExtractorGUI::DeleteBonePressed)
							]
						]
					]
				]
			]
		]
		+ SCanvas::Slot()
		.Position(FVector2D(5, 545))
		.Size(FVector2D(480, 430 /*420*/))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(5.0f, 5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				.MaxHeight(20)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Options")))
					.Font(Header)
				]
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor(48.0f, 48.0f, 48.0f, 1.0f)))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::DeleteAllCurveDataChanged)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::IsDeleteAllCurveData)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Delete All Existing Curve Data")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::IncrementMovementChanged)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::IsIncrementMovement)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Enable Incremental Movement")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::IncrementRotationChanged)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::IsIncrementRotation)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Enable Incremental Rotation")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged_Raw(this, &FRootMotionExtractorGUI::AutoCreateTargetCopyChanged)
								.IsChecked_Raw(this, &FRootMotionExtractorGUI::IsAutoCreateTargetCopy)
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Create In-Place target copies")))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(5.0f, 5.0f)
						[
							SNew(SWrapBox)
							.PreferredWidth(300.f)
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("Curve Multiplier: ")))
							]
							+ SWrapBox::Slot()
							.VAlign(VAlign_Center)
							[
								SNew(SBox).MinDesiredWidth(50)
								[
									SNew(SFloatWidget).Float(RMEEngine::RateScale).GetFloatUpdated_Raw(this, &FRootMotionExtractorGUI::UpdateRateScale)
								]
							]
						]
					]
				]
			]			
		]
		+ SCanvas::Slot()
		.Position(FVector2D(5, 790/*790*/))
		.Size(FVector2D(480, 50))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(5.0f, 5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor(48.0f, 48.0f, 48.0f, 1.0f)))
					[
						SNew(SProgressBar).Percent_Raw(this, &FRootMotionExtractorGUI::GetPercentage)
					]
				]
			]
		]
		+ SCanvas::Slot()
		.Position(FVector2D(5, 830 /*830*/))
		.Size(FVector2D(480, 50))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(5.0f, 5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2.0f, 2.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor(48.0f, 48.0f, 48.0f, 1.0f)))
					[
						SNew(SButton).Text_Raw(this, &FRootMotionExtractorGUI::GetExtractButtonCaption)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
			            .OnClicked_Raw(this, &FRootMotionExtractorGUI::ExtractPressed)
					]
				]
			]
		]
	];
}


TSharedRef<SWidget> FRootMotionExtractorGUI::MakeWidgetForCaptureType(TSharedPtr <FCaptureType> InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption.Get()->CaptureLabel));
}

void FRootMotionExtractorGUI::OnCaptureTypeChanged(TSharedPtr <FCaptureType> NewValue, ESelectInfo::Type)
{
	RMEEngine::ExtractionType = NewValue;

	if (RMEEngine::ExtractionType.Get()->CaptureType == ECaptureType::CT_Bulk) {
		if (WidgetSwitcher.IsValid()) {
			WidgetSwitcher->SetActiveWidgetIndex(0);
			RMEEngine::SourceFile = NULL;
			RMEEngine::TargetFile = NULL;
		}
	}
	else {
		if (WidgetSwitcher.IsValid()) {
			WidgetSwitcher->SetActiveWidgetIndex(1);
			RMEEngine::SourceDir = "";
			RMEEngine::TargetDir = "";
		}
	}

}

FText FRootMotionExtractorGUI::GetCurrentCaptureTypeLabel() const
{
	if (RMEEngine::ExtractionType.IsValid()) {
		return FText::FromString(*RMEEngine::ExtractionType.Get()->CaptureLabel);
	}
	else {
		return FText::FromString(FString(""));
	}
}


void FRootMotionExtractorGUI::BoneEntrySelectionChanged(TSharedPtr<FBoneTask> InItem, ESelectInfo::Type SelectInfo) {
	if (InItem.IsValid()) {
		SelectedItem = InItem;
	}
	else {
		SelectedItem = NULL;
	}
}

void FRootMotionExtractorGUI::BoneEntryDoubleClicked(TSharedPtr<FBoneTask> InItem) {
	if (InItem.IsValid()) {
		if (AddBoneWindowContainer.IsValid()) {
			AddBoneWindowContainer->RequestDestroyWindow();
			AddBoneWindowContainer.Reset();
		}
		FRootMotionExtractorGUI::ShowAddBoneWindow(InItem);
	}
}

FReply FRootMotionExtractorGUI::AddBonePressed()
{
	if (AddBoneWindowContainer.IsValid()) {
		AddBoneWindowContainer->RequestDestroyWindow();
		AddBoneWindowContainer.Reset();
	}
	FRootMotionExtractorGUI::ShowAddBoneWindow();
	return FReply::Handled();
}

FReply FRootMotionExtractorGUI::DeleteBonePressed()
{
	if (SelectedItem.IsValid()) {
		FName BoneName = SelectedItem.Get()->BoneName;
		auto FoundBoneEntry = RMEEngine::BoneTasks.IndexOfByPredicate([BoneName](const TSharedPtr <FBoneTask> & InItem)
		{
			return InItem.Get()->BoneName == BoneName;
		});
		if (FoundBoneEntry != INDEX_NONE) {
			RMEEngine::BoneTasks.RemoveAt(FoundBoneEntry, 1, true);
			BoneListView->RequestListRefresh();
		}
	}
	return FReply::Handled();
}

TSharedRef<ITableRow> FRootMotionExtractorGUI::OnGenerateRowForList(TSharedPtr<FBoneTask> Item, const TSharedRef<STableViewBase> &OwnerTable)
{
	//Create the row
	if (Item.Get()->BoneName.ToString() != "Root Motion") {
		return
			SNew(STableRow< TSharedPtr<FBoneTask> >, OwnerTable)
			.Padding(2.0f)
			[
				SNew(STextBlock).Text_Raw(this, &FRootMotionExtractorGUI::GetBoneNameForList, Item)
			];
	}
	else {
		return
			SNew(STableRow< TSharedPtr<FBoneTask> >, OwnerTable)
			[
				SNew(SBox)
			];
	}
	
}

FText FRootMotionExtractorGUI::GetBoneNameForList(TSharedPtr<FBoneTask> Item) const
{
	return FText::FromString(Item.Get()->BoneName.ToString());
}


/// <summary>
/// Validates selection state of capture chanel
/// </summary>
/// <param name="newState">New selection state of capture channel</param>
/// <param name="SelectedChannel">Selected Channel</param>
void FRootMotionExtractorGUI::SetCaptureChannel(ECheckBoxState newState, ECaptureChannel SelectedChannel, bool bIsRootMotion) {
	FName BoneName;
	if (bIsRootMotion) {
		BoneName = FName(TEXT("Root Motion"));
	}
	else {
		BoneName = GUI_SelectedBoneName;
	}

	if (!BoneName.IsNone() && BoneName.IsValid() && BoneName.ToString().Len() > 0) {
		if (!RMEEngine::bOperationStarted) {
			if (newState == ECheckBoxState::Checked) {
				EAssetType ValidateTargetAsset = RMEUtilities::GetAssetType(RMEEngine::TargetFile);
				if (ValidateTargetAsset == EAssetType::AT_CurveFloat) {
					if (RMEEngine::GetSelectedChannelsCount() >= 1) {
						FRootMotionExtractorGUI::DisplayMessage(TEXT("Selected target is a Float Curve asset. You can't capture more than one channel."));
						UE_LOG(RMELog, Error, TEXT("Selected target is a Float Curve asset. You can't capture more than one channel."));
						return;
					}
				}
				bool bRefreshList = false;
				RMEEngine::AddBoneChannel(BoneName, SkelFile, SelectedChannel, bRefreshList);
				if (bRefreshList) BoneListView->RequestListRefresh();
			}
			else {
				bool bRefreshList = false;
				RMEEngine::RemoveBoneChannel(BoneName, SelectedChannel, bRefreshList);
				if(bRefreshList) BoneListView->RequestListRefresh();
			}
		}
		else {
			FRootMotionExtractorGUI::DisplayMessage(TEXT("Extraction already in progress."));
			UE_LOG(RMELog, Error, TEXT("Extraction already in progress."));
		}
	}
	else {
		FRootMotionExtractorGUI::DisplayMessage(TEXT("Please select a bone first."));
	}
}


/// <summary>
/// Return capture channel selection state to slate checkbox
/// </summary>
/// <returns>ECheckBoxState as selection state</returns>
/// 
ECheckBoxState FRootMotionExtractorGUI::GUIIsChannelSelected(ECaptureChannel CaptureChannel, bool bIsRootMotion) const {

	FName BoneName;
	if (bIsRootMotion) {
		BoneName = FName(TEXT("Root Motion"));
	}
	else {
		BoneName = GUI_SelectedBoneName;
	}
	const TSharedPtr <FBoneTask> *FoundBoneEntry = RMEEngine::BoneTasks.FindByPredicate([BoneName](const TSharedPtr <FBoneTask>& InItem)
	{
		return InItem.Get()->BoneName == BoneName;
	});

	if (FoundBoneEntry) {
		const FCurveElem *FoundChannel = FoundBoneEntry->Get()->BoneChannels.FindByPredicate([CaptureChannel](const FCurveElem& InItem)
		{
			return InItem.CaptureChannel == CaptureChannel;
		});
		if (FoundChannel) {
			return ECheckBoxState::Checked;
		}

	}
	return ECheckBoxState::Unchecked;
}

void FRootMotionExtractorGUI::UpdateRateScale(float Value) {
	RMEEngine::RateScale = Value;
}


/// <summary>
/// Return DeleteAllCurveData
/// </summary>
/// <returns></returns>
ECheckBoxState FRootMotionExtractorGUI::IsDeleteAllCurveData() const
{
	return RMEEngine::bDeleteAllCurveData == false ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
}

ECheckBoxState FRootMotionExtractorGUI::IsIncrementRotation() const
{
	return RMEEngine::bIncrementalRotation == false ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
}

ECheckBoxState FRootMotionExtractorGUI::IsAutoCreateTargetCopy() const
{
	return RMEEngine::bCreateTargetCopy == false ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
}

ECheckBoxState FRootMotionExtractorGUI::IsIncrementMovement() const
{
	return RMEEngine::bIncrementalMovement == false ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
}
////////////////////

/// <summary>
/// Enable Delete all existing curve data when writing to file
/// </summary>
/// <param name="newState">DeleteAllCurveData checkbox state</param>
void FRootMotionExtractorGUI::DeleteAllCurveDataChanged(const ECheckBoxState newState)
{
	if (newState == ECheckBoxState::Checked) {
		RMEEngine::bDeleteAllCurveData = true;
	}
	else {
		RMEEngine::bDeleteAllCurveData = false;
	}
}
void FRootMotionExtractorGUI::IncrementRotationChanged(const ECheckBoxState newState)
{
	if (newState == ECheckBoxState::Checked) {
		RMEEngine::bIncrementalRotation = true;
	}
	else {
		RMEEngine::bIncrementalRotation = false;
	}
}

void FRootMotionExtractorGUI::AutoCreateTargetCopyChanged(const ECheckBoxState newState)
{
	if (newState == ECheckBoxState::Checked) {
		RMEEngine::bCreateTargetCopy = true;
	}
	else {
		RMEEngine::bCreateTargetCopy = false;
	}
}

void FRootMotionExtractorGUI::IncrementMovementChanged(const ECheckBoxState newState)
{
	if (newState == ECheckBoxState::Checked) {
		RMEEngine::bIncrementalMovement = true;
	}
	else {
		RMEEngine::bIncrementalMovement = false;
	}
}
////////////////////



/// <summary>
/// Returns the allowed classes for source asset picker.
/// </summary>
/// <param name="AllowedClasses"></param>
UClass* FRootMotionExtractorGUI::OnGetClassesForSourcePicker()
{
	return UObject::StaticClass();
}

/// <summary>
/// Returns the allowed classes for target asset picker.
/// </summary>
/// <param name="AllowedClasses"></param>
UClass* FRootMotionExtractorGUI::OnGetClassesForTargetPicker() {
	return UObject::StaticClass();
}

UClass * FRootMotionExtractorGUI::OnGetClassesForSkelPicker()
{
	return USkeleton::StaticClass();
}

/// <summary>
/// Filter source asset picker
/// </summary>
/// <param name="AssetData">target asset</param>
/// <returns>true if target asset is of class</returns>
bool FRootMotionExtractorGUI::FilterSourcePicker(const FAssetData & AssetData) const
{
	auto AssetType = RMEUtilities::GetAssetType(AssetData);

	switch (AssetType) {
	case EAssetType::AT_Montage:
		return true; //disable to show montages in source picker
		break;
	case EAssetType::AT_Animation:
		return false;
		break;
	default:
		return true;
		break;
	}
}

/// <summary>
/// Filter target asset picker
/// </summary>
/// <param name="AssetData">target asset</param>
/// <returns>true if target asset is of class</returns>
bool FRootMotionExtractorGUI::FilterTargetPicker(const FAssetData & AssetData) const
{
	auto AssetType = RMEUtilities::GetAssetType(AssetData);

	switch (AssetType) {
	case EAssetType::AT_Montage:
		return true; //disable to show montages in target picker
		break;
	case EAssetType::AT_Animation:
		return false;
		break;
	case EAssetType::AT_CurveFloat:
		return false;
		break;
	default:
		return true;
		break;
	}
}


/// <summary>
/// Validate the source asset and check if there is root motion in the animation.
/// </summary>
/// <param name="AssetData">Selected source asset.</param>
void FRootMotionExtractorGUI::OnSourceAnimAssetSelected(const FAssetData& AssetData) {
	if (AssetData.IsValid()) {
		UAnimSequence *LocalSource = Cast<UAnimSequence>(AssetData.GetAsset());
		if (LocalSource) {
			RMEEngine::SourceFile = AssetData;
		}
	}
}
void FRootMotionExtractorGUI::OnSkelAssetSelected(const FAssetData & AssetData)
{
	if (AssetData.IsValid()) {
		SkelFile = AssetData;
	}
	if (MenuSkelWidget.IsValid()) {
		#if ENGINE_MINOR_VERSION < 20
			MenuSkelWidget.Get()->FilterTextWidget->SetText(FText::FromString(" "));
			MenuSkelWidget.Get()->FilterTextWidget->SetText(FText::FromString(""));
		#else
		if (MenuSkelWidget.Get()->GetFilterTextWidget().Get()->GetType() == "SSearchBox")
		{
			SSearchBox* FilterTextWidget = static_cast<SSearchBox*>(MenuSkelWidget.Get()->GetFilterTextWidget().Get());
			FilterTextWidget->SetText(FText::FromString(" "));
			FilterTextWidget->SetText(FText::FromString(""));
		}
		#endif

	}
}
/// <summary>
/// Validate the target asset.
/// </summary>
/// <param name="AssetData">Selected target asset.</param>
void FRootMotionExtractorGUI::OnTargetAssetSelected(const FAssetData& AssetData) {
	if (AssetData.IsValid()) {
		RMEEngine::TargetFile = AssetData;
	}
}

/// <summary>
/// Return the source asset name to the slate asset picker.
/// </summary>
/// <returns>Source asset name</returns>
FString FRootMotionExtractorGUI::GetSourceAnimAssetName() const {
	//Return source target asset name.
	if (RMEEngine::SourceFile.IsValid())
		return RMEEngine::SourceFile.ObjectPath.ToString();
	else
		return FString("");
}
/// <summary>
/// Return the target asset name to the slate asset picker.
/// </summary>
/// <returns>Target asset name</returns>
FString FRootMotionExtractorGUI::GetTargetAssetName() const {
	//Return selected target asset name.
	if (RMEEngine::TargetFile.IsValid())
		return RMEEngine::TargetFile.ObjectPath.ToString();
	else
		return FString("");
}

FString FRootMotionExtractorGUI::GetSkelAssetName() const
{
	//Return selected target asset name.
	if (SkelFile.IsValid())
		return SkelFile.ObjectPath.ToString();
	else
		return FString("");
}

/// <summary>
/// Returns to the slate progressbar the percentage of the running extraction if in progress.
/// </summary>
/// <returns>Extraction completion percentage</returns>
TOptional<float> FRootMotionExtractorGUI::GetPercentage() const
{
	if (RMEEngine::FoundSourceFiles > 0) {
		float Percent = (float)RMEEngine::ProcessedFiles / RMEEngine::FoundSourceFiles;
		return Percent;
	}
	return 0.f;
}

/// <summary>
/// Display modal window.
/// </summary>
/// <param name="Message">Message to display in the modal window</param>
void FRootMotionExtractorGUI::DisplayMessage(FString Message)
{
	if (WindowContainer.IsValid()) {
		WindowContainer->Minimize();
	}
	if (AddBoneWindowContainer.IsValid()) {
		AddBoneWindowContainer->Minimize();
	}

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));

	if (WindowContainer.IsValid()) {
		WindowContainer->BringToFront();
	}
	if (AddBoneWindowContainer.IsValid()) {
		AddBoneWindowContainer->BringToFront();
	}
}


/// <summary>
/// Extract button was pressed and we need to proceed with some validation.
/// </summary>
/// <returns>handled status.</returns>
FReply FRootMotionExtractorGUI::ExtractPressed()
{
	//Extract was pressed, validate request.
	UAnimSequence* Source = NULL;
	if (!RMEEngine::bOperationStarted) {
		switch (RMEEngine::ExtractionType.Get()->CaptureType) {
		case ECaptureType::CT_Single:
			if (!RMEEngine::EnqueueSingleExtraction(RMEEngine::SourceFile, RMEEngine::TargetFile, true)) {
				DisplayMessage(RMEEngine::LastError);
				UE_LOG(RMELog, Error, TEXT(" %s "), *RMEEngine::LastError);
			}
			break;
		case ECaptureType::CT_Bulk:
			if (!RMEEngine::StartBatchExtraction()) {
				DisplayMessage(RMEEngine::LastError);
				UE_LOG(RMELog, Error, TEXT(" %s "), *RMEEngine::LastError);
			}
			break;
		}
	}
	else {
		RMEEngine::CancelOperation();
		FRootMotionExtractorGUI::DisplayMessage(FString::Printf(TEXT("Operation Canceled.")));
		UE_LOG(RMELog, Warning, TEXT("Operation Canceled."));
		return FReply::Handled();
	}
	return FReply::Handled();
}

FText FRootMotionExtractorGUI::GetSourceDir() const
{
	return FText::FromString(RMEEngine::SourceDir);
}

FText FRootMotionExtractorGUI::GetTargetDir() const
{
	return FText::FromString(RMEEngine::TargetDir);
}

FText FRootMotionExtractorGUI::GetExtractButtonCaption() const
{
	if (RMEEngine::bOperationStarted) {
		return FText::FromString(TEXT("Cancel"));
	}
	else {
		return FText::FromString(TEXT("Extract"));
	}
}

FReply FRootMotionExtractorGUI::SourceDirButton()
{
	WindowContainer->Minimize();
	TSharedPtr<SDlgPickPath> PickAssetPathWidget = SNew(SDlgPickPath).Title(FText::FromString("Select source directory"));

	if (EAppReturnType::Ok == PickAssetPathWidget->ShowModal())
	{
		RMEEngine::SourceDir = PickAssetPathWidget->GetPath().ToString();
	}
	WindowContainer->BringToFront();
	return FReply::Handled();
}
FReply FRootMotionExtractorGUI::TargetDirButton()
{
	WindowContainer->Minimize();
	TSharedPtr<SDlgPickPath> PickAssetPathWidget = SNew(SDlgPickPath).Title(FText::FromString("Select target directory"));

	if (EAppReturnType::Ok == PickAssetPathWidget->ShowModal())
	{
		RMEEngine::TargetDir = PickAssetPathWidget->GetPath().ToString();
	}
	WindowContainer->BringToFront();
	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE