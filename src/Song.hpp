#pragma once

#include <QString>
#include <chrono>

struct AlbumInfo
{
    int discNumber;
    int discTotal;
    int trackNumber;
    int trackTotal;
};

struct Song
{
    AlbumInfo albumInfo;
    QString path;
    QString name;
    QString artist;
    QString album;
    std::chrono::seconds duration;
};
