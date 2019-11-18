#pragma once

#include <QString>

#include <chrono>

struct AudioAlbumMetaData
{
    QString name;
    int discNumber;
    int discTotal;
    int trackNumber;
    int trackTotal;
};

struct AudioMetaData
{
    QString title;
    QString artist;
    AudioAlbumMetaData albumData;
    std::chrono::milliseconds duration;
};
