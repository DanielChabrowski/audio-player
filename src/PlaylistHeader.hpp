#pragma once

#include <QHeaderView>

class PlaylistWidget;

class PlaylistHeader : public QHeaderView
{
public:
    PlaylistHeader(PlaylistWidget *);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;
};
