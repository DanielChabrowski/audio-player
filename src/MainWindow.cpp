#include "MainWindow.hpp"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileSystemModel>
#include <QLabel>
#include <QMediaPlayer>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <QTime>
#include <QToolTip>
#include <QtGlobal>

#include "AudioMetaDataProvider.hpp"
#include "ConfigurationKeys.hpp"
#include "PlaylistHeader.hpp"
#include "PlaylistLoader.hpp"
#include "PlaylistModel.hpp"
#include "PlaylistWidget.hpp"

MainWindow::MainWindow(QWidget *parent)
: QWidget{ parent }
, settings_{ std::make_unique<QSettings>("OpenSource", "Foobar3000") }
, audioMetaDataProvider{ std::make_unique<AudioMetaDataProvider>() }
{
    ui.setupUi(this);

    setTheme(":/themes/gray-orange.css");

    {
        // Restoring window size and position
        restoreGeometry(settings_->value(geometryConfigKey).toByteArray());
    }

    setupMediaPlayer();

    setupMenu();
    setupPlaybackControlButtons();
    setupVolumeControl();
    setupSeekbar();
    setupAlbumsBrowser();

    connectMediaPlayerToSeekbar();

    setupGlobalShortcuts();

    {
        QTime startTime = QTime::currentTime();
        loadPlaylists();
        auto elapsedTime = startTime.msecsTo(QTime::currentTime());
        qDebug() << "Loaded playlist in: " << elapsedTime << "ms";
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings_->setValue(geometryConfigKey, saveGeometry());
    QWidget::closeEvent(event);
}

void MainWindow::setupMenu()
{
    QMenuBar *bar = new QMenuBar(this);
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

    ui.menuLayout->addWidget(bar);
}

void MainWindow::setupVolumeControl()
{
    constexpr auto defaultVolume{ 30 };
    constexpr auto minVolume{ 0 };
    constexpr auto maxVolume{ 100 };

    const auto volume = qBound(minVolume, settings_->value(volumeConfigKey, defaultVolume).toInt(), maxVolume);
    ui.volumeSlider->setMaximum(maxVolume);
    ui.volumeSlider->setValue(volume);

    mediaPlayer_->setVolume(volume);

    connect(ui.volumeSlider, &QSlider::valueChanged, [this](int volume) {
        this->mediaPlayer_->setVolume(volume);
        this->settings_->setValue(volumeConfigKey, volume);
        qDebug() << "Volume set to " << volume;
    });
}

void MainWindow::setupSeekbar()
{
    connect(ui.seekbar, &QSlider::sliderPressed, [this]() {
        disconnect(mediaPlayer_.get(), &QMediaPlayer::positionChanged, ui.seekbar, nullptr);
    });

    connect(ui.seekbar, &QSlider::sliderReleased, [this]() {
        this->mediaPlayer_->setPosition(this->ui.seekbar->value());
        connectMediaPlayerToSeekbar();
    });

    connect(ui.seekbar, &QSlider::sliderMoved, [this](int value) {
        const auto currentTime = QTime(0, 0, 0, 0).addMSecs(value);
        const auto verticalPos = QWidget::mapToGlobal(ui.seekbar->geometry().bottomLeft());

        QToolTip::showText(QPoint{ QCursor::pos().x(), verticalPos.y() },
                           currentTime.toString("H:mm:ss"));
    });
}

void MainWindow::setupPlaybackControlButtons()
{
    const auto createButtonFunc = [this](QString filename, std::function<void()> onReleaseEvent) {
        auto *button = new QPushButton(this);
        button->setFlat(true);
        button->setMaximumSize(24, 24);
        button->setIcon(QPixmap(filename));
        connect(button, &QPushButton::released, std::move(onReleaseEvent));

        ui.buttonsLayout->addWidget(button);
    };

    ui.buttonsLayout->QLayout::setSpacing(0);

    createButtonFunc(":/icons/play.png", [this]() { mediaPlayer_->play(); });
    createButtonFunc(":/icons/pause.png", [this]() { mediaPlayer_->pause(); });
    createButtonFunc(":/icons/stop.png", [this]() { mediaPlayer_->stop(); });
}

void MainWindow::setupAlbumsBrowser()
{
    const auto albumLocation =
    QStandardPaths::standardLocations(QStandardPaths::StandardLocation::MusicLocation).at(0);

    qDebug() << "Loading album view for:" << albumLocation;

    const auto dirModel = new QFileSystemModel(this);
    dirModel->setReadOnly(true);
    dirModel->setRootPath(albumLocation);

    const auto albumsView = new QTreeView(this);
    albumsView->setHeaderHidden(true);
    albumsView->setDragEnabled(true);
    albumsView->setModel(dirModel);

    // Hide all columns except the first one
    const auto columnCount = dirModel->columnCount();
    for(int i = 1; i < columnCount; ++i)
    {
        albumsView->setColumnHidden(i, true);
    }

    albumsView->setRootIndex(dirModel->index(albumLocation));

    ui.albums->addTab(albumsView, "Albums");
}

void MainWindow::setupPlaylistWidget(Playlist *playlist)
{
    ui.playlist->setFocusPolicy(Qt::NoFocus);

    auto playlistWidget = std::make_unique<PlaylistWidget>(*playlist, [this, playlist](int index) {
        playMediaFromCurrentPlaylist(playlist, index);
    });
    auto playlistModel = std::make_unique<PlaylistModel>(*playlist, playlistWidget.get());
    auto playlistHeader = std::make_unique<PlaylistHeader>(playlistWidget.get());

    playlistWidget->setModel(playlistModel.release());
    playlistWidget->setHeader(playlistHeader.release());

    playlistWidget->setColumnWidth(0, 10);
    playlistWidget->header()->setSectionResizeMode(PlaylistColumn::NOW_PLAYING, QHeaderView::ResizeMode::Fixed);
    playlistWidget->header()->setSectionResizeMode(PlaylistColumn::ARTIST_ALBUM, QHeaderView::ResizeMode::Stretch);
    playlistWidget->header()->setSectionResizeMode(PlaylistColumn::TRACK, QHeaderView::ResizeMode::ResizeToContents);
    playlistWidget->header()->setSectionResizeMode(PlaylistColumn::TITLE, QHeaderView::ResizeMode::Stretch);
    playlistWidget->header()->setSectionResizeMode(PlaylistColumn::DURATION,
                                                   QHeaderView::ResizeMode::ResizeToContents);

    ui.playlist->addTab(playlistWidget.release(), playlist->name);
}

void MainWindow::setupMediaPlayer()
{
    mediaPlayer_ = std::make_unique<QMediaPlayer>(this);

    connect(mediaPlayer_.get(), QOverload<const QString &, const QVariant &>::of(&QMediaObject::metaDataChanged),
            [](const QString &key, const QVariant &value) { qDebug() << key << value; });
}

void MainWindow::setupGlobalShortcuts()
{
    const auto togglePlayPause = new QShortcut(Qt::Key_Space, this);
    connect(togglePlayPause, &QShortcut::activated, this, &MainWindow::togglePlayPause);
}

void MainWindow::setTheme(const QString &filename)
{
    QFile file{ filename };
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Could not open a theme file for reading:";
        return;
    }

    QTextStream in(&file);
    const QString stylesheet = in.readAll();

    setStyleSheet(stylesheet);
}

void MainWindow::connectMediaPlayerToSeekbar()
{
    connect(mediaPlayer_.get(), &QMediaPlayer::positionChanged, ui.seekbar, &QSlider::setValue);
}

void MainWindow::playMediaFromCurrentPlaylist(Playlist *playlist, int index)
{
    const auto &track = playlist->tracks.at(index);

    playlist->currentSongIndex = index;

    this->mediaPlayer_->setMedia(QUrl::fromUserInput(track.path));
    this->mediaPlayer_->play();

    qDebug() << "Playing media: " << track.path;

    this->ui.seekbar->setValue(0);

    const auto audioDuration = track.audioMetaData ? track.audioMetaData->duration.count() : 0;
    this->ui.seekbar->setMaximum(audioDuration * 1000);

    this->ui.playlist->update();

    disconnect(mediaPlayer_.get(), &QMediaPlayer::mediaStatusChanged, nullptr, nullptr);
    connect(mediaPlayer_.get(), &QMediaPlayer::mediaStatusChanged,
            [this, playlist](const QMediaPlayer::MediaStatus status) {
                using Status = QMediaPlayer::MediaStatus;
                if(Status::EndOfMedia == status or Status::InvalidMedia == status)
                {
                    qDebug() << "QMediaPlayer status changed to " << status;
                    onMediaFinish(playlist);
                }
            });
}

void MainWindow::togglePlayPause()
{
    if(QMediaPlayer::State::PlayingState == mediaPlayer_->state())
    {
        mediaPlayer_->pause();
    }
    else
    {
        mediaPlayer_->play();
    }
}

void MainWindow::onMediaFinish(Playlist *playlist)
{
    const auto playlistSize = static_cast<int>(playlist->tracks.size());
    const auto currentSongIndex = playlist->currentSongIndex;

    auto nextSongIndex = currentSongIndex + 1;
    if(nextSongIndex > playlistSize - 1)
    {
        nextSongIndex = 0;
    }

    playMediaFromCurrentPlaylist(playlist, qBound(0, nextSongIndex, playlistSize - 1));
}

void MainWindow::loadPlaylists()
{
    static int lastPlaylistIndex{ 0 };

    // TODO: Move to playlist provider
    const auto playlistDirPath =
    QStandardPaths::standardLocations(QStandardPaths::StandardLocation::ConfigLocation).at(0) +
    "/playlists";

    QDir playlistDir{ playlistDirPath };
    if(not playlistDir.exists() and not playlistDir.mkpath(playlistDirPath))
    {
        qWarning() << "Could not create playlist directory" << playlistDirPath;
        return;
    }

    PlaylistLoader playlistLoader{ *audioMetaDataProvider };
    for(const auto &entry : playlistDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
    {
        qDebug() << "Loading playlist" << entry.absoluteFilePath();
        const auto playlist = playlistLoader.loadFromFile(entry.absoluteFilePath());
        playlists_.emplace(lastPlaylistIndex++, std::move(playlist));

        setupPlaylistWidget(&playlists_.at(lastPlaylistIndex - 1));
    }

    if(playlists_.size() == 0)
    {
        // Add default playlist widget
        constexpr auto defaultPlaylistName{ "Default" };
        const auto playlist =
        Playlist{ defaultPlaylistName, playlistDirPath + "/" + defaultPlaylistName, *audioMetaDataProvider };
        playlists_.emplace(lastPlaylistIndex++, std::move(playlist));
        setupPlaylistWidget(&playlists_.at(lastPlaylistIndex - 1));
    }
}
