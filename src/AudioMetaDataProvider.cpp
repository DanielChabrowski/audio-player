#include "AudioMetaDataProvider.hpp"

#include "taglib/fileref.h"
#include "taglib/tag.h"
#include "taglib/tpropertymap.h"

AudioMetaDataProvider::~AudioMetaDataProvider() = default;

AudioMetaData AudioMetaDataProvider::getMetaData(const QString &filepath)
{
    TagLib::FileRef ref{ filepath.toStdString().c_str() };
    if(ref.isNull() or not ref.tag())
    {
        throw std::runtime_error("Not an audio file");
    }

    std::chrono::seconds duration{ 0 };
    const auto *audioProperties = ref.audioProperties();
    if(audioProperties)
    {
        duration = std::chrono::seconds{ audioProperties->length() };
    }

    const auto *tags = ref.tag();
    AudioAlbumMetaData albumData{ QString::fromStdWString(tags->album().toWString()), -1, -1, -1, -1 };

    const auto properties = ref.file()->properties();
    for(const auto &property : properties)
    {
        for(const auto &propValue : property.second)
        {
            const auto &propertyName = property.first.to8Bit(true);
            if(u8"DISCNUMBER" == propertyName)
            {
                albumData.discNumber = propValue.toInt();
            }
            else if(u8"DISCTOTAL" == propertyName)
            {
                albumData.discTotal = propValue.toInt();
            }
            else if(u8"TRACKNUMBER" == propertyName)
            {
                albumData.trackNumber = propValue.toInt();
            }
            else if(u8"TRACKTOTAL" == propertyName)
            {
                albumData.trackTotal = propValue.toInt();
            }
        }
    }

    return {
        QString::fromStdWString(tags->title().toWString()),
        QString::fromStdWString(tags->artist().toWString()),
        std::move(albumData),
        duration,
    };
}
