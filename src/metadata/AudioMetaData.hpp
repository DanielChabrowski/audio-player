#pragma once

#include <QString>

#include <chrono>

struct AudioAlbumMetaData
{
    QString name;
    int discNumber;
    int trackNumber;
};

struct AudioMetaData
{
    QString title;
    QString artist;
    AudioAlbumMetaData albumData;
    std::chrono::seconds duration;
};
