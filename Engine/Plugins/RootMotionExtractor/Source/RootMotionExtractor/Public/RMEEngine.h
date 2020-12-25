#pragma once
#include "CoreMinimal.h"
#include "RMEData.h"
#include "RMEUtilities.h"

namespace RMEEngine /*that could =)) */ {

	TSharedPtr<FCaptureType>ExtractionType;
	FAssetData SourceFile = NULL;
	FAssetData TargetFile = NULL;
	FString SourceDir = "";
	FString TargetDir = "";
	bool bIncrementalMovement = false;
	bool bIncrementalRotation = false;
	bool bDeleteAllCurveData = false;
	bool bCreateTargetCopy = false; //if target file missing
	float RateScale = 1.f;
	TArray <TSharedPtr <FBoneTask>> BoneTasks;


	//Thread System Status
	FGraphEventArray RunningTasks;
	TQueue<FRMEJob, EQueueMode::Mpsc> ExtractJobs;
	FString LastError = "";

	int32 FoundSourceFiles = 0;
	int32 ProcessedFiles = 0;
	int32 CompletedFiles = 0;

	bool bOperationStarted = false;

	//Thread System Config
	int32 MaxConcurentTasks = 1;
	int32 ExtractionsLaunchedPerFrame = 1;

	void CancelOperation() {
		RunningTasks.Empty();
		bOperationStarted = false;
		LastError = "Operation Canceled.";
	}

	bool IsRunning()
	{
		for (int32 Index = 0; Index < RunningTasks.Num(); Index++)
		{
			if (!RunningTasks[Index]->IsComplete())
			{
				return true;
			}
		}
		return false;
	}

	int32 GetRunningTasksCount()
	{
		int32 iRunningTasks = 0;
		for (int32 Index = 0; Index < RunningTasks.Num(); Index++)
		{
			if (!RunningTasks[Index]->IsComplete())
			{
				iRunningTasks++;
			}
		}
		return iRunningTasks;
	}


	int32 GetSelectedChannelsCount() {
		if (BoneTasks.Num() > 0) {
			int32 ChannelsCount = 0;
			for (auto& BoneTask : BoneTasks) {
				ChannelsCount = ChannelsCount + BoneTask.Get()->BoneChannels.Num();
			}
			return ChannelsCount;
		}
		else {
			return 0;
		}
	}

	bool IsBoneSelected(FName BoneName, TSharedPtr <FBoneTask>*& FoundBoneEntry) {

		FoundBoneEntry = BoneTasks.FindByPredicate([BoneName](TSharedPtr <FBoneTask>& InItem)
		{
			return InItem.Get()->BoneName == BoneName;
		});
		if (FoundBoneEntry) {
			return true;
		}
		return false;
	}

	bool IsBoneChannelSelected(FName BoneName, ECaptureChannel CaptureChannel, TSharedPtr <FBoneTask>*& FoundBoneEntry, FCurveElem*& FoundChannel) {

		FoundBoneEntry = BoneTasks.FindByPredicate([BoneName](TSharedPtr <FBoneTask>& InItem)
		{
			return InItem.Get()->BoneName == BoneName;
		});

		if (FoundBoneEntry) {
			FoundChannel = FoundBoneEntry->Get()->BoneChannels.FindByPredicate([CaptureChannel](FCurveElem& InItem)
			{
				return InItem.CaptureChannel == CaptureChannel;
			});
			if (FoundChannel) {
				return true;
			}
		}
		return false;
	}

	bool EnqueueSingleExtraction(const FAssetData& aSourceFile, const FAssetData& aTargetFile, bool bTagStart = false) {
		FRMEJob ExtractionJob = FRMEJob();

		if (aSourceFile == NULL) {
			LastError = "No Source file selected.";
			if (!bTagStart)
				ProcessedFiles++;
			return false;
		}
		else {
			ExtractionJob.SourceType = RMEUtilities::GetAssetType(aSourceFile);
			if ((ExtractionJob.SourceType == EAssetType::AT_Invalid) || (ExtractionJob.SourceType == EAssetType::AT_CurveFloat) || (ExtractionJob.SourceType == EAssetType::AT_Montage) /*Remove to enable source montages*/) {
				LastError = "Source file is not a supported Animation.";
				if (!bTagStart)
					ProcessedFiles++;
				return false;
			}
		}

		if (GetSelectedChannelsCount() <= 0) {
			LastError = "Nothing selected for capture.";
			if (!bTagStart)
				ProcessedFiles++;
			return false;
		}

		if (aTargetFile != NULL) {
			ExtractionJob.TargetType = RMEUtilities::GetAssetType(aTargetFile);
			if ((ExtractionJob.TargetType == EAssetType::AT_Invalid) || (ExtractionJob.TargetType == EAssetType::AT_Montage) /*Remove to enable target montages*/) {
				LastError = "Target file is invalid.";
				if (!bTagStart)
					ProcessedFiles++;
				return false;
			}
		}
		else {
			ExtractionJob.SourceType = RMEUtilities::GetAssetType(aSourceFile);
			if (!bCreateTargetCopy) {
				LastError = "No Target file selected.";
				if (!bTagStart)
					ProcessedFiles++;
				return false;
			}
			else {
				ExtractionJob.bNeedsTargetFile = true;
			}
		}
		ExtractionJob.Source = aSourceFile;
		ExtractionJob.Target = aTargetFile;
		if (bTagStart) {
			FoundSourceFiles = 1;
			ProcessedFiles = 0;
			CompletedFiles = 0;
			if (ExtractionJob.TargetType == EAssetType::AT_CurveFloat && GetSelectedChannelsCount() > 1) {
				LastError = "Selected target is a Float Curve. You can't capture more than one channel.";
				ProcessedFiles++;
				return false;
			}
			bOperationStarted = true;
		}
		ExtractJobs.Enqueue(ExtractionJob);
		return true;
	}

	bool StartBatchExtraction() {

		FoundSourceFiles = 0;
		ProcessedFiles = 0;
		CompletedFiles = 0;
		if (SourceDir.IsEmpty() || FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*SourceDir)) {
			LastError = "No Source directory selected or invalid.";
			return false;
		}

		if (TargetDir.IsEmpty() || FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*TargetDir)) {
			LastError = "No Target directory selected or invalid.";
			return false;
		}

		if (GetSelectedChannelsCount() <= 0) {
			LastError = "Nothing selected for capture.";
			return false;
		}

		TArray <FAssetData> SourceFiles = RMEUtilities::GetFilesInFolder(SourceDir, UAnimSequenceBase::StaticClass()->GetFName(), true);

		if (SourceFiles.Num() <= 0) {
			LastError = "There are no files in Source directory.";
			return false;
		}

		bOperationStarted = true;
		FoundSourceFiles = SourceFiles.Num();
		TArray <FAssetData> TargetFiles = RMEUtilities::GetFilesInFolder(TargetDir, UAnimSequenceBase::StaticClass()->GetFName(), true);
		TargetFiles.Append(RMEUtilities::GetFilesInFolder(TargetDir, UCurveFloat::StaticClass()->GetFName(), true));

		for (auto& SF : SourceFiles) {

			if (!bOperationStarted) {
				break;
			}

			const int32 FoundEntry = TargetFiles.IndexOfByPredicate([&SF](const FAssetData& InItem)
			{
				return InItem.AssetName == SF.AssetName;
			});
			if (FoundEntry != INDEX_NONE)
			{
				EnqueueSingleExtraction(SF, TargetFiles[FoundEntry]);
			}
			else {
				EnqueueSingleExtraction(SF, NULL);
			}
		}
		return true;
	}


	void AddBoneChannel(FName BoneName, FAssetData SkeletonAsset, ECaptureChannel CaptureChannel, bool& bNeedListRefresh) {

		FCurveElem CurveElement;
		TSharedPtr <FBoneTask>* FoundBoneEntry;
		FCurveElem* FoundChannel;
		if (IsBoneSelected(BoneName, FoundBoneEntry)) {
			if (!IsBoneChannelSelected(BoneName, CaptureChannel, FoundBoneEntry, FoundChannel)) {
				CurveElement.CaptureChannel = CaptureChannel;
				FoundBoneEntry->Get()->SkeletonFile = SkeletonAsset;
				CurveElement.Curve.DisplayName = RMEUtilities::GenerateCurveDisplayName(BoneName, CaptureChannel);
				CurveElement.Curve.UID = RMEUtilities::GenerateUID(4, BoneTasks);
				FoundBoneEntry->Get()->BoneChannels.Add(CurveElement);
			}
		}
		else {
			TSharedPtr <FBoneTask> NewBoneTask = MakeShareable(new FBoneTask);
			NewBoneTask.Get()->BoneName = BoneName;
			NewBoneTask.Get()->SkeletonFile = SkeletonAsset;
			CurveElement.CaptureChannel = CaptureChannel;
			CurveElement.Curve.DisplayName = RMEUtilities::GenerateCurveDisplayName(BoneName, CaptureChannel);
			CurveElement.Curve.UID = RMEUtilities::GenerateUID(4, BoneTasks);
			NewBoneTask.Get()->BoneChannels.Add(CurveElement);
			BoneTasks.Add(NewBoneTask);
			bNeedListRefresh = true;
		}
	}

	bool RemoveBoneChannel(FName BoneName, ECaptureChannel CaptureChannel, bool& bNeedListRefresh) {

		auto FoundBoneEntry = BoneTasks.IndexOfByPredicate([BoneName](const TSharedPtr <FBoneTask>& InItem)
		{
			return InItem.Get()->BoneName == BoneName;
		});
		if (FoundBoneEntry != INDEX_NONE) {
			auto FoundChannel = BoneTasks[FoundBoneEntry].Get()->BoneChannels.IndexOfByPredicate([CaptureChannel](const FCurveElem& InItem)
			{
				return InItem.CaptureChannel == CaptureChannel;
			});
			if (FoundChannel != INDEX_NONE) {
				BoneTasks[FoundBoneEntry].Get()->BoneChannels.RemoveAt(FoundChannel, 1, true);
				if (BoneTasks[FoundBoneEntry].Get()->BoneChannels.Num() <= 0) {
					BoneTasks.RemoveAt(FoundBoneEntry, 1, true);
					bNeedListRefresh = true;
				}
				return true;
			}
		}
		return false;
	}

	/// <summary>
	/// 
	/// </summary>
	class FRMEExtractor
	{
		FRMEJob WorkJob;
	public:
		FRMEExtractor(const FRMEJob& aExtractJob) {
			this->WorkJob = aExtractJob;
		}
		static bool TryStartJob()
		{
			if (GetRunningTasksCount() < MaxConcurentTasks) {
				FRMEJob LocalWorkJob;
				if (ExtractJobs.Dequeue(LocalWorkJob))
				{
					RunningTasks.Add(TGraphTask<FRMEExtractor>::CreateTask(NULL, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(LocalWorkJob));
					return true;
				}
			}
			return false;
		}

		FORCEINLINE static TStatId GetStatId()
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FVoxelTask, STATGROUP_TaskGraphTasks);
		}

		static ENamedThreads::Type GetDesiredThread()
		{
			return ENamedThreads::AnyThread;
		}

		static ESubsequentsMode::Type GetSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
		{
			ReadData();

			if (WorkJob.Data.Num() > 0)
				WriteData();
		}

	private:
		TMap<TSharedPtr<FBoneTask>, FTransform> BoneTracker;


		void ReadData() {
			UAnimSequence* SourceAnimation;
			UAnimMontage* SourceMontage;
			FAssetData Source = WorkJob.Source;
			FGraphEventRef InitRes = FFunctionGraphTask::CreateAndDispatchWhenReady([this, &SourceAnimation, &SourceMontage]() {
				switch (WorkJob.SourceType) {
				case EAssetType::AT_Animation:
					SourceAnimation = Cast<UAnimSequence>(WorkJob.Source.GetAsset());
					break;
				case EAssetType::AT_Montage:
					SourceMontage = Cast<UAnimMontage>(WorkJob.Source.GetAsset());
					break;
				}
			}, TStatId(), NULL, ENamedThreads::GameThread);
			FTaskGraphInterface::Get().WaitUntilTaskCompletes(InitRes);

			switch (WorkJob.SourceType) {
			case EAssetType::AT_Animation:
				if (!SourceAnimation) {
					return;
				}
				IterateAnimSequence(SourceAnimation);
				break;
			case EAssetType::AT_Montage:
				if (!SourceMontage) {
					return;
				}
				break;
			}
		}


		void WriteData()
		{
			UAnimSequence* TAnim = nullptr;
			UCurveFloat* TCurve = nullptr;
			UAnimMontage* TMont = nullptr;
			TArray<FCurveElem> Curves;

			FGraphEventRef InitRes = FFunctionGraphTask::CreateAndDispatchWhenReady([this, &TAnim, &TCurve, &TMont]() {
				switch (WorkJob.TargetType) {
				case EAssetType::AT_Animation:
					TAnim = Cast<UAnimSequence>(WorkJob.Target.GetAsset());
					break;
				case EAssetType::AT_CurveFloat:
					TCurve = Cast<UCurveFloat>(this->WorkJob.Target.GetAsset());
					break;
				case EAssetType::AT_Montage:
					TMont = Cast<UAnimMontage>(WorkJob.Target.GetAsset());
					break;
				case EAssetType::AT_Invalid:
					if (WorkJob.bNeedsTargetFile) {
						FString PackagePath;
						if (ExtractionType->CaptureType == ECaptureType::CT_Bulk)
						{
							PackagePath = TargetDir + "/";
						}
						else {
							TArray<FString> AssetPath;
							PackagePath = WorkJob.Source.ObjectPath.ToString();
							WorkJob.Source.ObjectPath.ToString().ParseIntoArray(AssetPath, *FString("/"), true);
							PackagePath.RemoveFromEnd(AssetPath[AssetPath.Num() - 1]);
							PackagePath = PackagePath;
							UE_LOG(LogTemp, Warning, TEXT("%s"), *PackagePath);
						}

						TArray<FString> outname;
						WorkJob.Source.ObjectPath.ToString().ParseIntoArray(outname, *FString("."), true);
						FString ShortName = outname[outname.Num() - 1] + "_IP";
						PackagePath += ShortName;
						UPackage* Package = CreatePackage(nullptr, *PackagePath);
						switch (WorkJob.SourceType)
						{
						case EAssetType::AT_Animation:
							TAnim = NewObject<UAnimSequence>(Package, WorkJob.Source.GetClass(), *ShortName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
							if (TAnim) {
								TAnim = Cast<UAnimSequence>(DuplicateObject(WorkJob.Source.GetAsset(), Package, FName(*ShortName)));
								TAnim->bEnableRootMotion = false;
								TAnim->bForceRootLock = true;
								FAssetRegistryModule::AssetCreated(TAnim);
								Package->FullyLoad();
								Package->MarkPackageDirty();
								WorkJob.TargetType = EAssetType::AT_Animation;
							}
							break;
						case EAssetType::AT_Montage:
							TMont = NewObject<UAnimMontage>(Package, WorkJob.Source.GetClass(), *ShortName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
							if (TMont) {
								TMont = Cast<UAnimMontage>(DuplicateObject(WorkJob.Source.GetAsset(), Package, FName(*ShortName)));
								FAssetRegistryModule::AssetCreated(TAnim);
								Package->FullyLoad();
								Package->MarkPackageDirty();
								WorkJob.TargetType = EAssetType::AT_Montage;
							}
							break;
						}
					}
					break;
				}
			}, TStatId(), NULL, ENamedThreads::GameThread);
			FTaskGraphInterface::Get().WaitUntilTaskCompletes(InitRes);


			switch (WorkJob.TargetType) {
			case EAssetType::AT_Animation:
				if (!TAnim) {
					return;
				}
				WriteDataAnim(TAnim);
				break;
			case EAssetType::AT_CurveFloat:
				if (!TCurve) {
					return;
				}
				WorkJob.Data.GenerateKeyArray(Curves);
				WriteDataFloatCurve(TCurve, WorkJob.Data[Curves[0]]);
				break;
			case EAssetType::AT_Montage:
				if (!TMont) {
					return;
				}
				break;
			}

			auto GameTask = FFunctionGraphTask::CreateAndDispatchWhenReady([]()
			{
				ProcessedFiles++;
			}, TStatId(), NULL, ENamedThreads::GameThread);
		}


		void IterateAnimSequence(UAnimSequence* SourceAnimation) {
			float SourceLength = 0.f;
			int32 NumberofFrames = 0;
#if ENGINE_MINOR_VERSION < 20
			SourceLength = SourceAnimation->GetPlayLength();
			NumberofFrames = SourceAnimation->GetNumberOfFrames();
#else
			UAnimationBlueprintLibrary::GetSequenceLength(SourceAnimation, SourceLength);
			UAnimationBlueprintLibrary::GetNumFrames(SourceAnimation, NumberofFrames);
#endif

			const float Delta = (SourceLength / NumberofFrames);

			float LastCheckedTime = 0.f;
			for (int x = 1; x <= NumberofFrames; ++x)
			{

				if (!SourceAnimation)
				{
					return;
				}
				float CurrentTime = x * Delta;

				for (auto& BoneTask : BoneTasks)
				{
					FTransform LastTransform;
					if (BoneTracker.Contains(BoneTask))
					{
						LastTransform = BoneTracker[BoneTask];
					}
					else {
						LastTransform = BoneTracker.Add(BoneTask, FTransform());
					}
					if (!RMEUtilities::GetBoneTransform(SourceAnimation, BoneTask.Get()->BoneName, CurrentTime, BoneTracker[BoneTask]))
					{
						continue;
					}
					for (auto& Channel : BoneTask.Get()->BoneChannels)
					{
						ProcessChannel(BoneTask, Channel, CurrentTime - LastCheckedTime, CurrentTime, LastTransform);
					}
				}
				LastCheckedTime = CurrentTime;
			}
		}


		void ProcessChannel(const TSharedPtr<FBoneTask>& BoneTask, const FCurveElem& Channel, const float& Delta, const float& CurrentTime, FTransform& CurrentTransform) {
			FDataStruct Item;

			auto& Curve = WorkJob.Data.FindOrAdd(Channel);
			Item.KeyTime = CurrentTime;
			switch (Channel.CaptureChannel) {
			case ECaptureChannel::CC_MovementSpeed:
				Item.KeyValue = (((BoneTracker[BoneTask].GetTranslation() - CurrentTransform.GetTranslation()).Size()) / Delta) * RateScale;
				Curve.Add(Item);
				break;
			case ECaptureChannel::CC_Right:
				if (bIncrementalMovement) {
					Item.KeyValue = (BoneTracker[BoneTask].GetTranslation().X * -1) * RateScale;
				}
				else {
					Item.KeyValue = ((BoneTracker[BoneTask].GetTranslation().X - CurrentTransform.GetTranslation().X) * -1) * RateScale;
				}
				Curve.Add(Item);
				break;
			case ECaptureChannel::CC_Forward:
				if (bIncrementalMovement) {
					Item.KeyValue = (BoneTracker[BoneTask].GetTranslation().Y) * RateScale;
				}
				else {
					Item.KeyValue = (BoneTracker[BoneTask].GetTranslation().Y - CurrentTransform.GetTranslation().Y) * RateScale;
				}
				Curve.Add(Item);
				break;
			case ECaptureChannel::CC_Up:
				if (bIncrementalMovement) {
					Item.KeyValue = (BoneTracker[BoneTask].GetTranslation().Z) * RateScale;
				}
				else {
					Item.KeyValue = (BoneTracker[BoneTask].GetTranslation().Z - CurrentTransform.GetTranslation().Z) * RateScale;
				}
				Curve.Add(Item);
				break;
			case ECaptureChannel::CC_Roll:
				if (bIncrementalRotation) {
					Item.KeyValue = (BoneTracker[BoneTask].GetRotation().GetNormalized().Euler().X) * RateScale;
				}
				else {
					Item.KeyValue = (BoneTracker[BoneTask].GetRotation().GetNormalized().Euler().X - CurrentTransform.GetRotation().GetNormalized().Euler().X) * RateScale;
				}
				Curve.Add(Item);
				break;
			case ECaptureChannel::CC_Pitch:
				if (bIncrementalRotation) {
					Item.KeyValue = (BoneTracker[BoneTask].GetRotation().GetNormalized().Euler().Y) * RateScale;
				}
				else {
					Item.KeyValue = (BoneTracker[BoneTask].GetRotation().GetNormalized().Euler().Y - CurrentTransform.GetRotation().GetNormalized().Euler().Y) * RateScale;
				}
				Curve.Add(Item);
				break;
			case ECaptureChannel::CC_Yaw:
				if (bIncrementalRotation) {
					Item.KeyValue = (BoneTracker[BoneTask].GetRotation().GetNormalized().Euler().Z) * RateScale;
				}
				else {
					Item.KeyValue = (BoneTracker[BoneTask].GetRotation().GetNormalized().Euler().Z - CurrentTransform.GetRotation().GetNormalized().Euler().Z) * RateScale;
				}
				Curve.Add(Item);
				break;
			}
		}

		void WriteDataFloatCurve(UCurveFloat* TCurve, const TArray<FDataStruct>& Data) {
			if (TCurve) {
				FGraphEventRef AsyncTask = FFunctionGraphTask::CreateAndDispatchWhenReady([TCurve, &Data]()
				{
					if (TCurve->CanModify()) {
						TCurve->Modify(true); //modify
						UCurveBase* BaseCurve = TCurve;
#if ENGINE_MINOR_VERSION < 20
						TCurve->FloatCurve.Reset();
#else
						auto FloatCurves = BaseCurve->GetCurves();
						FloatCurves[0].CurveToEdit->Reset(); //clear all data from float curve
#endif

						for (auto& x : Data) {
#if ENGINE_MINOR_VERSION < 20
							TCurve->FloatCurve.AddKey(x.KeyTime, x.KeyValue);
#else
							auto FloatCurvesBase = BaseCurve->GetCurves();
							FloatCurvesBase[0].CurveToEdit->AddKey(x.KeyTime, x.KeyValue);
#endif
						}
						TCurve->MarkPackageDirty();
					}
					CompletedFiles++;
				}, TStatId(), NULL, ENamedThreads::GameThread);
				FTaskGraphInterface::Get().WaitUntilTaskCompletes(AsyncTask);
			}
		}

		void WriteDataAnim(UAnimSequence* TAnim) {

			if (TAnim) {
				FGraphEventRef AsyncTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this, TAnim]()
				{
					if (TAnim->CanModify()) {
						TAnim->Modify(true);
						if (bDeleteAllCurveData) {
#if ENGINE_MINOR_VERSION < 20
							TAnim->RawCurveData.Empty();
#else
							UAnimationBlueprintLibrary::RemoveAllCurveData(TAnim);
#endif
							TAnim->RefreshCurveData();
						}
						else {
							if (!ClearAnimCurves(TAnim)) {
								return;
							}
						}
						for (auto& CurveElement : WorkJob.Data) {
							for (auto& x : CurveElement.Value) {
#if ENGINE_MINOR_VERSION < 20
								TAnim->RawCurveData.AddFloatCurveKey(CurveElement.Key.Curve, 4, x.KeyTime, x.KeyValue);
#else
								if (!UAnimationBlueprintLibrary::DoesCurveExist(TAnim, CurveElement.Key.Curve.DisplayName, ERawCurveTrackTypes::RCT_Float))
									UAnimationBlueprintLibrary::AddCurve(TAnim, CurveElement.Key.Curve.DisplayName, ERawCurveTrackTypes::RCT_Float);
								UAnimationBlueprintLibrary::AddFloatCurveKey(TAnim, CurveElement.Key.Curve.DisplayName, x.KeyTime, x.KeyValue);
#endif
							}
						}
						TAnim->RefreshCurveData();
						TAnim->MarkRawDataAsModified();
						TAnim->MarkPackageDirty(); //mark target as modified.
						CompletedFiles++;
					}
				}, TStatId(), NULL, ENamedThreads::GameThread);
				FTaskGraphInterface::Get().WaitUntilTaskCompletes(AsyncTask);
			}
		}

		bool ClearAnimCurves(UAnimSequence* Target)
		{
			USkeleton* Skeleton = Target->GetSkeleton();
			if (Skeleton) {
				const FSmartNameMapping* NameMapping = Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
				FSmartName CurveUid = FSmartName();
				for (auto& CurveElement : WorkJob.Data) {
					if (NameMapping->FindSmartName(CurveElement.Key.Curve.DisplayName, CurveUid)) {
						CurveElement.Key.Curve = CurveUid;
#if ENGINE_MINOR_VERSION < 20
						Target->RawCurveData.DeleteCurveData(CurveUid, ERawCurveTrackTypes::RCT_Float);
#else
						UAnimationBlueprintLibrary::RemoveCurve(Target, CurveUid.DisplayName);
#endif
						Target->RefreshCurveData();
					}
				}
				return true;
			}
			return false;
		}

	};
};