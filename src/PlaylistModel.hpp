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
struct Song;

class PlaylistModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    PlaylistModel(Playlist &, QObject * = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation, int = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &, int = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &) const override;

    QVariant roleAlignment(int column) const;
    QVariant dataTitle(const Song &) const;
    QVariant dataArtistAlbum(const Song &) const;
    QVariant dataDuration(const Song &) const;
    QVariant dataTrack(const Song &) const;

public slots:
    void update();

private:
    Playlist &playlist_;
};
