#pragma once

#include <chrono>
#include <string>

struct AlbumInfo
{
    int discNumber;
    int discTotal;
    int trackNumber;
    int trackTotal;
};

struct Song
{
    std::string path;
    std::string name;
    std::string artist;
    std::string album;
    std::chrono::seconds duration;
    AlbumInfo albumInfo;
};
