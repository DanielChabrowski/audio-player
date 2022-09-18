#pragma once

#include <QHeaderView>

class PlaylistWidget;

class PlaylistHeader final : public QHeaderView
{
public:
    explicit PlaylistHeader(PlaylistWidget *);
};
