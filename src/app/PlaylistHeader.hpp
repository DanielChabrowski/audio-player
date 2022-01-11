#pragma once

#include <QHeaderView>

class PlaylistWidget;

class PlaylistHeader : public QHeaderView
{
public:
    explicit PlaylistHeader(PlaylistWidget *);
};
