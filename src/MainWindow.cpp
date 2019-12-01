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

#include "ConfigurationKeys.hpp"
#include "PlaylistHeader.hpp"
#include "PlaylistLoader.hpp"
#include "PlaylistManager.hpp"
#include "PlaylistModel.hpp"
#include "PlaylistWidget.hpp"

MainWindow::MainWindow(QSettings &settings, PlaylistManager &playlistManager)
: QWidget{ nullptr }
, settings_{ settings }
, playlistManager_{ playlistManager }
{
    ui.setupUi(this);

    setTheme(":/themes/gray-orange.css");

    {
        // Restoring window size and position
        restoreGeometry(settings_.value(geometryConfigKey).toByteArray());
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
    settings_.setValue(geometryConfigKey, saveGeometry());
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

    fileMenu->addSeparator();

    auto *newPlaylistAction = fileMenu->addAction("Add new playlist", [this]() {
        const auto index = playlistManager_.create("New playlist");
        if(not index)
        {
            // TODO: Error handling
            return;
        }
        setupPlaylistWidget(playlistManager_.get(*index));
    });
    newPlaylistAction->setShortcut(QKeySequence(QKeySequence::New));

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

    const auto volume = qBound(minVolume, settings_.value(volumeConfigKey, defaultVolume).toInt(), maxVolume);
    ui.volumeSlider->setMaximum(maxVolume);
    ui.volumeSlider->setValue(volume);

    mediaPlayer_->setVolume(volume);

    connect(ui.volumeSlider, &QSlider::valueChanged, [this](int volume) {
        this->mediaPlayer_->setVolume(volume);
        this->settings_.setValue(volumeConfigKey, volume);
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

void MainWindow::setupPlaylistWidget(Playlist &playlist)
{
    ui.playlist->setFocusPolicy(Qt::NoFocus);

    const auto playlistId = playlist.getPlaylistId();
    auto playlistWidget = std::make_unique<PlaylistWidget>(playlist, [this, playlistId](int index) {
        playMediaFromPlaylist(playlistId, index);
    });
    auto playlistModel = std::make_unique<PlaylistModel>(playlist, playlistWidget.get());
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

    ui.playlist->addTab(playlistWidget.release(), playlist.getName());
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

    const auto removePlaylist = new QShortcut(QKeySequence::Close, this);
    connect(removePlaylist, &QShortcut::activated, this, &MainWindow::removeCurrentPlaylist);
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

void MainWindow::playMediaFromPlaylist(std::uint32_t playlistId, std::size_t index)
{
    auto &playlist = playlistManager_.get(playlistId);

    const auto *track = playlist.getTrack(index);
    qDebug() << "Playing media: " << track->path;

    this->mediaPlayer_->setMedia(QUrl::fromUserInput(track->path));
    this->mediaPlayer_->play();

    this->ui.seekbar->setValue(0);

    const auto audioDuration = track->audioMetaData ? track->audioMetaData->duration.count() : 0;
    this->ui.seekbar->setMaximum(audioDuration * 1000);

    playlist.setCurrentTrackIndex(index);
    this->ui.playlist->update();

    disconnect(mediaPlayer_.get(), &QMediaPlayer::mediaStatusChanged, nullptr, nullptr);
    connect(mediaPlayer_.get(), &QMediaPlayer::mediaStatusChanged,
            [this, playlistId](const QMediaPlayer::MediaStatus status) {
                using Status = QMediaPlayer::MediaStatus;
                if(Status::EndOfMedia == status or Status::InvalidMedia == status)
                {
                    qDebug() << "QMediaPlayer status changed to " << status;
                    onMediaFinish(playlistId);
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

void MainWindow::removeCurrentPlaylist()
{
    const auto *currentWidget = ui.playlist->currentWidget();
    if(not currentWidget)
    {
        return;
    }

    const auto *currentPlaylistWidget = qobject_cast<const PlaylistWidget *>(currentWidget);
    const auto &currentPlaylist = currentPlaylistWidget->getPlaylist();
    Q_UNUSED(currentPlaylist);
}

void MainWindow::onMediaFinish(std::uint32_t playlistId)
{
    const auto &playlist = playlistManager_.get(playlistId);
    const auto nextTrackIndex = playlist.getNextTrackIndex();
    if(nextTrackIndex)
    {
        playMediaFromPlaylist(playlistId, *nextTrackIndex);
    }
}

void MainWindow::loadPlaylists()
{
    auto &playlists = playlistManager_.getAll();
    for(auto &playlist : playlists)
    {
        setupPlaylistWidget(playlist.second);
    }
}
