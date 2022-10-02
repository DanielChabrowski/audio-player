#pragma once

#include <QTreeView>

class Playlist;

class PlaylistWidget final : public QTreeView
{
    Q_OBJECT

public:
    PlaylistWidget(Playlist &, QWidget * = nullptr);

    const Playlist &getPlaylist() const;

    void mouseDoubleClickEvent(QMouseEvent *) override;
    void timerEvent(QTimerEvent *) override;

signals:
    void itemPicked(int index);

private:
    void enablePlayTrackShortcut();
    void enableDeleteTrackShortcut();

private:
    Playlist &playlist_;
};
