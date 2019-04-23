#include "MainWindow.hpp"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QMediaPlayer>
#include <QMenuBar>
#include <QPushButton>
#include <QStandardPaths>

#include "Playlist.hpp"
#include "PlaylistHeader.hpp"
#include "PlaylistModel.hpp"
#include "PlaylistWidget.hpp"
#include "TaglibAudioPropertyReader.hpp"

void loadPlaylistFromDir(QDir dir, Playlist &playlist)
{
    for(const auto &entry : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if(entry.isFile())
        {
            try
            {
                playlist.songs.push_back(TaglibAudioPropertyReader{}.loadSong(entry.absoluteFilePath()));
            }
            catch(const std::runtime_error &e)
            {
                qDebug() << e.what();
            }
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
    loadPlaylistFromDir(
    QStandardPaths::standardLocations(QStandardPaths::StandardLocation::MusicLocation).at(0), *playlist);

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
        this->mediaPlayer_->setMedia(QUrl::fromLocalFile(song.path));
        this->mediaPlayer_->play();

        this->ui.seekbar->setValue(0);
        this->ui.seekbar->setMaximum(song.duration.count() * 1000);

        qDebug() << "Callback with index: " << index << song.path;
    });

    auto *playlistModel = new PlaylistModel{ *playlist };
    playlistWidget->setModel(playlistModel);

    auto *playlistHeader = new PlaylistHeader{ playlistWidget };
    playlistWidget->setHeader(playlistHeader);

    ui.playlist->addTab(playlistWidget, "Default");

    mediaPlayer_ = std::make_unique<QMediaPlayer>(this);

    ui.seekbar->setMaximum(1532 * 1000);

    connect(ui.volumeSlider, &QSlider::valueChanged, &(*mediaPlayer_), &QMediaPlayer::setVolume);

    connect(ui.seekbar, &QSlider::sliderReleased,
            [this]() { this->mediaPlayer_->setPosition(this->ui.seekbar->value()); });
    connect(&(*mediaPlayer_), &QMediaPlayer::positionChanged, ui.seekbar, &QSlider::setValue);

    ui.menuLayout->addWidget(bar);

    auto *playButton = new QPushButton(this);
    playButton->setFlat(true);
    playButton->setIcon(QPixmap(":/resources/play.png"));

    ui.buttonsLayout->addWidget(playButton);
}

MainWindow::~MainWindow() = default;
