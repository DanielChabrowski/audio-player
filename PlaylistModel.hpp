#pragma once

#include <QAbstractListModel>

struct Playlist;

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT

public:
    PlaylistModel(Playlist &, QObject * = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation, int = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &, int = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &) const override;

public slots:
    void update();

private:
    Playlist &playlist_;
};
