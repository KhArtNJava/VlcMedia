// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#include "VlcTypes.h"


// library instance
typedef FLibvlcInstance* (*FLibvlcNewProc)(int32 /*Argc*/, const ANSICHAR* const* /*Argv*/);
typedef void (*FLibvlcReleaseProc)(FLibvlcInstance* /*Instance*/);
typedef void (*FLibvlcRetainProc)(FLibvlcInstance* /*Instance*/);

// error handling
typedef const char* (*FLibvlcErrmsgProc)();
typedef void (*FLibvlcClearerrProc)();

// events
typedef void (*FLibvlcCallback)(FLibvlcEvent* /*Event*/, void* /*UserData*/);
typedef int (*FLibvlcEventAttachProc)(FLibvlcEventManager* /*EventManager*/, ELibvlcEventType /*EventType*/, FLibvlcCallback /*Callback*/, void* /*UserData*/);
typedef int (*FLibvlcEventDetachProc)(FLibvlcEventManager* /*EventManager*/, ELibvlcEventType /*EventType*/, FLibvlcCallback /*Callback*/, void* /*UserData*/);
typedef const ANSICHAR* (*FLibvlcEventTypeNameProc)(ELibvlcEventType /*EventType*/);

// media callbacks
typedef void (*FLibvlcMediaCloseCb)(void* /*Opaque*/);
typedef int (*FLibvlcMediaOpenCb)(void* /*Opaque*/, void** /*OutData*/, uint64* /*OutSize*/);
typedef SSIZE_T (*FLibvlcMediaReadCb)(void* /*Opaque*/, void* /*Buffer*/, SIZE_T /*Length*/);
typedef int (*FLibvlcMediaSeekCb)(void* /*Opaque*/, uint64 /*Offset*/);

// media
typedef FLibvlcEventManager* (*FLibvlcMediaEventManagerProc)(FLibvlcMedia* /*Media*/);

typedef FLibvlcMedia* (*FLibvlcMediaNewCallbacksProc)(
	FLibvlcInstance* /*Instance*/,
	FLibvlcMediaOpenCb /*OpenCb*/,
	FLibvlcMediaReadCb /*ReadCb*/,
	FLibvlcMediaSeekCb /*SeekCb*/,
	FLibvlcMediaCloseCb /*CloseCb*/,
	void* /*Opaque*/);

typedef FLibvlcMedia* (*FLibvlcMediaNewLocationProc)(FLibvlcInstance* /*Instance*/, const ANSICHAR* /*Location*/);
typedef FLibvlcMedia* (*FLibvlcMediaNewPathProc)(FLibvlcInstance* /*Instance*/, const ANSICHAR* /*Path*/);
typedef void (*FLibvlcMediaParseAsyncProc)(FLibvlcMedia* /*Media*/);
typedef void (*FLibvlcMediaReleaseProc)(FLibvlcMedia* /*Media*/);
typedef void (*FLibvlcMediaRetainProc)(FLibvlcMedia* /*Media*/);
typedef uint32(*FLibvlcMediaTracksGetProc)(FLibvlcMedia* /*Media*/, FLibvlcMediaTrack*** /*OutTracks*/);
typedef void(*FLibvlcMediaTracksReleaseProc)(FLibvlcMediaTrack** /*Tracks*/, uint32 /*Count*/);

// media player
typedef FLibvlcEventManager* (*FLibvlcMediaPlayerEventManagerProc)(FLibvlcMediaPlayer* /*Player*/);
typedef FLibvlcMedia* (*FLibvlcMediaPlayerGetMediaProc)(FLibvlcMediaPlayer* /*Player*/);
typedef FLibvlcMediaPlayer* (*FLibvlcMediaPlayerNewProc)(FLibvlcInstance* /*Instance*/);
typedef FLibvlcMediaPlayer* (*FLibvlcMediaPlayerNewFromMediaProc)(FLibvlcMedia* /*Media*/);
typedef void (*FLibvlcMediaPlayerReleaseProc)(FLibvlcMediaPlayer* /*Player*/);
typedef void (*FLibvlcMediaPlayerRetainProc)(FLibvlcMediaPlayer* /*Player*/);
typedef void (*FLibvlcMediaPlayerSetMediaProc)(FLibvlcMediaPlayer* /*Player*/, FLibvlcMedia* /*Media*/);

// media player status
typedef float (*FLibvlcMediaPlayerGetFpsProc)(const FLibvlcMediaPlayer* /*Player*/);
typedef int64 (*FLibvlcMediaPlayerGetLengthProc)(const FLibvlcMediaPlayer* /*Player*/);
typedef float (*FLibvlcMediaPlayerGetPositionProc)(const FLibvlcMediaPlayer* /*Player*/);
typedef float (*FLibvlcMediaPlayerGetRateProc)(const FLibvlcMediaPlayer* /*Player*/);
typedef ELibvlcState (*FLibvlcMediaPlayerGetStateProc)(const FLibvlcMediaPlayer* /*Player*/);
typedef int64 (*FLibvlcMediaPlayerGetTimeProc)(const FLibvlcMediaPlayer* /*Player*/);
typedef int32 (*FLibvlcMediaPlayerIsSeekableProc)(const FLibvlcMediaPlayer* /*Player*/);
typedef void (*FLibvlcMediaPlayerSetPositionProc)(FLibvlcMediaPlayer* /*Player*/, float /*Position*/);
typedef int32 (*FLibvlcMediaPlayerSetRateProc)(FLibvlcMediaPlayer* /*Player*/, float /*Rate*/);
typedef void (*FLibvlcMediaPlayerSetTimeProc)(FLibvlcMediaPlayer* /*Player*/, int64 /*Time*/);

// media player control
typedef int32 (*FLibvlcMediaPlayerIsPlayingProc)(const FLibvlcMediaPlayer* /*Player*/);
typedef void (*FLibvlcMediaPlayerPauseProc)(FLibvlcMediaPlayer* /*Player*/);
typedef void (*FLibvlcMediaPlayerPlayProc)(FLibvlcMediaPlayer* /*Player*/);
typedef void (*FLibvlcMediaPlayerSetPauseProc)(FLibvlcMediaPlayer* /*Player*/, int32 /*DoPause*/);
typedef void (*FLibvlcMediaPlayerStopProc)(FLibvlcMediaPlayer* /*Player*/);
typedef int32 (*FLibvlcMediaPlayerWillPlayProc)(FLibvlcMediaPlayer* /*Player*/);

// audio
typedef int32 (*FLibvlcAudioGetTrackProc)(FLibvlcMediaPlayer* /*Player*/);
typedef int32 (*FLibvlcAudioSetTrackProc)(FLibvlcMediaPlayer* /*Player*/, int32 /*TrackId*/);

// video callbacks
typedef void* (*FLibvlcVideoLockCb)(void* /*Opaque*/, void** /*Planes*/);
typedef void (*FlibvlcVideoUnlockCb)(void* /*Opaque*/, void* /*Picture*/, void* const* /*Planes*/);
typedef void (*FlibvlcVideoDisplayCb)(void* /*Opaque*/, void* /*Picture*/);

// video
typedef void (*FLibvlcVideoSetCallbacksProc)(
	FLibvlcMediaPlayer* /*Player*/,
	FLibvlcVideoLockCb /*Lock*/,
	FlibvlcVideoUnlockCb /*Unlock*/,
	FlibvlcVideoDisplayCb /*Display*/,
	void* /*Opaque*/);

typedef void (*FLibvlcVideoSetFormatProc)(
	FLibvlcMediaPlayer* /*Player*/,
	const ANSICHAR* /*Chroma*/,
	uint32 /*Width*/,
	uint32 /*Height*/,
	uint32 /*Pitch*/);

typedef int32 (*FLibvlcVideoGetHeightProc)(FLibvlcMediaPlayer* /*Player*/);
typedef int32 (*FLibvlcVideoGetWidthProc)(FLibvlcMediaPlayer* /*Player*/);
typedef int32 (*FLibvlcVideoGetSpuProc)(FLibvlcMediaPlayer* /*Player*/);
typedef int32 (*FLibvlcVideoGetTrackProc)(FLibvlcMediaPlayer* /*Player*/);
typedef int32 (*FLibvlcVideoSetSpuProc)(FLibvlcMediaPlayer* /*Player*/, int32 /*SpuId*/);
typedef int32 (*FLibvlcVideoSetTrackProc)(FLibvlcMediaPlayer* /*Player*/, int32 /*TrackId*/);

// tracks
typedef FLibvlcTrackDescription* (*FLibvlcAudioGetTrackDescriptionProc)(FLibvlcMediaPlayer* /*Player*/);
typedef FLibvlcTrackDescription* (*FLibvlcVideoGetSpuDescriptionProc)(FLibvlcMediaPlayer* /*Player*/);
typedef FLibvlcTrackDescription* (*FLibvlcVideoGetTrackDescriptionProc)(FLibvlcMediaPlayer* /*Player*/);
typedef void (*FLibvlcTrackDescriptionListReleaseProc)(FLibvlcTrackDescription* /*Desription*/);
