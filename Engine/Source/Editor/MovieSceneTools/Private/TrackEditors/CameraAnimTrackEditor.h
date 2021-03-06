// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "Templates/SubclassOf.h"
#include "MovieSceneTrack.h"
#include "Widgets/SWidget.h"
#include "ISequencerSection.h"
#include "ISequencer.h"
#include "ISequencerTrackEditor.h"
#include "MovieSceneTrackEditor.h"

struct FAssetData;
class FMenuBuilder;
class UCameraAnim;

/**
 * Tools for playing a camera anim
 */
class FCameraAnimTrackEditor : public FMovieSceneTrackEditor
{
public:

	/**
	 * Constructor
	 *
	 * @param InSequencer The sequencer instance to be used by this tool
	 */
	FCameraAnimTrackEditor(TSharedRef<ISequencer> InSequencer);

	/** Virtual destructor. */
	virtual ~FCameraAnimTrackEditor() { }

	/**
	 * Creates an instance of this class.  Called by a sequencer 
	 *
	 * @param OwningSequencer The sequencer instance to be used by this tool
	 * @return The new instance of this class
	 */
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor( TSharedRef<ISequencer> OwningSequencer );

public:

	static FKeyPropertyResult AddCameraAnimKey(FMovieSceneTrackEditor& TrackEditor, FFrameNumber KeyTime, const TArray<TWeakObjectPtr<UObject>> Objects, UCameraAnim* CameraAnim);

	// ISequencerTrackEditor interface
	virtual void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass) override;
	virtual bool HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid) override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;
	virtual TSharedPtr<SWidget> BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params) override;

private:

	/** Delegate for AnimatablePropertyChanged in AddKey */
	FKeyPropertyResult AddKeyInternal(FFrameNumber KeyTime, const TArray<TWeakObjectPtr<UObject>> Objects, UCameraAnim* CameraAnim);

	/** Animation sub menu */
	TSharedRef<SWidget> BuildCameraAnimSubMenu(FGuid ObjectBinding);
	void AddCameraAnimSubMenu(FMenuBuilder& MenuBuilder, TArray<FGuid> ObjectBindings);

	/** Animation asset selected */
	void OnCameraAnimAssetSelected(const FAssetData& AssetData, TArray<FGuid> ObjectBindings);

	/** Animation asset enter pressed */
	void OnCameraAnimAssetEnterPressed(const TArray<FAssetData>& AssetData, TArray<FGuid> ObjectBindings);

	class UCameraComponent* AcquireCameraComponentFromObjectGuid(const FGuid& Guid);
};


