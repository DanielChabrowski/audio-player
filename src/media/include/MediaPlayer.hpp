#pragma once

#include <QObject>
#include <QUrl>

struct PlaylistTrack;

enum class MediaStatus
{
    NoMedia,
    LoadingMedia,
    LoadedMedia,
    StalledMedia,
    BufferingMedia,
    BufferedMedia,
    EndOfMedia,
    InvalidMedia,
};

enum class PlaybackState
{
    StoppedState,
    PlayingState,
    PausedState
};

class MediaPlayer : public QObject
{
    Q_OBJECT

public:
    virtual ~MediaPlayer() = default;

    virtual qint64 position() const = 0;
    virtual float volume() const = 0;
    virtual PlaybackState playbackState() const = 0;

    static std::unique_ptr<MediaPlayer> create();

public slots:
    virtual void setPosition(qint64 position) = 0;
    virtual void setVolume(float volume, const char *eventSource) = 0;
    virtual void setTrack(const PlaylistTrack &) = 0;

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

signals:
    void positionChanged(qint64 position);
    void volumeChanged(float volume, const char *eventSource);
    void mediaStatusChanged(MediaStatus status);
    void playbackStateChanged(PlaybackState state);
    void trackChanged(const PlaylistTrack &track);
};
