#pragma once

#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QString>

struct MprisPlaylist
{
    QDBusObjectPath id;
    QString name;
    QString icon;
};
Q_DECLARE_METATYPE(MprisPlaylist)

using MprisPlaylistList = QList<MprisPlaylist>;
Q_DECLARE_METATYPE(MprisPlaylistList)

struct MprisMaybePlaylist
{
    bool valid;
    MprisPlaylist playlist;
};
Q_DECLARE_METATYPE(MprisMaybePlaylist)
