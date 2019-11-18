#include "PlaylistLoader.hpp"

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include <stdexcept>
#include <vector>

Playlist PlaylistLoader::loadFromFile(const QString &filename)
{
    QFile playlistFile{ filename };
    if(not playlistFile.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error("Playlist file not found");
    }

    std::vector<QString> audioFilePaths;

    QString line;
    QTextStream ss{ &playlistFile };
    while(ss.readLineInto(&line))
    {
        // TODO: Does isEmpty return false on " " ?
        if(not line.isEmpty())
        {
            audioFilePaths.emplace_back(line);
        }
    }

    QFileInfo playlistFileInfo{ playlistFile };
    constexpr auto currentSongIndex{ -1 };
    return {
        playlistFileInfo.completeBaseName(),
        playlistFileInfo.absoluteFilePath(),
        currentSongIndex,
        std::move(audioFilePaths),
    };
}
