// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"
#include "Ticker.h"
#include "vlc/vlc.h"
#include <mutex>

#define LOCTEXT_NAMESPACE "FVlcMediaPlayer"


/* FVlcMediaPlayer structors
 *****************************************************************************/


FVlcMediaPlayer::FVlcMediaPlayer(FLibvlcInstance* InVlcInstance)
	: CurrentTime(0.0f)
	, DataPosition(0)
	, DesiredRate(0.0)
	, Player(nullptr)
	, ShouldLoop(false)
	, VlcInstance(InVlcInstance)
{
	TickerHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FVlcMediaPlayer::HandleTicker), 0.0f);
}


FVlcMediaPlayer::~FVlcMediaPlayer()
{
	Close();

	FTicker::GetCoreTicker().RemoveTicker(TickerHandle);
}


/* IMediaInfo interface
 *****************************************************************************/

FTimespan FVlcMediaPlayer::GetDuration() const
{
	if (Player == nullptr)
	{
		return FTimespan::Zero();
	}

	return FTimespan::FromMilliseconds(FVlc::MediaPlayerGetLength(Player));
}


TRange<float> FVlcMediaPlayer::GetSupportedRates(EMediaPlaybackDirections Direction, bool Unthinned) const
{
	return TRange<float>(1.0f);
}


FString FVlcMediaPlayer::GetUrl() const
{
	return MediaUrl;
}


bool FVlcMediaPlayer::SupportsRate(float Rate, bool Unthinned) const
{
	return (Rate == 1.0f);
}


bool FVlcMediaPlayer::SupportsScrubbing() const
{
	return ((Player != nullptr) && (FVlc::MediaPlayerIsSeekable(Player) != 0));
}


bool FVlcMediaPlayer::SupportsSeeking() const
{
	return ((Player != nullptr) && (FVlc::MediaPlayerIsSeekable(Player) != 0));
}


/* IMediaPlayer interface
 *****************************************************************************/

void FVlcMediaPlayer::Close()
{
	if (Player == nullptr)
	{
		return;
	}

	Data.Reset();
	Tracks.Reset();

	// release player
	FVlc::MediaPlayerStop(Player);
	FVlc::MediaPlayerRelease(Player);
	Player = nullptr;

	// reset fields
	DataPosition = 0;
	MediaUrl = FString();

	ClosedEvent.Broadcast();
}


const IMediaInfo& FVlcMediaPlayer::GetMediaInfo() const 
{
	return *this;
}


float FVlcMediaPlayer::GetRate() const
{
	if ((Player == nullptr) || !IsPlaying())
	{
		return 0.0f;
	}

	return FVlc::MediaPlayerGetRate(Player);
}


FTimespan FVlcMediaPlayer::GetTime() const 
{
	if (Player == nullptr)
	{
		return FTimespan::Zero();
	}

	return FTimespan::FromSeconds(CurrentTime);
	/*int64 Time = FMath::Min<int64>(0, FVlc::MediaPlayerGetTime(Player));

	if (Time < 0)
	{
		return FTimespan::Zero();
	}

	return FTimespan::FromMilliseconds(Time);*/
}


const TArray<IMediaTrackRef>& FVlcMediaPlayer::GetTracks() const
{
	return Tracks;
}


bool FVlcMediaPlayer::IsLooping() const 
{
	return ShouldLoop;
}


bool FVlcMediaPlayer::IsPaused() const
{
	return ((Player != nullptr) && (FVlc::MediaPlayerGetState(Player) == ELibvlcState::Paused));
}


bool FVlcMediaPlayer::IsPlaying() const
{
	return ((Player != nullptr) && (FVlc::MediaPlayerGetState(Player) == ELibvlcState::Playing));
}


bool FVlcMediaPlayer::IsReady() const
{
	if (Player == nullptr)
	{
		return false;
	}

	ELibvlcState State = FVlc::MediaPlayerGetState(Player);

	return ((State >= ELibvlcState::Playing) && (State < ELibvlcState::Error));
}


bool FVlcMediaPlayer::Open(const FString& Url)
{
	if (Url.IsEmpty())
	{
		return false;
	}

	Close();

	FLibvlcMedia* NewMedia = (Url.Contains(TEXT("://")))
		? FVlc::MediaNewLocation(VlcInstance, TCHAR_TO_ANSI(*Url))
		: FVlc::MediaNewPath(VlcInstance, TCHAR_TO_ANSI(*Url));

	if (NewMedia == nullptr)
	{
		return false;
	}

	MediaUrl = Url;

	return InitializeMediaPlayer(NewMedia);
}


bool FVlcMediaPlayer::Open( const TSharedRef<TArray<uint8>, ESPMode::ThreadSafe>& Buffer, const FString& OriginalUrl )
{
	if ((Buffer->Num() == 0) || OriginalUrl.IsEmpty())
	{
		return false;
	}

	Data = Buffer;

	FLibvlcMedia* NewMedia = FVlc::MediaNewCallbacks(
		VlcInstance,
		&FVlcMediaPlayer::HandleMediaOpen,
		&FVlcMediaPlayer::HandleMediaRead,
		&FVlcMediaPlayer::HandleMediaSeek,
		&FVlcMediaPlayer::HandleMediaClose,
		this);

	if (NewMedia == nullptr)
	{
		Data.Reset();

		return false;
	}

	MediaUrl = OriginalUrl;

	return InitializeMediaPlayer(NewMedia);
}


bool FVlcMediaPlayer::Seek( const FTimespan& Time )
{
	if (!IsReady())
	{
		return false;
	}

	FVlc::MediaPlayerSetTime(Player, Time.GetTotalMilliseconds());
	CurrentTime = Time.GetTotalSeconds();

	return true;
}


bool FVlcMediaPlayer::SetLooping( bool Looping )
{
	ShouldLoop = Looping;
	return true;
}


bool FVlcMediaPlayer::SetRate(float Rate)
{
	if (Player == nullptr)
	{
		return false;
	}

	if ((FVlc::MediaPlayerSetRate(Player, Rate) == -1))
	{
		return false;
	}

	DesiredRate = Rate;

	if (FMath::IsNearlyZero(Rate))
	{
		if (IsPlaying())
		{
			FVlc::MediaPlayerPause(Player);
		}
	}
	else if (!IsPlaying())
	{
		FVlc::MediaPlayerPlay(Player);
	}

	return true;
}


/* FVlcMediaPlayer implementation
 *****************************************************************************/

bool FVlcMediaPlayer::InitializeMediaPlayer(FLibvlcMedia* Media)
{
	Player = FVlc::MediaPlayerNewFromMedia(Media);

	if (Player == nullptr)
	{
		Close();

		return false;
	}

	// attach to event managers
	FLibvlcEventManager* MediaEventManager = FVlc::MediaEventManager(Media);
	FLibvlcEventManager* PlayerEventManager = FVlc::MediaPlayerEventManager(Player);

	if ((MediaEventManager == nullptr) || (PlayerEventManager == nullptr))
	{
		Close();

		return false;
	}

	FVlc::EventAttach(MediaEventManager, ELibvlcEventType::MediaParsedChanged, &FVlcMediaPlayer::HandleEventCallback, this);
	FVlc::EventAttach(PlayerEventManager, ELibvlcEventType::MediaPlayerEndReached, &FVlcMediaPlayer::HandleEventCallback, this);
	FVlc::EventAttach(PlayerEventManager, ELibvlcEventType::MediaPlayerPlaying, &FVlcMediaPlayer::HandleEventCallback, this);

	//FVlc::MediaParseAsync(Media);
	FVlc::MediaPlayerPlay(Player);
	FVlc::MediaRelease(Media);

	OpenedEvent.Broadcast(MediaUrl);

	return true;
}


void FVlcMediaPlayer::InitializeTracks()
{
	if (Player == nullptr)
	{
		return;
	}
	
	FVlc::MediaPlayerStop(Player);

	FLibvlcMedia* Media = FVlc::MediaPlayerGetMedia(Player);

	if (Media == nullptr)
	{
		return;
	}

	Tracks.Empty();

	// audio tracks
	FLibvlcTrackDescription* AudioTrackDescr = FVlc::AudioGetTrackDescription(Player);

	while (AudioTrackDescr != nullptr)
	{
		if (AudioTrackDescr->Id != -1)
		{
			Tracks.Add(MakeShareable(new FVlcMediaAudioTrack(Player, Tracks.Num(), AudioTrackDescr)));
		}

		AudioTrackDescr = AudioTrackDescr->Next;
	}

	FVlc::TrackDescriptionListRelease(AudioTrackDescr);

	// caption tracks
	FLibvlcTrackDescription* CaptionTrackDescr = FVlc::VideoGetSpuDescription(Player);

	while (CaptionTrackDescr != nullptr)
	{
		if (CaptionTrackDescr->Id != -1)
		{
			Tracks.Add(MakeShareable(new FVlcMediaCaptionTrack(Player, Tracks.Num(), CaptionTrackDescr)));
		}

		CaptionTrackDescr = CaptionTrackDescr->Next;
	}

	FVlc::TrackDescriptionListRelease(AudioTrackDescr);

	// video tracks
	FLibvlcTrackDescription* VideoTrackDescr = FVlc::VideoGetTrackDescription(Player);

	while (VideoTrackDescr != nullptr)
	{
		if (VideoTrackDescr->Id != -1)
		{
			Tracks.Add(MakeShareable(new FVlcMediaVideoTrack(Player, Tracks.Num(), VideoTrackDescr)));
		}

		VideoTrackDescr = VideoTrackDescr->Next;
	}

	FVlc::TrackDescriptionListRelease(VideoTrackDescr);
}


/* FVlcMediaPlayer callbacks
 *****************************************************************************/

bool FVlcMediaPlayer::HandleTicker(float DeltaTime)
{
	if (Tracks.Num() == 0)
	{
		InitializeTracks();
	}
	else
	{
		// process events
		ELibvlcEventType Event;

		while (Events.Dequeue(Event))
		{
			switch (Event)
			{
			case ELibvlcEventType::MediaParsedChanged:
				//MediaPlayer->InitializeTracks();
				break;

			case ELibvlcEventType::MediaPlayerEndReached:
				FVlc::MediaPlayerStop(Player);
				CurrentTime = 0.0f;

				if (ShouldLoop && (DesiredRate != 0.0f))
				{
					SetRate(DesiredRate);
				}
				break;

			case ELibvlcEventType::MediaPlayerPlaying:
				break;

			default:
				continue;
			}
		}

		// update tracks
		if (IsPlaying())
		{
			CurrentTime += GetRate() * DeltaTime;

			for (IMediaTrackRef& Track : Tracks)
			{
				FVlcMediaTrack& VlcTrack = static_cast<FVlcMediaTrack&>(*Track);
				VlcTrack.SetTime(CurrentTime);
			}
		}
	}

	return true;
}


/* FVlcMediaPlayer static functions
 *****************************************************************************/

void FVlcMediaPlayer::HandleEventCallback(FLibvlcEvent* Event, void* UserData)
{
	FVlcMediaPlayer* MediaPlayer = (FVlcMediaPlayer*)UserData;
	MediaPlayer->Events.Enqueue(Event->Type);
}


int FVlcMediaPlayer::HandleMediaOpen(void* Opaque, void** OutData, uint64* OutSize)
{
	FVlcMediaPlayer* MediaPlayer = (FVlcMediaPlayer*)Opaque;

	if (!MediaPlayer->Data.IsValid())
	{
		return 0;
	}

	*OutData = MediaPlayer->Data->GetData();
	*OutSize = MediaPlayer->Data->Num();
	
	return 0;
}


SSIZE_T FVlcMediaPlayer::HandleMediaRead(void* Opaque, void* Buffer, SIZE_T Length)
{
	FVlcMediaPlayer* MediaPlayer = (FVlcMediaPlayer*)Opaque;
	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Data = MediaPlayer->Data;

	if (!MediaPlayer->Data.IsValid())
	{
		return -1;
	}

	SIZE_T DataSize = (SIZE_T)Data->Num();
	SIZE_T BytesToRead = FMath::Min(Length, DataSize);
	SIZE_T& DataPosition = MediaPlayer->DataPosition;

	if ((DataSize - BytesToRead) < DataPosition)
	{
		BytesToRead = DataSize - DataPosition;
	}

	if (BytesToRead > 0)
	{
		FMemory::Memcpy(Buffer, Data->GetData() + DataPosition, BytesToRead);
		DataPosition += BytesToRead;
	}

	return (SSIZE_T)BytesToRead;
}


int FVlcMediaPlayer::HandleMediaSeek(void* Opaque, uint64 Offset)
{
	FVlcMediaPlayer* MediaPlayer = (FVlcMediaPlayer*)Opaque;
	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Data = MediaPlayer->Data;

	if (!MediaPlayer->Data.IsValid())
	{
		return -1;
	}

	if ((uint64)Data->Num() <= Offset)
	{
		return -1;
	}

	MediaPlayer->DataPosition = SIZE_T(Offset);

	return 0;
}


void FVlcMediaPlayer::HandleMediaClose(void* Opaque)
{
	FVlcMediaPlayer* MediaPlayer = (FVlcMediaPlayer*)Opaque;

	MediaPlayer->Data.Reset();
	MediaPlayer->DataPosition = 0;
}


#undef LOCTEXT_NAMESPACE
