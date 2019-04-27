#include "TaglibAudioPropertyReader.hpp"

#include "taglib/fileref.h"
#include "taglib/tag.h"
#include "taglib/tpropertymap.h"

Song TaglibAudioPropertyReader::loadSong(const QString &path)
{
    TagLib::FileRef ref{ path.toStdString().c_str() };
    if(ref.isNull() or not ref.tag())
    {
        throw std::runtime_error("Not an audio file");
    }

    std::int64_t length{ 0 };
    const auto *audioProperties = ref.audioProperties();
    if(audioProperties)
    {
        length = audioProperties->length();
    }

    AlbumInfo albumInfo{ -1, -1, -1, -1 };

    const auto properties = ref.file()->properties();
    for(const auto &property : properties)
    {
        for(const auto &propValue : property.second)
        {
            const auto &propertyName = property.first.to8Bit(true);
            if(u8"DISCNUMBER" == propertyName)
            {
                albumInfo.discNumber = propValue.toInt();
            }
            else if(u8"DISCTOTAL" == propertyName)
            {
                albumInfo.discTotal = propValue.toInt();
            }
            else if(u8"TRACKNUMBER" == propertyName)
            {
                albumInfo.trackNumber = propValue.toInt();
            }
            else if(u8"TRACKTOTAL" == propertyName)
            {
                albumInfo.trackTotal = propValue.toInt();
            }
        }
    }

    const auto *tags = ref.tag();
    return {
        std::move(albumInfo),
        path,
        QString::fromStdWString(tags->title().toWString()),
        QString::fromStdWString(tags->artist().toWString()),
        QString::fromStdWString(tags->album().toWString()),
        std::chrono::seconds{ length },
    };
}
