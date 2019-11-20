#pragma once

#include <QTreeView>

#include <functional>

class PlaylistWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit PlaylistWidget(std::function<void(int)> itemSelectedCallback, QWidget * = nullptr);

    void keyPressEvent(QKeyEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;
    void currentChanged(const QModelIndex &, const QModelIndex &) override;
    void selectionChanged(const QItemSelection &, const QItemSelection &) override;

private:
    std::function<void(int)> itemSelectedCallback_;
};
