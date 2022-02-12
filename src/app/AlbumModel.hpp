#pragma once

#include <QAbstractListModel>

class LibraryManager;

class AlbumModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AlbumModel(LibraryManager &libraryManager, QObject * = nullptr);

protected:
    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation, int = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &, int = Qt::DisplayRole) const override;

private:
    LibraryManager &libraryManager_;
};
