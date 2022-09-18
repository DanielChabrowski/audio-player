#pragma once

#include <QSortFilterProxyModel>

class PlaylistFilterModel final : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit PlaylistFilterModel(QObject *parent = nullptr);

    void setFilterQuery(QString query);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &) const override;

private:
    QString query;
};
