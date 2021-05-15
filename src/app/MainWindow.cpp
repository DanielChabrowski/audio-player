#include "MainWindow.hpp"

#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
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
#include "EscapableLineEdit.hpp"
#include "MultilineTabBar.hpp"
#include "PlaylistHeader.hpp"
#include "PlaylistManager.hpp"
#include "PlaylistModel.hpp"
#include "PlaylistWidget.hpp"

MainWindow::MainWindow(QSettings &settings, PlaylistManager &playlistManager)
: QWidget{ nullptr }
, settings_{ settings }
, playlistManager_{ playlistManager }
{
    setupWindow();

    setTheme(":/themes/gray-orange.css");

    {
        // Restoring window size and position
        restoreGeometry(settings_.value(config::geometryKey).toByteArray());
    }

    setupMediaPlayer();

    setupMenu();
    setupPlaybackControlButtons();
    setupVolumeControl();
    setupSeekbar();
    setupAlbumsBrowser();
    setupPlaylistWidget();

    connectMediaPlayerToSeekbar();

    setupGlobalShortcuts();

    {
        QElapsedTimer timer;
        timer.start();
        loadPlaylists();
        const auto elapsedTime = timer.elapsed();
        qDebug() << "Loaded playlists widgets in" << elapsedTime << "ms";
    }

    restoreLastPlaylist();
    enablePlaylistChangeTracking();

    QCoreApplication::instance()->installEventFilter(this);
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings_.setValue(config::geometryKey, saveGeometry());
    QWidget::closeEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(obj && obj->isWidgetType() && ui.playlistRenameWidget && ui.playlistRenameWidget != obj)
        {
            ui.playlistRenameWidget->deleteLater();
            ui.playlistRenameWidget = nullptr;
        }
    }

    return false;
}

void MainWindow::setupWindow()
{
    setWindowTitle("Foobar");
    setGeometry(0, 0, 1100, 470);

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    setSizePolicy(sizePolicy);

    auto *layout = new QGridLayout(this);
    layout->setMargin(9);
    layout->setSpacing(6);

    auto *topHLayout = new QHBoxLayout();

    ui.menuLayout = new QHBoxLayout();
    topHLayout->addLayout(ui.menuLayout);

    ui.buttonsLayout = new QHBoxLayout();
    topHLayout->addLayout(ui.buttonsLayout);

    ui.volumeSlider = new QSlider(this);
    {
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ui.volumeSlider->sizePolicy().hasHeightForWidth());
        ui.volumeSlider->setSizePolicy(sizePolicy);
        ui.volumeSlider->setOrientation(Qt::Horizontal);

        topHLayout->addWidget(ui.volumeSlider);
    }

    ui.seekbar = new QSlider(this);
    {
        ui.seekbar->setOrientation(Qt::Horizontal);

        topHLayout->addWidget(ui.seekbar);
    }

    layout->addLayout(topHLayout, 0, 5, 1, 1);

    auto *bottomHLayout = new QHBoxLayout();
    bottomHLayout->setSpacing(2);

    ui.albums = new QTreeView(this);
    {
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ui.albums->sizePolicy().hasHeightForWidth());
        ui.albums->setSizePolicy(sizePolicy);
        ui.albums->setMinimumSize(QSize(225, 0));

        const auto visibility = settings_.value(config::albumsVisibility, true);
        ui.albums->setVisible(visibility.toBool());

        bottomHLayout->addWidget(ui.albums);
    }

    ui.playlist = new MultilineTabWidget(this);
    {
        // ui.playlist->setMovable(true);
        ui.playlist->setCurrentIndex(-1);

        bottomHLayout->addWidget(ui.playlist);
    }

    layout->addLayout(bottomHLayout, 1, 5, 1, 1);
}

void MainWindow::setupMenu()
{
    auto *bar = new QMenuBar(this);
    auto *fileMenu = bar->addMenu("File");

    fileMenu->addAction("Open");

    auto *newPlaylistAction = fileMenu->addAction("Add new playlist", this, [this]() {
        const auto index = playlistManager_.create("New playlist");
        if(not index)
        {
            qCritical() << "Playlist could not be created";
            return;
        }

        const auto newTabIndex = setupPlaylistTab(*playlistManager_.get(*index));
        if(newTabIndex != -1)
        {
            ui.playlist->setCurrentIndex(newTabIndex);
        }
    });
    newPlaylistAction->setShortcut(QKeySequence(QKeySequence::New));

    fileMenu->addAction("Preferences");

    auto *exitAction = fileMenu->addAction("Exit", this, []() {
        constexpr int exitCode{ 0 };
        QApplication::exit(exitCode);
    });
    exitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    bar->addMenu("Edit");

    {
        auto *viewMenu = bar->addMenu("View");
        viewMenu->addAction("Toggle album view", this, [this]() {
            const auto toggle = !ui.albums->isVisible();
            ui.albums->setVisible(toggle);
            settings_.setValue(config::albumsVisibility, toggle);
        });
    }

    auto *playbackMenu = bar->addMenu("Playback");

    auto *actionGroup = new QActionGroup(this);

    const auto currentPlayMode = getCurrentPlayMode();

    auto addPlaybackAction = [this, actionGroup, currentPlayMode](const QString &label, PlayMode mode) {
        auto *action = actionGroup->addAction(label);
        action->setCheckable(true);
        action->setChecked(currentPlayMode == mode);

        connect(action, &QAction::toggled, [this, mode](bool checked) {
            if(checked)
            {
                this->settings_.setValue(config::playModeKey, static_cast<int>(mode));
            }
        });
    };

    addPlaybackAction("Repeat playlist", PlayMode::RepeatPlaylist);
    addPlaybackAction("Repeat track", PlayMode::RepeatTrack);
    addPlaybackAction("Random play", PlayMode::Random);

    playbackMenu->addActions(actionGroup->actions());

    bar->addMenu("Library");
    bar->addMenu("Help");

    ui.menuLayout->addWidget(bar);
}

void MainWindow::setupVolumeControl()
{
    constexpr auto defaultVolume{ 30 };
    constexpr auto minVolume{ 0 };
    constexpr auto maxVolume{ 100 };

    const auto volume = qBound(minVolume, settings_.value(config::volumeKey, defaultVolume).toInt(), maxVolume);
    ui.volumeSlider->setMaximum(maxVolume);
    ui.volumeSlider->setValue(volume);

    mediaPlayer_->setVolume(volume);

    connect(ui.volumeSlider, &QSlider::valueChanged, [this](int volume) {
        this->mediaPlayer_->setVolume(volume);
        this->settings_.setValue(config::volumeKey, volume);
        qDebug() << "Volume set to" << volume;
    });
}

void MainWindow::setupSeekbar()
{
    disableSeekbar();

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
    const auto createButtonFunc = [this](const QString &filename, std::function<void()> onReleaseEvent) {
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

QStringList getAudioFilesNameFilter()
{
    // Using only most popular extensions to keep the comparison somewhat faster
    // than using all available audio formats.
    return QStringList() << "*.flac"
                         << "*.ogg"
                         << "*.mp3"
                         << "*.wav"
                         << "*.m4a"
                         << "*.m4b"
                         << "*.webm"
                         << "*.mkv"
                         << "*.mp4";
}

void MainWindow::setupAlbumsBrowser()
{
    const auto albumLocation =
        QStandardPaths::standardLocations(QStandardPaths::StandardLocation::MusicLocation).at(0);

    qDebug() << "Loading album view for:" << albumLocation;

    const auto dirModel = new QFileSystemModel(this);
    dirModel->setReadOnly(true);
    dirModel->setNameFilterDisables(false); // hidden, not disabled
    dirModel->setNameFilters(getAudioFilesNameFilter());
    const auto rootIndex = dirModel->setRootPath(albumLocation);

    auto &albums = ui.albums;
    albums->setHeaderHidden(true);
    albums->setDragEnabled(true);
    albums->setModel(dirModel);
    albums->setUniformRowHeights(true);

    // Hide all columns except the first one
    const auto columnCount = dirModel->columnCount();
    for(int i = 1; i < columnCount; ++i)
    {
        albums->setColumnHidden(i, true);
    }

    albums->setRootIndex(rootIndex);
}

void MainWindow::setupPlaylistWidget()
{
    auto *tabbar = ui.playlist->tabBar();
    tabbar->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tabbar, &MultilineTabBar::tabDoubleClicked,
            [this](int tabIndex) { togglePlaylistRenameControl(tabIndex); });

    connect(tabbar, &QTabBar::customContextMenuRequested, [this, tabbar](const QPoint &point) {
        const auto tabIndex = tabbar->tabAt(point);
        const auto playlistId = getPlaylistIdByTabIndex(tabIndex);

        QMenu menu;
        menu.addAction("Rename playlist", this,
                       [this, tabIndex] { togglePlaylistRenameControl(tabIndex); });
        menu.addAction("Remove playlist", this, [this, playlistId = *playlistId, tabIndex]() {
            ui.playlist->removeTab(tabIndex);
            playlistManager_.removeById(playlistId);
        });

        menu.exec(tabbar->mapToGlobal(point));
    });
}

int MainWindow::setupPlaylistTab(Playlist &playlist)
{
    const auto playlistId = playlist.getPlaylistId();
    auto playlistWidget = std::make_unique<PlaylistWidget>(playlist, [this, playlistId](int index) {
        playMediaFromPlaylist(playlistId, index);
    });
    auto playlistModel = std::make_unique<PlaylistModel>(playlist, playlistWidget.get());
    auto playlistHeader = std::make_unique<PlaylistHeader>(playlistWidget.get());

    playlistWidget->setModel(playlistModel.release());
    playlistWidget->setHeader(playlistHeader.release());

    // Hide first column containing tree-view decorations
    playlistWidget->setColumnHidden(PlaylistColumn::DISABLED, true);

    // Set fixed width for NOW_PLAYING marker column
    playlistWidget->setColumnWidth(PlaylistColumn::NOW_PLAYING, 10);

    auto *header = playlistWidget->header();
    header->setSectionResizeMode(PlaylistColumn::NOW_PLAYING, QHeaderView::ResizeMode::Fixed);
    header->setSectionResizeMode(PlaylistColumn::ARTIST_ALBUM, QHeaderView::ResizeMode::Stretch);
    header->setSectionResizeMode(PlaylistColumn::TRACK, QHeaderView::ResizeMode::ResizeToContents);
    header->setSectionResizeMode(PlaylistColumn::TITLE, QHeaderView::ResizeMode::Stretch);
    header->setSectionResizeMode(PlaylistColumn::DURATION, QHeaderView::ResizeMode::ResizeToContents);

    return ui.playlist->addTab(playlistWidget.release(), playlist.getName());
}

void MainWindow::setupMediaPlayer()
{
    mediaPlayer_ = std::make_unique<QMediaPlayer>(this);

    constexpr bool defaultDebugAudioMetadata{ false };
    const bool debugAudioMetadata =
        settings_.value(config::debugAudioMetadataKey, defaultDebugAudioMetadata).toBool();

    if(debugAudioMetadata)
    {
        connect(mediaPlayer_.get(),
                QOverload<const QString &, const QVariant &>::of(&QMediaObject::metaDataChanged),
                [](const QString &key, const QVariant &value) {
                    qDebug() << "Metadata changed" << key << value;
                });
    }
}

void MainWindow::setupGlobalShortcuts()
{
    const auto togglePlayPause = new QShortcut(Qt::Key_Space, this);
    connect(togglePlayPause, &QShortcut::activated, this, &MainWindow::togglePlayPause);

    const auto removePlaylist = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);
    connect(removePlaylist, &QShortcut::activated, this, &MainWindow::removeCurrentPlaylist);

    const auto changePlaylist = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Tab), this);
    connect(changePlaylist, &QShortcut::activated, this, [this]() {
        const auto playlistCount = ui.playlist->count();
        const auto currentPlaylistIndex = ui.playlist->currentIndex();

        auto newPlaylistIndex = currentPlaylistIndex + 1;
        if(newPlaylistIndex >= playlistCount)
        {
            newPlaylistIndex = 0;
        }

        ui.playlist->setCurrentIndex(newPlaylistIndex);
    });

    const auto backChangePlaylist = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Tab), this);
    connect(backChangePlaylist, &QShortcut::activated, this, [this]() {
        const auto playlistCount = ui.playlist->count();
        const auto currentPlaylistIndex = ui.playlist->currentIndex();

        auto newPlaylistIndex = currentPlaylistIndex - 1;
        if(newPlaylistIndex < 0)
        {
            newPlaylistIndex = playlistCount - 1;
        }

        ui.playlist->setCurrentIndex(newPlaylistIndex);
    });
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

void MainWindow::disableSeekbar()
{
    ui.seekbar->setValue(0);
    ui.seekbar->setDisabled(true);
}

void MainWindow::enableSeekbar(std::chrono::seconds trackDuration)
{
    this->ui.seekbar->setMaximum(trackDuration.count() * 1000);
    this->ui.seekbar->setDisabled(false);
}

void MainWindow::togglePlaylistRenameControl(int tabIndex)
{
    const auto playlistId{ getPlaylistIdByTabIndex(tabIndex) };
    if(not playlistId)
    {
        qWarning() << "No playlist exist on tabIndex:" << tabIndex;
        return;
    }

    auto *tabbar = ui.playlist->tabBar();
    const auto tabRect = tabbar->tabRect(tabIndex);

    auto renameLineEdit = new EscapableLineEdit(tabbar);
    renameLineEdit->show();
    renameLineEdit->move(tabRect.topLeft());
    renameLineEdit->resize(tabRect.width(), tabRect.height());
    renameLineEdit->setText(tabbar->tabText(tabIndex));
    renameLineEdit->selectAll();
    renameLineEdit->setFocus();

    ui.playlistRenameWidget = renameLineEdit;

    connect(renameLineEdit, &EscapableLineEdit::editingFinished,
            [this, tabbar, playlistId = playlistId.value(), renameLineEdit]() {
                const auto tabIndex = getTabIndexByPlaylistId(playlistId);
                if(not tabIndex)
                {
                    qWarning() << "Playlist not found";
                    renameLineEdit->deleteLater();
                    ui.playlistRenameWidget = nullptr;
                    return;
                }

                const QString oldValue = tabbar->tabText(*tabIndex);
                const QString newValue = renameLineEdit->text();
                if(oldValue != newValue)
                {
                    const auto renameResult = playlistManager_.rename(playlistId, newValue);
                    if(renameResult)
                    {
                        tabbar->setTabText(*tabIndex, newValue);
                        qDebug() << "Playlist" << oldValue << "renamed to" << newValue;

                        const auto lastPlaylistName = settings_.value(config::lastPlaylistKey).toString();
                        if(lastPlaylistName == oldValue)
                        {
                            settings_.setValue(config::lastPlaylistKey, newValue);
                            qDebug() << "Updated last playlist:" << newValue;
                        }
                    }
                }

                renameLineEdit->deleteLater();
                ui.playlistRenameWidget = nullptr;
            });

    connect(renameLineEdit, &EscapableLineEdit::cancelEdit, this, [this]() {
        ui.playlistRenameWidget->deleteLater();
        ui.playlistRenameWidget = nullptr;
    });
}

void MainWindow::playMediaFromPlaylist(std::uint32_t playlistId, std::size_t index)
{
    auto *playlist = playlistManager_.get(playlistId);
    if(not playlist)
    {
        return;
    }

    const auto *track = playlist->getTrack(index);
    qDebug() << "Playing media:" << track->path;

    this->mediaPlayer_->setMedia(QUrl::fromUserInput(track->path));
    this->mediaPlayer_->play();

    const auto trackDuration =
        track->audioMetaData ? track->audioMetaData->duration : std::chrono::seconds{ 0 };
    enableSeekbar(trackDuration);

    playlist->setCurrentTrackIndex(index);
    this->ui.playlist->update();

    disconnect(mediaPlayer_.get(), &QMediaPlayer::mediaStatusChanged, nullptr, nullptr);
    connect(mediaPlayer_.get(), &QMediaPlayer::mediaStatusChanged,
            [this, playlistId](const QMediaPlayer::MediaStatus status) {
                using Status = QMediaPlayer::MediaStatus;
                if(Status::EndOfMedia == status or Status::InvalidMedia == status)
                {
                    disableSeekbar();
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
    const auto currentTabIndex = ui.playlist->currentIndex();
    if(currentTabIndex == -1)
    {
        return;
    }

    const auto playlistId = getPlaylistIdByTabIndex(currentTabIndex);
    if(not playlistId)
    {
        ui.playlist->removeTab(currentTabIndex);
        return;
    }

    ui.playlist->removeTab(currentTabIndex);
    playlistManager_.removeById(*playlistId);
}

void MainWindow::onMediaFinish(std::uint32_t playlistId)
{
    const auto *playlist = playlistManager_.get(playlistId);
    if(not playlist)
    {
        // TODO: Update/notify that nothing's playing
        return;
    }

    const auto nextTrackIndex = playlist->getNextTrackIndex(getCurrentPlayMode());
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
        setupPlaylistTab(playlist.second);
    }
}

void MainWindow::enablePlaylistChangeTracking()
{
    // Tracking cannot be enabled before playlists are loaded
    // Otherwise last playlist name is updated with loaded playlists

    auto *tabbar = ui.playlist->tabBar();
    connect(tabbar, &MultilineTabBar::currentChanged, [this](int newIndex) {
        const auto playlistId = getPlaylistIdByTabIndex(newIndex);
        if(not playlistId)
        {
            qDebug() << "No playlist at tab index:" << newIndex;
            return;
        }

        const auto *playlist = playlistManager_.get(*playlistId);
        if(not playlist)
        {
            qCritical() << "No playlist with given playlistId:" << *playlistId;
            return;
        }
        settings_.setValue(config::lastPlaylistKey, playlist->getName());
        qDebug() << "Updated last playlist:" << playlist->getName();
    });
}

void MainWindow::restoreLastPlaylist()
{
    if(settings_.contains(config::lastPlaylistKey))
    {
        const auto lastPlaylistName = settings_.value(config::lastPlaylistKey).toString();
        const auto playlistTabIndex = getTabIndexByPlaylistName(lastPlaylistName);
        if(playlistTabIndex)
        {
            qDebug() << "Restoring playlist:" << lastPlaylistName;
            ui.playlist->setCurrentIndex(*playlistTabIndex);
            ui.playlist->widget(*playlistTabIndex)->setFocus(Qt::FocusReason::NoFocusReason);
        }
    }
}

PlayMode MainWindow::getCurrentPlayMode()
{
    constexpr auto defaultPlayMode = static_cast<int>(PlayMode::RepeatPlaylist);
    return static_cast<PlayMode>(settings_.value(config::playModeKey, defaultPlayMode).toInt());
}

const Playlist *MainWindow::getPlaylistByTabIndex(int tabIndex)
{
    const auto *widget = ui.playlist->widget(tabIndex);
    if(not widget)
    {
        return nullptr;
    }
    const auto *playlistWidget = qobject_cast<const PlaylistWidget *>(widget);
    return &playlistWidget->getPlaylist();
}

std::optional<std::uint32_t> MainWindow::getPlaylistIdByTabIndex(int tabIndex)
{
    const auto *playlist = getPlaylistByTabIndex(tabIndex);
    if(not playlist)
    {
        return std::nullopt;
    }
    return playlist->getPlaylistId();
}

std::optional<int> MainWindow::getTabIndexByPlaylistName(const QString &name)
{
    for(int tabIndex = 0; tabIndex < ui.playlist->count(); ++tabIndex)
    {
        const auto *playlist = getPlaylistByTabIndex(tabIndex);
        if(playlist && name == playlist->getName())
        {
            return tabIndex;
        }
    }

    return std::nullopt;
}

std::optional<int> MainWindow::getTabIndexByPlaylistId(std::uint32_t playlistId)
{
    for(int tabIndex = 0; tabIndex < ui.playlist->count(); ++tabIndex)
    {
        const auto *playlist = getPlaylistByTabIndex(tabIndex);
        if(playlist && playlistId == playlist->getPlaylistId())
        {
            return tabIndex;
        }
    }

    return std::nullopt;
}
