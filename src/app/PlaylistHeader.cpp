#include "PlaylistHeader.hpp"

#include "PlaylistWidget.hpp"

PlaylistHeader::PlaylistHeader(PlaylistWidget *playlistWidget)
: QHeaderView{ Qt::Horizontal, playlistWidget }
{
    setSectionsMovable(true);
}
