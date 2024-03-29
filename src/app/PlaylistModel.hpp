#pragma once

#include <QAbstractListModel>
#include <QStringList>

#include <optional>

enum PlaylistColumn
{
    NOW_PLAYING,
    ARTIST_ALBUM,
    TRACK,
    TITLE,
    DURATION
};

class Playlist;
struct AudioMetaData;

class PlaylistModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    PlaylistModel(Playlist &, QObject * = nullptr);

    Playlist &getPlaylist()
    {
        return playlist_;
    }

protected:
    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation, int = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &, int = Qt::DisplayRole) const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    bool removeRows(int, int, const QModelIndex &) override;

    Qt::ItemFlags flags(const QModelIndex &) const override;
    Qt::DropActions supportedDropActions() const override;

    QMimeData *mimeData(const QModelIndexList &) const override;
    bool canDropMimeData(const QMimeData *, Qt::DropAction, int row, int column, const QModelIndex &) const override;
    bool dropMimeData(const QMimeData *, Qt::DropAction, int row, int column, const QModelIndex &) override;

private:
    QVariant roleAlignment(int column) const;
    QVariant dataTitle(const QString &filepath, const std::optional<AudioMetaData> &) const;
    QVariant dataArtistAlbum(const std::optional<AudioMetaData> &) const;
    QVariant dataDuration(const std::optional<AudioMetaData> &) const;
    QVariant dataTrack(const std::optional<AudioMetaData> &) const;

public slots:
    void onDuplicateRemoveRequest();
    void onInsertRequest(QStringList);

private:
    Playlist &playlist_;
    std::size_t fetched_{ 0 };
};
