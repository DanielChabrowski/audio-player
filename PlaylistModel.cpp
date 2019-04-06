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
            if(not song.name.empty())
            {
                return QString{ song.name.c_str() };
            }
            else
            {
                return QFileInfo(song.path.c_str()).completeBaseName();
            }
        }

        case PlaylistColumn::ARTIST_ALBUM:
        {
            return QString{ song.artist.c_str() } + " - " + QString{ song.album.c_str() };
        }

        case PlaylistColumn::TRACK:
        {
            const auto track = QString{ "%1" }.arg(song.albumInfo.trackNumber, 2, 10, QChar('0'));

            if(-1 != song.albumInfo.discNumber)
            {
                return QString{ "%1.%2" }.arg(song.albumInfo.discNumber).arg(track);
            }

            return track;
        }

        case PlaylistColumn::DURATION:
        {
            const auto duration = song.duration.count();
            const auto minutes = duration / 60;
            const auto seconds = duration % 60;
            return QString{ "%1:%2" }.arg(minutes).arg(seconds, 2, 10, QChar('0'));
        }
        }

        return QString("Unsupported column %1").arg(index.column());
    }

    return {};
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
}

void PlaylistModel::update()
{
    beginResetModel();
    endResetModel();
}
