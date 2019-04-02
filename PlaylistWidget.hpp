#pragma once

#include <QTreeView>

class PlaylistWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit PlaylistWidget(QWidget * = nullptr);

    void keyPressEvent(QKeyEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void dropEvent(QDropEvent *) override;
    void currentChanged(const QModelIndex &, const QModelIndex &) override;
    void selectionChanged(const QItemSelection &, const QItemSelection &) override;
};
