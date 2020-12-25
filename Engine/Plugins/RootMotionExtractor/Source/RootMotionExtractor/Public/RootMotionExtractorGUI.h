#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "RMEData.h"
#include "RMEEngine.h"
#include "Runtime/SlateCore/Public/Widgets/SCompoundWidget.h"
#include "Runtime/Slate/Public/Widgets/Input/SNumericEntryBox.h"

DECLARE_LOG_CATEGORY_EXTERN(RMELog, Log, All);

class SFloatWidget : public SCompoundWidget
{
public:
	typedef float FFloatType;
	DECLARE_DELEGATE_OneParam(FFloatUpdated, FFloatType)

	SLATE_BEGIN_ARGS(SFloatWidget) : _Float(), _GetFloatUpdated()
	{}
	SLATE_ARGUMENT(float, Float)
		SLATE_EVENT(FFloatUpdated, GetFloatUpdated)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
	{
		Float = InArgs._Float;
		GetFloatUpdated = InArgs._GetFloatUpdated;
		ChildSlot
			[
				SNew(SNumericEntryBox<float>)
				.AllowSpin(false)
			.OnValueChanged(this, &SFloatWidget::ValueChanged)
			.Value(this, &SFloatWidget::GetValue)
			];
	}

	void ValueChanged(float Value) {
		Float = Value;
		GetFloatUpdated.ExecuteIfBound(Float);
	}

	TOptional<float> GetValue() const {
		return Float;
	}

private:
	FFloatUpdated GetFloatUpdated;
	float Float = 0.f;
};

class FRootMotionExtractorGUI
{
public:
	void ShowAddBoneWindow(const TSharedPtr <FBoneTask> BoneTask = NULL);
	void ShowMainWindow();
	void OnAddBoneWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed);
	void OnMainWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed);

	TSharedRef<SWidget> GenerateAddBoneWindow(const TSharedPtr <FBoneTask> BoneTask = NULL);
	TSharedRef<SWidget> GenerateWindow();

	void DisplayMessage(FString Message);
	TSharedPtr<class SWindow> WindowContainer;
	TSharedPtr<class SWindow> AddBoneWindowContainer;

	TSharedPtr< SListView< TSharedPtr<FBoneTask> > > BoneListView;
	TSharedPtr< SComboBox< TSharedPtr<FCaptureType> > > CaptureTypeOptions;
	TArray <TSharedPtr <FCaptureType>> CaptureTypes;

private:

	void SetCaptureChannel(ECheckBoxState newState, ECaptureChannel SelectedChannel, bool bIsRootMotion);
	ECheckBoxState GUIIsChannelSelected(ECaptureChannel CaptureChannel, bool bIsRootMotion) const;

	void UpdateRateScale(float Value);

	ECheckBoxState IsDeleteAllCurveData() const;
	ECheckBoxState IsIncrementRotation() const;
	ECheckBoxState IsAutoCreateTargetCopy() const;
	ECheckBoxState IsIncrementMovement() const;

	void DeleteAllCurveDataChanged(const ECheckBoxState newState);
	void IncrementRotationChanged(const ECheckBoxState newState);
	void AutoCreateTargetCopyChanged(const ECheckBoxState newState);
	void IncrementMovementChanged(const ECheckBoxState newState);
	UClass * OnGetClassesForSourcePicker();
	UClass * OnGetClassesForTargetPicker();
	UClass * OnGetClassesForSkelPicker();

	bool FilterSourcePicker(const FAssetData & AssetData) const;

	bool FilterTargetPicker(const FAssetData & AssetData) const;
	void OnSourceAnimAssetSelected(const FAssetData & AssetData);
	void OnSkelAssetSelected(const FAssetData & AssetData);
	void OnTargetAssetSelected(const FAssetData & AssetData);

	const FReferenceSkeleton & GetSkeletonRef();

	FString GetSourceAnimAssetName() const;
	FString GetTargetAssetName() const;
	FString GetSkelAssetName() const;
	TOptional<float> GetPercentage() const;
	FReply ExtractPressed();
	FText GetSourceDir() const;
	FText GetTargetDir() const;
	FText GetExtractButtonCaption() const;
	FReply SourceDirButton();
	FReply TargetDirButton();


	void OnBoneNameChanged(FName BoneName);
	void BoneEntrySelectionChanged(TSharedPtr<FBoneTask> InItem, ESelectInfo::Type SelectInfo);
	void BoneEntryDoubleClicked(TSharedPtr<FBoneTask> InItem);


	FReply AddBonePressed();
	FReply DeleteBonePressed();
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FBoneTask> Item, const TSharedRef<STableViewBase>& OwnerTable);
	FText GetBoneNameForList(TSharedPtr<FBoneTask> Item) const;
	FReply AddBoneWindowCloseButton();
	TSharedRef<SWidget> MakeWidgetForCaptureType(TSharedPtr<FCaptureType> InOption);

	void OnCaptureTypeChanged(TSharedPtr<FCaptureType> NewValue, ESelectInfo::Type);

	FText GetCurrentCaptureTypeLabel() const;


	TSharedPtr<FBoneTask> SelectedItem;
	FAssetData SkelFile = NULL;
	FName GUI_SelectedBoneName;
	const FReferenceSkeleton RefEmpty = FReferenceSkeleton();

private:
	TSharedPtr<class SWidgetSwitcher> WidgetSwitcher;
	TSharedPtr<class SBoneTreeMenu> MenuSkelWidget;
};