#pragma once

#include <QTreeView>

#include <functional>

class Playlist;

class PlaylistWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit PlaylistWidget(Playlist &, std::function<void(int)> itemSelectedCallback, QWidget * = nullptr);

    void keyPressEvent(QKeyEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;
    void currentChanged(const QModelIndex &, const QModelIndex &) override;
    void selectionChanged(const QItemSelection &, const QItemSelection &) override;

private:
    void enablePlayTrackShortcut();
    void enableDeleteTrackShortcut();

private:
    Playlist &playlist_;
    std::function<void(int)> itemSelectedCallback_;
};
