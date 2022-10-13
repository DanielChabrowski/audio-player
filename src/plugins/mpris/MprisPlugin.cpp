#include "MprisPlugin.hpp"

#include "MediaPlayer.hpp"
#include "MprisTypes.hpp"
#include "Playlist.hpp"
#include "mediaplayer2adaptor.h"
#include "playeradaptor.h"
#include "playlistsadaptor.h"

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QFileInfo>
#include <QtGlobal>

namespace
{
constexpr auto mprisEventSource{ "mpris" };
}

QDBusArgument &operator<<(QDBusArgument &arg, const MprisPlaylist &playlist)
{
    arg.beginStructure();
    arg << playlist.id << playlist.name << playlist.icon;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, MprisPlaylist &playlist)
{
    arg.beginStructure();
    arg >> playlist.id >> playlist.name >> playlist.icon;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const MprisMaybePlaylist &playlist)
{
    arg.beginStructure();
    arg << playlist.valid << playlist.playlist;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, MprisMaybePlaylist &playlist)
{
    arg.beginStructure();
    arg >> playlist.valid >> playlist.playlist;
    arg.endStructure();
    return arg;
}

namespace
{
QString toString(PlaybackState state)
{
    switch(state)
    {
    case PlaybackState::PausedState:
        return "Paused";
    case PlaybackState::StoppedState:
        return "Stopped";
    case PlaybackState::PlayingState:
        return "Playing";
    }

    Q_UNREACHABLE();
}

void emitPropertyChanged(const QString &key, const QVariant &value, const char *interfaceName)
{
    QDBusMessage msg = QDBusMessage::createSignal(
        "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    msg.setArguments({
        interfaceName,
        QVariantMap{ { key, value } },
        QStringList{},
    });
    QDBusConnection::sessionBus().send(msg);
}

QVariantMap convert(const PlaylistTrack &track)
{
    auto length = std::chrono::duration<quint64, std::milli>(track.audioMetaData->duration).count();

    if(track.audioMetaData && !track.audioMetaData->title.isEmpty())
    {
        return {
            { "xesam:url", track.path },
            { "xesam:title", track.audioMetaData->title },
            { "xesam:artist", track.audioMetaData->artist },
            { "xesam:album", track.audioMetaData->albumName },
            { "xesam:discNumber", track.audioMetaData->discNumber },
            { "xesam:trackNumber", track.audioMetaData->trackNumber },
            { "mpris:length", length },
        };
    }
    else
    {
        return {
            { "xesam:url", track.path },
            { "xesam:title", QFileInfo(track.path).completeBaseName() },
            { "mpris:length", length },
        };
    }
}

constexpr auto playerInterfaceName = "org.mpris.MediaPlayer2.Player";
// constexpr auto playlistInterfaceName = "org.mpris.MediaPlayer2.Playlist";
} // namespace

namespace plugins
{
MprisPlugin::MprisPlugin(MediaPlayer &mediaPlayer)
: mediaPlayer_{ mediaPlayer }
{
    qDBusRegisterMetaType<MprisPlaylist>();
    qDBusRegisterMetaType<MprisMaybePlaylist>();
    qDBusRegisterMetaType<MprisPlaylistList>();

    new PlayerAdaptor(this);
    new MediaPlayer2Adaptor(this);
    new PlaylistsAdaptor(this);

    connect(&mediaPlayer, &MediaPlayer::playbackStateChanged, this,
        [](PlaybackState state)
        { emitPropertyChanged("playbackStatus", toString(state), playerInterfaceName); });

    connect(&mediaPlayer, &MediaPlayer::trackChanged, this,
        [](const PlaylistTrack &track)
        { emitPropertyChanged("metadata", convert(track), playerInterfaceName); });

    connect(&mediaPlayer, &MediaPlayer::volumeChanged, this,
        [](float volume, const char *eventSource)
        {
            if(eventSource != mprisEventSource)
            {
                emitPropertyChanged("volume", static_cast<double>(volume), playerInterfaceName);
            }
        });

    auto dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.mpris.MediaPlayer2.foobar");
    dbus.registerObject("/org/mpris/MediaPlayer2", this);
    dbus.registerObject("/org/mpris/MediaPlayer2/Player", this);
    dbus.registerObject("/org/mpris/MediaPlayer2/Playlist", this);

    qInfo() << "MPRIS plugin enabled";
}

MprisPlugin::~MprisPlugin()
{
    auto dbus = QDBusConnection::sessionBus();
    dbus.unregisterService("org.mpris.MediaPlayer2.foobar");
}

bool MprisPlugin::canControl() const
{
    return true;
}

bool MprisPlugin::canGoNext() const
{
    return true;
}

bool MprisPlugin::canGoPrevious() const
{
    return true;
}

bool MprisPlugin::canPause() const
{
    return true;
}

bool MprisPlugin::canPlay() const
{
    return true;
}

bool MprisPlugin::canSeek() const
{
    return true;
}

QString MprisPlugin::loopStatus() const
{
    return "Playing";
}

void MprisPlugin::setLoopStatus(const QString &)
{
}

double MprisPlugin::maximumRate() const
{
    return 1.0;
}

QVariantMap MprisPlugin::metadata() const
{
    return QVariantMap{};
}

double MprisPlugin::minimumRate() const
{
    return 1.0;
}

QString MprisPlugin::playbackStatus() const
{
    const auto state = mediaPlayer_.playbackState();
    return toString(state);
}

qlonglong MprisPlugin::position() const
{
    return mediaPlayer_.position();
}

double MprisPlugin::rate() const
{
    return 1.0;
}

void MprisPlugin::setRate(double)
{
}

bool MprisPlugin::shuffle() const
{
    return false;
}

void MprisPlugin::setShuffle(bool)
{
}

double MprisPlugin::volume() const
{
    return mediaPlayer_.volume();
}

void MprisPlugin::setVolume(double volume)
{
    mediaPlayer_.setVolume(volume, mprisEventSource);
}

void MprisPlugin::Next()
{
}

void MprisPlugin::OpenUri(const QString &url)
{
    Q_UNUSED(url);
}

void MprisPlugin::Pause()
{
    mediaPlayer_.pause();
}

void MprisPlugin::Play()
{
    mediaPlayer_.play();
}

void MprisPlugin::PlayPause()
{
    if(mediaPlayer_.playbackState() == PlaybackState::PlayingState)
    {
        mediaPlayer_.pause();
    }
    else
    {
        mediaPlayer_.play();
    }
}

void MprisPlugin::Previous()
{
}

void MprisPlugin::Seek(qlonglong offset)
{
    Q_UNUSED(offset);
}

void MprisPlugin::SetPosition(const QDBusObjectPath &trackId, qlonglong position)
{
    Q_UNUSED(trackId);
    mediaPlayer_.setPosition(position);
}

void MprisPlugin::Stop()
{
    mediaPlayer_.stop();
}

bool MprisPlugin::canQuit() const
{
    return false;
}

bool MprisPlugin::canRaise() const
{
    return false;
}

bool MprisPlugin::canSetFullscreen() const
{
    return false;
}

QString MprisPlugin::desktopEntry() const
{
    return "foobar";
}

bool MprisPlugin::fullscreen() const
{
    return false;
}

void MprisPlugin::setFullscreen(bool)
{
}

bool MprisPlugin::hasTrackList() const
{
    return false;
}

QString MprisPlugin::identity() const
{
    return "Foobar";
}

QStringList MprisPlugin::supportedMimeTypes() const
{
    return {};
}

QStringList MprisPlugin::supportedUriSchemes() const
{
    return {};
}

void MprisPlugin::Quit()
{
}

void MprisPlugin::Raise()
{
}

MprisMaybePlaylist MprisPlugin::activePlaylist() const
{
    return MprisMaybePlaylist{
        false,
        MprisPlaylist{ QDBusObjectPath{ "/org/foobar/playlist/0" }, "Playlist name", "" },
    };
}

QStringList MprisPlugin::orderings() const
{
    return { "Alphabetical" };
}

uint MprisPlugin::playlistCount() const
{
    return 0;
}

void MprisPlugin::ActivatePlaylist(const QDBusObjectPath &playlistId)
{
    Q_UNUSED(playlistId);
}

MprisPlaylistList MprisPlugin::GetPlaylists(uint index, uint maxCount, const QString &order, bool reverseOrder)
{
    Q_UNUSED(index);
    Q_UNUSED(maxCount);
    Q_UNUSED(order);
    Q_UNUSED(reverseOrder);
    return {};
}
} // namespace plugins
