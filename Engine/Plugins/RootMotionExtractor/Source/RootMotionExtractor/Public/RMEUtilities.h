#pragma once
#include "CoreMinimal.h"
#include "RMEData.h"
#if ENGINE_MINOR_VERSION >= 20
#include "AnimationModifiers/Public/AnimationModifier.h"
#endif
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSequence.h"

namespace RMEUtilities {

	TArray <FAssetData> GetFilesInFolder(FString Dir, FName ByClass, bool bSearchSubClasses) {
		TArray <FAssetData> AssetData;
		TArray <FAssetData> FilesInFolder;
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().GetAssetsByClass(ByClass, AssetData, bSearchSubClasses);
		FString Path = Dir + "/";
		for (auto &Asset : AssetData) {
			FString testString = Asset.GetFullName();
			testString.RemoveFromStart(Asset.AssetClass.ToString() + " ");
			if (testString.StartsWith(Path)) {
				FilesInFolder.Add(Asset);
			}
		}
		return FilesInFolder;
	}

	int32 GenerateUID(int32 Length, const TArray <TSharedPtr <FBoneTask>> &BoneTasks) {
		FString GeneratedUID = "";
		if (Length > 0)
		{
			for (int32 x = 0; x < Length; x++)
			{
				GeneratedUID += FString::FromInt(FMath::RandRange(0, 9));
			}
			for (auto &BoneTask : BoneTasks) {
				for (auto &Channel : BoneTask.Get()->BoneChannels) 
				{
					if (Channel.Curve.UID == FCString::Atoi(*GeneratedUID)) 
					{
						return GenerateUID(Length, BoneTasks);
					}
				}
			}
			return FCString::Atoi(*GeneratedUID);
		}
		else {
			return 0;
		}
	}

	bool GetSkeleton(UAnimSequence* TSource, USkeleton* &AnimSkeleton) {
		AnimSkeleton = TSource->GetSkeleton();
		if (AnimSkeleton) 
		{
			return true;
		}
		else {
			return false;
		}
	}
	bool GetSkeleton(UAnimMontage* TSource, USkeleton* &AnimSkeleton) {
		AnimSkeleton = TSource->GetSkeleton();
		if (AnimSkeleton) {
			return true;
		}
		else {
			return false;
		}
	}

	bool GetBoneIndex(USkeleton* AnimSkeleton, FName BoneName, int32 &BoneIndex) {
		if (AnimSkeleton) 
		{
			BoneIndex = AnimSkeleton->GetReferenceSkeleton().FindBoneIndex(BoneName);
			if (BoneIndex != INDEX_NONE)
			{
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}

	EAssetType GetAssetType(FAssetData AssetData) 
	{
		if (AssetData.GetClass()->IsChildOf(UCurveFloat::StaticClass())) 
		{
			return EAssetType::AT_CurveFloat;
		}
		else if (AssetData.GetClass() == UCurveFloat::StaticClass()) 
		{
			return EAssetType::AT_CurveFloat;
		}
		else if (AssetData.GetClass()->IsChildOf(UAnimSequence::StaticClass()))
		{
			return EAssetType::AT_Animation;
		}
		else if (AssetData.GetClass() == UAnimSequence::StaticClass()) 
		{
			return EAssetType::AT_Animation;
		}
		else if (AssetData.GetClass()->IsChildOf(UAnimMontage::StaticClass()))
		{
			return EAssetType::AT_Montage;
		}
		else if (AssetData.GetClass() == UAnimMontage::StaticClass()) 
		{
			return EAssetType::AT_Montage;
		}
		else {
			return EAssetType::AT_Invalid;
		}
	}

	bool GetBoneTransform(UAnimSequence* InAnimation, FName BoneName, float Time, FTransform &BoneTransform) 
	{

		int32 BoneIndex;
		USkeleton *Skeleton;
		FTransform Result = FTransform();
		if (InAnimation != NULL)
		{

			if (GetSkeleton(InAnimation, Skeleton)) 
			{
				if (BoneName != FName(TEXT("Root Motion"))) 
				{
					if (!GetBoneIndex(Skeleton, BoneName, BoneIndex)) 
					{
						return false;
					}
				}
				else {
					BoneIndex = 0;
				}
				if (BoneIndex > 0) 
				{
					while (BoneIndex > 0)
					{
						FTransform OutTransform = FTransform();
						FTransform OutParentTransform = FTransform();
						int32 TrackIndex, ParentTrackIndex = 0;

#if ENGINE_MINOR_VERSION > 22
						TrackIndex = Skeleton->GetRawAnimationTrackIndex(BoneIndex, InAnimation);
#else
						TrackIndex = Skeleton->GetAnimationTrackIndex(BoneIndex, InAnimation, true);
#endif
						InAnimation->GetBoneTransform(OutTransform, TrackIndex, Time, true);
						FVector NewResultLocation = Result.GetLocation() + OutTransform.GetLocation();
						BoneIndex = Skeleton->GetReferenceSkeleton().GetParentIndex(BoneIndex);

#if ENGINE_MINOR_VERSION > 22
						ParentTrackIndex = Skeleton->GetRawAnimationTrackIndex(BoneIndex, InAnimation);
#else
						ParentTrackIndex = Skeleton->GetAnimationTrackIndex(BoneIndex, InAnimation, true);
#endif
						InAnimation->GetBoneTransform(OutParentTransform, ParentTrackIndex, Time, true);
						NewResultLocation = OutParentTransform.Rotator().RotateVector(NewResultLocation);
						Result.SetLocation(NewResultLocation);
						Result.SetRotation(OutTransform.GetRotation() * Result.GetRotation());
					}
				}
				else {
					if (InAnimation->HasRootMotion()) 
					{
						Result = InAnimation->ExtractRootMotionFromRange(0.0f, Time);
					}
					else {
						return false;
					}
				}
				BoneTransform = Result;
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}

	FName GenerateCurveDisplayName(FName BoneName, ECaptureChannel CaptureChannel) 
	{
		FString DisplayName = "";

		if (BoneName != FName(TEXT("Root Motion"))) 
		{
			DisplayName = BoneName.ToString();
			DisplayName += "_";
		}
		switch (CaptureChannel) {
		case ECaptureChannel::CC_MovementSpeed:
			//Channel is MovementSpeed
			DisplayName += "MovementSpeed";
			break;
		case ECaptureChannel::CC_Right:
			//Channel is Right Movement
			DisplayName += "RightMovement";
			break;
		case ECaptureChannel::CC_Forward:
			//Channel is Forward Movement
			DisplayName += "ForwardMovement";
			break;
		case ECaptureChannel::CC_Up:
			//Channel is Up Movement
			DisplayName += "UpMovement";
			break;
		case ECaptureChannel::CC_Yaw:
			//Channel is Yaw
			DisplayName += "YawRotation";
			break;
		case ECaptureChannel::CC_Pitch:
			//Channel is Pitch
			DisplayName += "PitchRotation";
			break;
		case ECaptureChannel::CC_Roll:
			//Channel is Roll
			DisplayName += "RollRotation";
			break;
		}
		return FName(*DisplayName);
	}
}