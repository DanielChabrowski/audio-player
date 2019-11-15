#include "MainWindow.hpp"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QMediaPlayer>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QTime>
#include <QtGlobal>

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
            catch(const std::runtime_error &)
            {
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
, settings_{ std::make_unique<QSettings>("OpenSource", "Foobar3000") }
, playlist{ std::make_unique<Playlist>() }
{
    ui.setupUi(this);

    playlist->name = "Default";

    {
        QTime startTime = QTime::currentTime();
        loadPlaylistFromDir(
        QStandardPaths::standardLocations(QStandardPaths::StandardLocation::MusicLocation).at(0), *playlist);
        auto elapsedTime = startTime.msecsTo(QTime::currentTime());
        qDebug() << "Loaded playlist in: " << elapsedTime << "ms";
    }

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

    auto *playlistWidget =
    new PlaylistWidget([this](int index) { playMediaFromCurrentPlaylist(index); });

    auto *playlistModel = new PlaylistModel{ *playlist };
    playlistWidget->setModel(playlistModel);

    auto *playlistHeader = new PlaylistHeader{ playlistWidget };
    playlistWidget->setHeader(playlistHeader);

    ui.playlist->addTab(playlistWidget, "Default");

    mediaPlayer_ = std::make_unique<QMediaPlayer>(this);

    {
        // Playback
        connect(mediaPlayer_.get(), &QMediaPlayer::mediaStatusChanged,
                [this](const QMediaPlayer::MediaStatus status) {
                    using Status = QMediaPlayer::MediaStatus;
                    if(Status::EndOfMedia == status or Status::InvalidMedia == status)
                    {
                        qDebug() << "QMediaPlayer status changed to " << status;
                        onMediaFinish();
                    }
                });
    }

    {
        // Volume settings
        constexpr auto volumeConfigKey{ "player/volume" };
        constexpr auto defaultVolume{ 30 };
        constexpr auto minVolume{ 0 };
        constexpr auto maxVolume{ 100 };

        const auto volume =
        qBound(minVolume, settings_->value(volumeConfigKey, defaultVolume).toInt(), maxVolume);
        ui.volumeSlider->setMaximum(maxVolume);
        ui.volumeSlider->setValue(volume);

        connect(ui.volumeSlider, &QSlider::valueChanged, [this](int volume) {
            this->mediaPlayer_->setVolume(volume);
            this->settings_->setValue(volumeConfigKey, volume);
            qDebug() << "Volume set to " << volume;
        });

        mediaPlayer_->setVolume(volume);
    }

    setupSeekbar();
    connectMediaPlayerToSeekbar();

    ui.menuLayout->addWidget(bar);

    auto *playButton = new QPushButton(this);
    playButton->setFlat(true);
    playButton->setIcon(QPixmap(":/play.png"));

    ui.buttonsLayout->addWidget(playButton);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupSeekbar()
{
    connect(ui.seekbar, &QSlider::sliderPressed, [this]() {
        disconnect(mediaPlayer_.get(), &QMediaPlayer::positionChanged, ui.seekbar, nullptr);
    });

    connect(ui.seekbar, &QSlider::sliderReleased, [this]() {
        this->mediaPlayer_->setPosition(this->ui.seekbar->value());
        connectMediaPlayerToSeekbar();
    });
}

void MainWindow::connectMediaPlayerToSeekbar()
{
    connect(mediaPlayer_.get(), &QMediaPlayer::positionChanged, ui.seekbar, &QSlider::setValue);
}

void MainWindow::playMediaFromCurrentPlaylist(int index)
{
    const auto &song = this->playlist->songs.at(index);

    this->playlist->currentSongIndex = index;
    this->mediaPlayer_->setMedia(QUrl::fromLocalFile(song.path));
    this->mediaPlayer_->play();

    qDebug() << "Playing media: " << song.path;

    this->ui.seekbar->setValue(0);
    this->ui.seekbar->setMaximum(song.duration.count() * 1000);

    this->ui.playlist->update();
}

void MainWindow::onMediaFinish()
{
    const auto songsCount = static_cast<int>(this->playlist->songs.size());
    const auto currentSongIndex = this->playlist->currentSongIndex;

    auto nextSongIndex = currentSongIndex + 1;
    if(nextSongIndex > songsCount - 1)
    {
        nextSongIndex = 0;
    }

    playMediaFromCurrentPlaylist(qBound(0, nextSongIndex, songsCount - 1));
}
