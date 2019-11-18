#pragma once

#include <QString>
#include <vector>

struct Playlist
{
    QString name;
    QString playlistPath;
    int currentSongIndex;
    std::vector<QString> audioFiles;
};
