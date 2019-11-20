#include "PlaylistLoader.hpp"

#include "IAudioMetaDataProvider.hpp"

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include <stdexcept>
#include <vector>

PlaylistLoader::PlaylistLoader(IAudioMetaDataProvider &audioMetaDataProvider)
: audioMetaDataProvider_{ audioMetaDataProvider }
{
}

Playlist PlaylistLoader::loadFromFile(const QString &filename)
{
    QFile playlistFile{ filename };
    if(not playlistFile.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error("Playlist file not found");
    }

    std::vector<QUrl> audioFilePaths;

    QString line;
    QTextStream ss{ &playlistFile };
    while(ss.readLineInto(&line))
    {
        // TODO: Does isEmpty return false on " " ?
        if(not line.isEmpty())
        {
            audioFilePaths.emplace_back(QUrl{ line });
        }
    }

    QFileInfo playlistFileInfo{ playlistFile };
    return {
        playlistFileInfo.completeBaseName(),
        playlistFileInfo.absoluteFilePath(),
        std::move(audioFilePaths),
        audioMetaDataProvider_,
    };
}
