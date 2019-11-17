#include "PlaylistHeader.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>

#include "PlaylistWidget.hpp"

PlaylistHeader::PlaylistHeader(PlaylistWidget *playlistWidget)
: QHeaderView{ Qt::Horizontal, playlistWidget }
{
    setSectionsMovable(true);
}

void PlaylistHeader::contextMenuEvent(QContextMenuEvent *e)
{
    auto menu = new QMenu(this);
    menu->addAction(new QAction("Dummy action", this));
    menu->popup(e->globalPos());
}
