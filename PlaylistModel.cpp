#include "PlaylistModel.hpp"

#include <QFileInfo>

#include "Playlist.hpp"

namespace
{
const char *const labels[] = {
    "Playing", "Artist/album", "Track", "Title", "Duration",
};

enum PlaylistColumn
{
    NOW_PLAYING,
    ARTIST_ALBUM,
    TRACK,
    TITLE,
    DURATION
};
} // namespace

PlaylistModel::PlaylistModel(Playlist &playlist, QObject *parent)
: QAbstractListModel{ parent }
, playlist_{ playlist }
{
}

int PlaylistModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(playlist_.songs.size());
}

int PlaylistModel::columnCount(const QModelIndex &) const
{
    return 5;
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(Qt::DisplayRole != role)
    {
        return {};
    }

    if(Qt::Horizontal == orientation)
    {
        return QString(labels[section]);
    }
    else
    {
        return QString("Vertical %1").arg(section);
    }
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
    {
        return {};
    }

    const auto playlistSize = static_cast<int>(playlist_.songs.size());
    const auto row = index.row();
    if(row >= playlistSize || row < 0)
    {
        return {};
    }
    const auto col = index.column();

    if(Qt::DisplayRole == role)
    {
        const auto &song = playlist_.songs.at(row);

        switch(col)
        {
        case PlaylistColumn::NOW_PLAYING:
        {
            return row == playlist_.currentSongIndex ? QString{ '>' } : "";
        }

        case PlaylistColumn::TITLE:
        {
            return dataTitle(song);
        }

        case PlaylistColumn::ARTIST_ALBUM:
        {
            return dataArtistAlbum(song);
        }

        case PlaylistColumn::TRACK:
        {
            return dataTrack(song);
        }

        case PlaylistColumn::DURATION:
        {
            return dataDuration(song);
        }
        }

        return QString("Unsupported column %1").arg(index.column());
    }

    if(Qt::TextAlignmentRole == role)
    {
        return roleAlignment(col);
    }

    return {};
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
}


QVariant PlaylistModel::roleAlignment(int column) const
{
    switch(column)
    {
    case PlaylistColumn::NOW_PLAYING:
        return Qt::AlignmentFlag::AlignCenter + Qt::AlignmentFlag::AlignVCenter;

    case PlaylistColumn::TRACK:
    case PlaylistColumn::DURATION:
        return Qt::AlignmentFlag::AlignRight + Qt::AlignVCenter;
    }

    return Qt::AlignmentFlag::AlignLeft + Qt::AlignVCenter;
}

QVariant PlaylistModel::dataTitle(const Song &song) const
{
    if(not song.name.empty())
    {
        return QString{ song.name.c_str() };
    }

    return QFileInfo(song.path.c_str()).completeBaseName();
}

QVariant PlaylistModel::dataArtistAlbum(const Song &song) const
{
    const QString artist{ song.artist.empty() ? "?" : song.artist.c_str() };
    const QString album{ song.album.empty() ? "?" : song.album.c_str() };
    return artist + " - " + album;
}

QVariant PlaylistModel::dataDuration(const Song &song) const
{
    const auto duration = song.duration.count();
    const auto minutes = duration / 60;
    const auto seconds = duration % 60;
    return QString{ "%1:%2" }.arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

QVariant PlaylistModel::dataTrack(const Song &song) const
{
    constexpr int missingData{ -1 };

    if(missingData != song.albumInfo.trackNumber)
    {
        const auto track = QString{ "%1" }.arg(song.albumInfo.trackNumber, 2, 10, QChar('0'));

        if(missingData != song.albumInfo.discNumber)
        {
            return QString{ "%1.%2" }.arg(song.albumInfo.discNumber).arg(track);
        }

        return track;
    }

    return {};
}

void PlaylistModel::update()
{
    beginResetModel();
    endResetModel();
}
