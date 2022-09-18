#pragma once

#include <QTreeView>

#include <functional>

class Playlist;

class PlaylistWidget final : public QTreeView
{
    Q_OBJECT

public:
    explicit PlaylistWidget(Playlist &, std::function<void(int)> itemSelectedCallback, QWidget * = nullptr);

    const Playlist &getPlaylist() const;

    void mouseDoubleClickEvent(QMouseEvent *) override;

private:
    void enablePlayTrackShortcut();
    void enableDeleteTrackShortcut();

private:
    Playlist &playlist_;
    std::function<void(int)> itemSelectedCallback_;
};
