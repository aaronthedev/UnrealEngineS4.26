#pragma once
#include "CoreMinimal.h"
#include "AssetRegistryModule.h"
#include "Engine.h"

#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif

struct FDataStruct {
	float KeyTime;
	float KeyValue;
};

enum class EAssetType : uint8
{
	AT_CurveFloat,
	AT_Animation,
	AT_Montage,
	AT_Invalid
};

enum class ECaptureType : uint8
{
	CT_Bulk,
	CT_Single
};

struct FCaptureType
{
	ECaptureType CaptureType;
	FString CaptureLabel;
};

enum class ECaptureChannel : uint8
{
	CC_MovementSpeed,
	CC_Right,
	CC_Forward,
	CC_Up,
	CC_Yaw,
	CC_Pitch,
	CC_Roll
};

struct FCurveElem {
	FSmartName Curve;
	ECaptureChannel CaptureChannel;

	bool operator==(const FCurveElem& CurveElement) const
	{
		return Curve == CurveElement.Curve  && CaptureChannel == CurveElement.CaptureChannel;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FCurveElem& CurveElement)
	{
		return FCrc::MemCrc32(&CurveElement, sizeof(FCurveElem));
	}

};

struct FBoneTask {
	FName BoneName;
	FAssetData SkeletonFile = NULL;
	TArray<FCurveElem> BoneChannels;
};

struct FSmartNameHash {
	uint32 operator()(FSmartName const & k) const {
		return FCrc::MemCrc32(&k, sizeof(FSmartName));
	}
};

struct FRMEJob {
	TMap<FCurveElem, TArray<FDataStruct>> Data;
	FAssetData Source = NULL;
	FAssetData Target = NULL;
	bool bNeedsTargetFile = false;
	EAssetType SourceType = EAssetType::AT_Invalid;
	EAssetType TargetType = EAssetType::AT_Invalid;
};


