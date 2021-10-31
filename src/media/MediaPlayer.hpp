#pragma once

#include <QObject>
#include <QUrl>

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
    virtual PlaybackState playbackState() const = 0;

public slots:
    virtual void setPosition(qint64 position) = 0;
    virtual void setVolume(float volume) = 0;
    virtual void setSource(QUrl url) = 0;

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

signals:
    void positionChanged(qint64 position);
    void mediaStatusChanged(MediaStatus status);
};
