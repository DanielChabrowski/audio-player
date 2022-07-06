#include "MprisPlugin.hpp"

#include "mediaplayer2adaptor.h"
#include "playeradaptor.h"

namespace plugins
{
MprisPlugin::MprisPlugin(QObject *parent)
: QObject(parent)
{
    new PlayerAdaptor(this);
    new MediaPlayer2Adaptor(this);

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
    return "Playing";
}

qlonglong MprisPlugin::position() const
{
    return 0;
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

void MprisPlugin::setVolume(double)
{
}

void MprisPlugin::Next()
{
}

void MprisPlugin::OpenUri(const QString &) // Uri
{
}

void MprisPlugin::Pause()
{
}

void MprisPlugin::Play()
{
}

void MprisPlugin::PlayPause()
{
}

void MprisPlugin::Previous()
{
}

void MprisPlugin::Seek(qlonglong) // offset
{
}

void MprisPlugin::SetPosition(const QDBusObjectPath &, qlonglong) // trackid, position
{
}

void MprisPlugin::Stop()
{
}
} // namespace plugins
