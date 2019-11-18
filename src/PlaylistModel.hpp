#pragma once

#include <QAbstractListModel>

enum PlaylistColumn
{
    NOW_PLAYING,
    ARTIST_ALBUM,
    TRACK,
    TITLE,
    DURATION
};

struct Playlist;
struct AudioMetaData;
struct IAudioMetaDataProvider;

class PlaylistModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    PlaylistModel(IAudioMetaDataProvider &, Playlist &, QObject * = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation, int = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &, int = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &) const override;

private:
    QVariant roleAlignment(int column) const;
    QVariant dataTitle(const QString &filepath, const AudioMetaData &) const;
    QVariant dataArtistAlbum(const AudioMetaData &) const;
    QVariant dataDuration(const AudioMetaData &) const;
    QVariant dataTrack(const AudioMetaData &) const;

public slots:
    void update();

private:
    IAudioMetaDataProvider &audioMetaDataProvider_;
    Playlist &playlist_;
};
