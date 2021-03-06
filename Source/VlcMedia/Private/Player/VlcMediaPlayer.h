// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#include "ModuleManager.h"


/**
 * Implements a media player using the Video LAN Codec (VLC) framework.
 */
class FVlcMediaPlayer
	: public IMediaInfo
	, public IMediaPlayer
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InInstance The LibVLC instance to use. 
	 */
	FVlcMediaPlayer(FLibvlcInstance* InInstance);

	/** Destructor. */
	~FVlcMediaPlayer();

public:

	// IMediaInfo interface

	virtual FTimespan GetDuration() const override;
	virtual TRange<float> GetSupportedRates( EMediaPlaybackDirections Direction, bool Unthinned ) const override;
	virtual FString GetUrl() const override;
	virtual bool SupportsRate( float Rate, bool Unthinned ) const override;
	virtual bool SupportsScrubbing() const override;
	virtual bool SupportsSeeking() const override;

public:

	// IMediaPlayer interface

	virtual void Close() override;
	virtual const IMediaInfo& GetMediaInfo() const override;
	virtual float GetRate() const override;
	virtual FTimespan GetTime() const override;
	virtual const TArray<IMediaTrackRef>& GetTracks() const override;
	virtual bool IsLooping() const override;
	virtual bool IsPaused() const override;
	virtual bool IsPlaying() const override;
	virtual bool IsReady() const override;
	virtual bool Open( const FString& Url ) override;
	virtual bool Open( const TSharedRef<TArray<uint8>, ESPMode::ThreadSafe>& Buffer, const FString& OriginalUrl ) override;
	virtual bool Seek( const FTimespan& Time ) override;
	virtual bool SetLooping( bool Looping ) override;
	virtual bool SetRate( float Rate ) override;

	DECLARE_DERIVED_EVENT(FWmfMediaPlayer, IMediaPlayer::FOnMediaClosed, FOnMediaClosed);
	virtual FOnMediaClosed& OnClosed() override
	{
		return ClosedEvent;
	}

	DECLARE_DERIVED_EVENT(FWmfMediaPlayer, IMediaPlayer::FOnMediaOpened, FOnMediaOpened);
	virtual FOnMediaOpened& OnOpened() override
	{
		return OpenedEvent;
	}

protected:

	/**
	 * Initialize the media player object.
	 *
	 * @param Media The media to play.
	 * @return true on success, false otherwise.
	 */
	bool InitializeMediaPlayer(FLibvlcMedia* Media);

	/** Initialize the media tracks. */
	void InitializeTracks();

private:

	/** Handles the ticker. */
	bool HandleTicker(float DeltaTime);

private:

	/** Handles event callbacks. */
	static void HandleEventCallback(FLibvlcEvent* Event, void* UserData);

	/** Handles open callbacks from VLC. */
	static int HandleMediaOpen(void* Opaque, void** OutData, uint64* OutSize);

	/** Handles read callbacks from VLC. */
	static SSIZE_T HandleMediaRead(void* Opaque, void* Buffer, SIZE_T Length);

	/** Handles seek callbacks from VLC. */
	static int HandleMediaSeek(void* Opaque, uint64 Offset);

	/** Handles close callbacks from VLC. */
	static void HandleMediaClose(void* Opaque);

private:

	/** Current playback time to work around VLC's broken time tracking. */
	float CurrentTime;

	/** Buffer holding media data (for in-memory playback only). */
	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Data;

	/** The current read position in the media data (for in-memory playback only). */
	SIZE_T DataPosition;

	/** The desired playback rate. */
	float DesiredRate;

	/** Collection of received player events. */
	TQueue<ELibvlcEventType, EQueueMode::Mpsc> Events;

	// Currently opened media.
	FString MediaUrl;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Whether playback should be looping. */
	bool ShouldLoop;

	/** Handle to the registered ticker. */
	FDelegateHandle TickerHandle;

	/** The pseudo-tracks in the media. */
	TArray<IMediaTrackRef> Tracks;

	/** The LibVLC instance. */
	FLibvlcInstance* VlcInstance;

private:

	/** Holds an event delegate that is invoked when media has been closed. */
	FOnMediaClosed ClosedEvent;

	/** Holds an event delegate that is invoked when media has been opened. */
	FOnMediaOpened OpenedEvent;
};
