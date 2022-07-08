#include "MprisPlugin.hpp"

#include <QtGlobal>

#include "MediaPlayer.hpp"

#include "mediaplayer2adaptor.h"
#include "playeradaptor.h"

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

void emitPropertyChanged(const QString &key, const QVariant &value)
{
    QDBusMessage msg = QDBusMessage::createSignal(
        "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    msg.setArguments({
        "org.mpris.MediaPlayer2.Player",
        QVariantMap{ { key, value } },
        QStringList{},
    });
    QDBusConnection::sessionBus().send(msg);
}
} // namespace

namespace plugins
{
MprisPlugin::MprisPlugin(MediaPlayer &mediaPlayer)
: mediaPlayer_{ mediaPlayer }
{
    new PlayerAdaptor(this);
    new MediaPlayer2Adaptor(this);

    connect(&mediaPlayer, &MediaPlayer::playbackStateChanged, this,
        [](PlaybackState state) { emitPropertyChanged("playbackStatus", toString(state)); });

    auto dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.mpris.MediaPlayer2.foobar");
    dbus.registerObject("/org/mpris/MediaPlayer2", this);
    dbus.registerObject("/org/mpris/MediaPlayer2/Player", this);
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
    return 1.0;
}

void MprisPlugin::setVolume(double volume)
{
    mediaPlayer_.setVolume(volume);
}

void MprisPlugin::Next()
{
}

void MprisPlugin::OpenUri(const QString &) // Uri
{
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

void MprisPlugin::Seek(qlonglong) // offset
{
}

void MprisPlugin::SetPosition(const QDBusObjectPath &, qlonglong position) // trackid, position
{
    mediaPlayer_.setPosition(position);
}

void MprisPlugin::Stop()
{
    mediaPlayer_.stop();
}
} // namespace plugins
