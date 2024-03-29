#include "MediaPlayerQtBackend.hpp"

#include "Playlist.hpp"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QtGlobal>

namespace
{
MediaStatus convert(QMediaPlayer::MediaStatus status)
{
    switch(status)
    {
    case QMediaPlayer::NoMedia:
        return MediaStatus::NoMedia;
    case QMediaPlayer::LoadingMedia:
        return MediaStatus::LoadingMedia;
    case QMediaPlayer::LoadedMedia:
        return MediaStatus::LoadedMedia;
    case QMediaPlayer::StalledMedia:
        return MediaStatus::StalledMedia;
    case QMediaPlayer::BufferingMedia:
        return MediaStatus::BufferingMedia;
    case QMediaPlayer::BufferedMedia:
        return MediaStatus::BufferedMedia;
    case QMediaPlayer::EndOfMedia:
        return MediaStatus::EndOfMedia;
    case QMediaPlayer::InvalidMedia:
        return MediaStatus::InvalidMedia;
    }

    Q_UNREACHABLE();
}

PlaybackState convert(QMediaPlayer::PlaybackState state)
{
    switch(state)
    {
    case QMediaPlayer::StoppedState:
        return PlaybackState::StoppedState;
    case QMediaPlayer::PlayingState:
        return PlaybackState::PlayingState;
    case QMediaPlayer::PausedState:
        return PlaybackState::PausedState;
    }

    Q_UNREACHABLE();
}
} // namespace

std::unique_ptr<MediaPlayer> MediaPlayer::create()
{
    return std::make_unique<MediaPlayerQtBackend>();
}

struct MediaPlayerQtBackend::Private
{
public:
    explicit Private(QObject *parent)
    : player{ parent }
    {
        player.setAudioOutput(new QAudioOutput(&player));
    }

    QMediaPlayer player;
};

MediaPlayerQtBackend::MediaPlayerQtBackend(QObject *parent)
: impl{ std::make_unique<Private>(parent) }
{
    connect(&impl->player, &QMediaPlayer::positionChanged, this, &MediaPlayerQtBackend::priv_positionChanged);
    connect(&impl->player, &QMediaPlayer::mediaStatusChanged, this,
        [this](QMediaPlayer::MediaStatus status) { emit mediaStatusChanged(convert(status)); });
    connect(&impl->player, &QMediaPlayer::playbackStateChanged, this,
        [this](QMediaPlayer::PlaybackState newState)
        { emit playbackStateChanged(convert(newState)); });
}

MediaPlayerQtBackend::~MediaPlayerQtBackend() = default;

qint64 MediaPlayerQtBackend::position() const
{
    return impl->player.position();
}

float MediaPlayerQtBackend::volume() const
{
    return impl->player.audioOutput()->volume();
}

PlaybackState MediaPlayerQtBackend::playbackState() const
{
    return convert(impl->player.playbackState());
}

void MediaPlayerQtBackend::setPosition(qint64 position)
{
    impl->player.setPosition(position);
}

void MediaPlayerQtBackend::setVolume(float volume, const char *eventSource)
{
    impl->player.audioOutput()->setVolume(volume);
    emit volumeChanged(volume, eventSource);
}

void MediaPlayerQtBackend::setTrack(const PlaylistTrack &playlistTrack)
{
    impl->player.setSource(playlistTrack.path);
    emit trackChanged(playlistTrack);
}

void MediaPlayerQtBackend::play()
{
    impl->player.play();
}

void MediaPlayerQtBackend::pause()
{
    impl->player.pause();
}

void MediaPlayerQtBackend::stop()
{
    impl->player.stop();
}

void MediaPlayerQtBackend::priv_positionChanged(qint64 position)
{
    emit positionChanged(position);
}
