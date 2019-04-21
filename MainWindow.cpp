#include "MainWindow.hpp"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QMediaPlayer>
#include <QMenuBar>

#include "taglib/fileref.h"
#include "taglib/tag.h"
#include "taglib/tpropertymap.h"

#include "Playlist.hpp"
#include "PlaylistModel.hpp"
#include "PlaylistWidget.hpp"

void loadFileToPlaylist(std::string path, Playlist &playlist)
{
    TagLib::FileRef ref{ path.c_str() };
    if(ref.isNull() or not ref.tag())
    {
        qDebug() << "Skipping file without tags: " << path.c_str();
        return;
    }

    std::int64_t length{ 0 };
    const auto *audioProperties = ref.audioProperties();
    if(audioProperties)
    {
        length = audioProperties->length();
        qDebug() << path.c_str() << length;
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

    playlist.songs.push_back(Song{
    path,
    QString::fromStdWString(tags->title().toWString()).toStdString(),
    QString::fromStdWString(tags->artist().toWString()).toStdString(),
    QString::fromStdWString(tags->album().toWString()).toStdString(),
    std::chrono::seconds{ length },
    std::move(albumInfo),
    });
}

void loadPlaylistFromDir(QDir dir, Playlist &playlist)
{
    for(const auto &entry : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if(entry.isFile())
        {
            const auto absolutePath = entry.absoluteFilePath().toStdString();
            loadFileToPlaylist(absolutePath, playlist);
        }
        else if(entry.isDir())
        {
            loadPlaylistFromDir(entry.absoluteFilePath(), playlist);
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
: QWidget{ parent }
, playlist{ std::make_unique<Playlist>() }
{
    ui.setupUi(this);

    playlist->name = "Default";
    loadPlaylistFromDir(QDir{ "/home/dantez/Music/" }, *playlist);

    QMenuBar *bar = new QMenuBar;
    auto *fileMenu = bar->addMenu(tr("File"));

    fileMenu->addAction("Open");
    fileMenu->addSeparator();
    fileMenu->addAction("Preferences");
    fileMenu->addSeparator();
    auto *exitAction = fileMenu->addAction("Exit", []() {
        constexpr int exitCode{ 0 };
        QApplication::exit(exitCode);
    });
    exitAction->setShortcut(QKeySequence(QKeySequence::Quit));

    bar->addMenu(tr("Edit"));
    bar->addMenu(tr("View"));
    bar->addMenu(tr("Playback"));
    bar->addMenu(tr("Library"));
    bar->addMenu(tr("Help"));

    ui.playlist->setFocusPolicy(Qt::NoFocus);

    auto *playlistWidget = new PlaylistWidget([this](int index) {
        const auto &song = this->playlist->songs.at(index);
        this->playlist->currentSongIndex = index;
        this->mediaPlayer_->setMedia(QUrl::fromLocalFile(song.path.c_str()));
        this->mediaPlayer_->play();

        this->ui.seekbar->setValue(0);
        this->ui.seekbar->setMaximum(song.duration.count() * 1000);

        qDebug() << "Callback with index: " << index << song.path.c_str();
    });

    auto *playlistModel = new PlaylistModel(*playlist);
    playlistWidget->setModel(playlistModel);
    ui.playlist->addTab(playlistWidget, "Default");

    mediaPlayer_ = std::make_unique<QMediaPlayer>(this);

    ui.seekbar->setMaximum(1532 * 1000);

    connect(ui.volumeSlider, &QSlider::valueChanged, &(*mediaPlayer_), &QMediaPlayer::setVolume);

    connect(ui.seekbar, &QSlider::sliderReleased,
            [this]() { this->mediaPlayer_->setPosition(this->ui.seekbar->value()); });
    connect(&(*mediaPlayer_), &QMediaPlayer::positionChanged, ui.seekbar, &QSlider::setValue);

    ui.menuLayout->addWidget(bar);
}

MainWindow::~MainWindow() = default;
