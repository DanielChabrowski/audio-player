#include "MainWindow.hpp"

#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <QTime>
#include <QToolTip>
#include <QtGlobal>

#include "AlbumGallery.hpp"
#include "AlbumModel.hpp"
#include "ConfigurationKeys.hpp"
#include "EscapableLineEdit.hpp"
#include "FilesystemPlaylistIO.hpp"
#include "MediaPlayer.hpp"
#include "MultilineTabBar.hpp"
#include "PlaylistFilterModel.hpp"
#include "PlaylistHeader.hpp"
#include "PlaylistManager.hpp"
#include "PlaylistModel.hpp"
#include "PlaylistWidget.hpp"

#include <algorithm>

MainWindow::MainWindow(QSettings &settings, LibraryManager &libraryManager, PlaylistManager &playlistManager, MediaPlayer &mediaPlayer)
: QWidget{ nullptr }
, settings_{ settings }
, libraryManager_{ libraryManager }
, playlistManager_{ playlistManager }
, mediaPlayer_{ mediaPlayer }
{
    setupWindow();

    setTheme(":/themes/gray-orange.css");

    {
        // Restoring window size and position
        restoreGeometry(settings_.value(config::geometryKey).toByteArray());
    }

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
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(6);

    auto *topHLayout = new QHBoxLayout();

    ui.menuLayout = new QHBoxLayout();
    topHLayout->addLayout(ui.menuLayout);

    ui.buttonsLayout = new QHBoxLayout();
    topHLayout->addLayout(ui.buttonsLayout);

    {
        ui.volumeSlider = new QSlider(this);
        ui.volumeSlider->setFocusPolicy(Qt::FocusPolicy::NoFocus);

        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ui.volumeSlider->sizePolicy().hasHeightForWidth());
        ui.volumeSlider->setSizePolicy(sizePolicy);
        ui.volumeSlider->setOrientation(Qt::Horizontal);

        topHLayout->addWidget(ui.volumeSlider);
    }

    {
        ui.seekbar = new QSlider(this);
        ui.seekbar->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        ui.seekbar->setOrientation(Qt::Horizontal);

        topHLayout->addWidget(ui.seekbar);
    }

    layout->addLayout(topHLayout, 0, 5, 1, 1);

    auto *bottomHLayout = new QHBoxLayout();
    bottomHLayout->setContentsMargins(0, 0, 0, 0);
    bottomHLayout->setSpacing(2);

    {
        ui.albums = new QTreeView(this);

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

    auto *vLayout = new QVBoxLayout();

    {
        ui.playlist = new MultilineTabWidget(this);
        ui.playlist->setCurrentIndex(-1);

        vLayout->addWidget(ui.playlist);
    }

    {
        ui.playlistSearch = createPlaylistSearchWidget();
        vLayout->addWidget(ui.playlistSearch);
    }

    bottomHLayout->addLayout(vLayout);

    {
        auto *widget = new QWidget(this);
        widget->setLayout(bottomHLayout);

        ui.mainStack = new QStackedWidget(this);
        ui.mainStack->addWidget(widget);

        auto *albumWidget = new AlbumGallery(this);
        albumWidget->setModel(new AlbumModel(libraryManager_, this));

        ui.mainStack->addWidget(albumWidget);

        layout->addWidget(ui.mainStack, 1, 5, 1, 1);
    }
}

void MainWindow::setupMenu()
{
    auto *bar = new QMenuBar(this);
    auto *fileMenu = bar->addMenu("File");

    fileMenu->addAction(
        "Open", this,
        [this]()
        {
            auto filenames = QFileDialog::getOpenFileNames(this, "Open audio",
                QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0),
                "Audio Files (*.mp3 *.wav *.ogg)");

            const auto currentTabIndex = getCurrentPlaylistTabIndex();
            const auto playlistId = getPlaylistIdByTabIndex(currentTabIndex);

            if(not playlistId)
            {
                qWarning() << "Attempted insert into playlist that doesn't exist";
                return;
            }

            emit playlistInsertRequest(playlistId.value(), filenames);
        },
        QKeySequence::Open);

    auto *newPlaylistAction = fileMenu->addAction("Add new playlist", this,
        [this]()
        {
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

    auto *exitAction = fileMenu->addAction("Exit", this,
        []()
        {
            constexpr int exitCode{ 0 };
            QApplication::exit(exitCode);
        });
    exitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));

    {
        auto *editMenu = bar->addMenu("Edit");
        editMenu->addAction("Remove duplicates", this,
            [this]()
            {
                const auto currentTabIndex = getCurrentPlaylistTabIndex();
                const auto playlistId = getPlaylistIdByTabIndex(currentTabIndex);
                if(not playlistId)
                {
                    qWarning() << "Playlist not found, tab index:" << currentTabIndex;
                    return;
                }

                emit removeDuplicates(*playlistId);
            });
    }

    {
        auto *viewMenu = bar->addMenu("View");
        viewMenu->addAction("Toggle album view", this,
            [this]()
            {
                const auto toggle = !ui.albums->isVisible();
                ui.albums->setVisible(toggle);
                settings_.setValue(config::albumsVisibility, toggle);
            });
    }

    auto *playbackMenu = bar->addMenu("Playback");

    auto *actionGroup = new QActionGroup(this);

    const auto currentPlayMode = getCurrentPlayMode();

    auto addPlaybackAction = [this, actionGroup, currentPlayMode](const QString &label, PlayMode mode)
    {
        auto *action = actionGroup->addAction(label);
        action->setCheckable(true);
        action->setChecked(currentPlayMode == mode);

        connect(action, &QAction::toggled,
            [this, mode](bool checked)
            {
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

    auto *libraryMenu = bar->addMenu("Library");
    libraryMenu->addAction("Albums", this,
        [this]()
        {
            auto index = ui.mainStack->currentIndex();
            ui.mainStack->setCurrentIndex(!index);
        });

    bar->addMenu("Help");

    ui.menuLayout->addWidget(bar);
}

void MainWindow::setupVolumeControl()
{
    constexpr auto defaultVolume{ 30 };
    constexpr auto minVolume{ 0 };
    constexpr auto maxVolume{ 100 };

    const auto sliderValue =
        qBound(minVolume, settings_.value(config::volumeKey, defaultVolume).toInt(), maxVolume);
    ui.volumeSlider->setMaximum(maxVolume);
    ui.volumeSlider->setValue(sliderValue);

    const float volume = static_cast<float>(sliderValue) / maxVolume;
    mediaPlayer_.setVolume(volume);

    connect(ui.volumeSlider, &QSlider::valueChanged,
        [this](int sliderValue)
        {
            const float volume = static_cast<float>(sliderValue) / maxVolume;
            mediaPlayer_.setVolume(volume);

            this->settings_.setValue(config::volumeKey, sliderValue);
            qDebug() << "Volume set to" << volume;
        });
}

void MainWindow::setupSeekbar()
{
    disableSeekbar();

    connect(ui.seekbar, &QSlider::sliderPressed,
        [this]() { disconnect(&mediaPlayer_, &MediaPlayer::positionChanged, ui.seekbar, nullptr); });

    connect(ui.seekbar, &QSlider::sliderReleased,
        [this]()
        {
            this->mediaPlayer_.setPosition(this->ui.seekbar->value());
            connectMediaPlayerToSeekbar();
        });

    connect(ui.seekbar, &QSlider::sliderMoved,
        [this](int value)
        {
            const auto currentTime = QTime(0, 0, 0, 0).addMSecs(value);
            const auto verticalPos = QWidget::mapToGlobal(ui.seekbar->geometry().bottomLeft());

            QToolTip::showText(QPoint{ QCursor::pos().x(), verticalPos.y() }, currentTime.toString("H:mm:ss"));
        });
}

void MainWindow::setupPlaybackControlButtons()
{
    const auto createButtonFunc = [this](const QString &filename, std::function<void()> onReleaseEvent)
    {
        auto *button = new QPushButton(this);
        button->setFlat(true);
        button->setMaximumSize(24, 24);
        button->setIcon(QPixmap(filename));
        button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        connect(button, &QPushButton::released, std::move(onReleaseEvent));

        ui.buttonsLayout->addWidget(button);
    };

    ui.buttonsLayout->QLayout::setSpacing(0);

    createButtonFunc(":/icons/play.png", [this]() { mediaPlayer_.play(); });
    createButtonFunc(":/icons/pause.png", [this]() { mediaPlayer_.pause(); });
    createButtonFunc(":/icons/stop.png", [this]() { mediaPlayer_.stop(); });
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

    connect(tabbar, &QTabBar::customContextMenuRequested,
        [this, tabbar](const QPoint &point)
        {
            const auto tabIndex = tabbar->tabAt(point);
            const auto playlistId = getPlaylistIdByTabIndex(tabIndex);

            QMenu menu;
            menu.addAction("Rename playlist", this,
                [this, tabIndex] { togglePlaylistRenameControl(tabIndex); });
            menu.addAction("Remove playlist", this,
                [this, playlistId = *playlistId, tabIndex]()
                {
                    ui.playlist->removeTab(tabIndex);
                    playlistManager_.removeById(playlistId);
                });

            menu.exec(tabbar->mapToGlobal(point));
        });
}

EscapableLineEdit *MainWindow::createPlaylistSearchWidget()
{
    auto *widget = new EscapableLineEdit(this);
    widget->setPlaceholderText("Type your search query");
    widget->hide();

    connect(widget, &EscapableLineEdit::cancelEdit, this, &MainWindow::onPlaylistSearchCanceled);
    connect(widget, &EscapableLineEdit::textChanged, this, &MainWindow::onPlaylistSearchTextChanged);

    return widget;
}

int MainWindow::setupPlaylistTab(Playlist &playlist)
{
    const auto playlistId = playlist.getPlaylistId();

    auto playlistModel = std::make_unique<PlaylistModel>(playlist, this);
    connect(this, &MainWindow::removeDuplicates, playlistModel.get(),
        [playlistId, model = playlistModel.get()](PlaylistId eventPlaylistId)
        {
            if(playlistId == eventPlaylistId)
            {
                model->onDuplicateRemoveRequest();
            }
        });

    connect(this, &MainWindow::playlistInsertRequest, playlistModel.get(),
        [playlistId, model = playlistModel.get()](PlaylistId eventPlaylistId, QStringList filenames)
        {
            if(playlistId == eventPlaylistId)
            {
                model->onInsertRequest(filenames);
            }
        });

    auto filterModel = std::make_unique<PlaylistFilterModel>(this);
    filterModel->setSourceModel(playlistModel.release());

    auto playlistWidget = std::make_unique<PlaylistWidget>(playlist,
        [this, playlistId, proxyModel = filterModel.get()](int index) {
            playMediaFromPlaylist(playlistId, proxyModel->mapToSource(proxyModel->index(index, 0)).row());
        });
    auto playlistHeader = std::make_unique<PlaylistHeader>(playlistWidget.get());

    connect(this, &MainWindow::updateSearchResult, filterModel.get(),
        [model = filterModel.get()](QString query) { model->setFilterQuery(query); });

    playlistWidget->setModel(filterModel.release());
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

void MainWindow::setupGlobalShortcuts()
{
    const auto togglePlayPause = new QShortcut(Qt::Key_Space, this);
    connect(togglePlayPause, &QShortcut::activated, this, &MainWindow::togglePlayPause);

    const auto removePlaylist = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this);
    connect(removePlaylist, &QShortcut::activated, this, &MainWindow::removeCurrentPlaylist);

    const auto changePlaylist = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab), this);
    connect(changePlaylist, &QShortcut::activated, this,
        [this]()
        {
            const auto playlistCount = ui.playlist->count();
            const auto currentPlaylistIndex = getCurrentPlaylistTabIndex();

            auto newPlaylistIndex = currentPlaylistIndex + 1;
            if(newPlaylistIndex >= playlistCount)
            {
                newPlaylistIndex = 0;
            }

            ui.playlist->setCurrentIndex(newPlaylistIndex);
        });

    const auto backChangePlaylist = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Tab), this);
    connect(backChangePlaylist, &QShortcut::activated, this,
        [this]()
        {
            const auto playlistCount = ui.playlist->count();
            const auto currentPlaylistIndex = getCurrentPlaylistTabIndex();

            auto newPlaylistIndex = currentPlaylistIndex - 1;
            if(newPlaylistIndex < 0)
            {
                newPlaylistIndex = playlistCount - 1;
            }

            ui.playlist->setCurrentIndex(newPlaylistIndex);
        });

    constexpr auto seekMilliseconds = 5 * 1000;

    const auto seekForwards = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right), this);
    connect(seekForwards, &QShortcut::activated, this,
        [&, this]()
        {
            const auto maximum = ui.seekbar->maximum();
            const auto newPosition = std::clamp<qint64>(mediaPlayer_.position() + seekMilliseconds, 0, maximum);
            mediaPlayer_.setPosition(newPosition);
        });

    const auto seekBackwards = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left), this);
    connect(seekBackwards, &QShortcut::activated, this,
        [&, this]()
        {
            const auto maximum = ui.seekbar->maximum();
            const auto newPosition = std::clamp<qint64>(mediaPlayer_.position() - seekMilliseconds, 0, maximum);
            mediaPlayer_.setPosition(newPosition);
        });

    const auto playlistSearchShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(playlistSearchShortcut, &QShortcut::activated, this,
        [&, this]()
        {
            const bool hasFocus = ui.playlistSearch->hasFocus();
            const bool isVisible = ui.playlistSearch->isVisible();

            bool toggleVisibility = true;

            if(isVisible && hasFocus)
            {
                toggleVisibility = false;
            }

            ui.playlistSearch->setVisible(toggleVisibility);

            if(toggleVisibility)
            {
                ui.playlistSearch->setFocus();
            }
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
    connect(&mediaPlayer_, &MediaPlayer::positionChanged, ui.seekbar, &QSlider::setValue);
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
        [this, tabbar, playlistId = playlistId.value(), renameLineEdit]()
        {
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

    connect(renameLineEdit, &EscapableLineEdit::cancelEdit, this,
        [this]()
        {
            ui.playlistRenameWidget->deleteLater();
            ui.playlistRenameWidget = nullptr;
        });
}

void MainWindow::playMediaFromPlaylist(PlaylistId playlistId, std::size_t index)
{
    auto *playlist = playlistManager_.get(playlistId);
    if(not playlist)
    {
        return;
    }

    const auto *track = playlist->getTrack(index);
    qDebug() << "Playing media:" << track->path;

    this->mediaPlayer_.setSource(QUrl::fromUserInput(track->path));
    this->mediaPlayer_.play();

    const auto trackDuration =
        track->audioMetaData ? track->audioMetaData->duration : std::chrono::seconds{ 0 };
    enableSeekbar(trackDuration);

    playlist->setCurrentTrackIndex(index);
    this->ui.playlist->update();

    disconnect(&mediaPlayer_, &MediaPlayer::mediaStatusChanged, nullptr, nullptr);
    connect(&mediaPlayer_, &MediaPlayer::mediaStatusChanged,
        [this, playlistId](const MediaStatus status)
        {
            if(MediaStatus::EndOfMedia == status or MediaStatus::InvalidMedia == status)
            {
                disableSeekbar();
                onMediaFinish(playlistId);
            }
        });
}

void MainWindow::togglePlayPause()
{
    if(PlaybackState::PlayingState == mediaPlayer_.playbackState())
    {
        mediaPlayer_.pause();
    }
    else
    {
        mediaPlayer_.play();
    }
}

void MainWindow::removeCurrentPlaylist()
{
    const auto currentTabIndex = getCurrentPlaylistTabIndex();
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

void MainWindow::onMediaFinish(PlaylistId playlistId)
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
    connect(tabbar, &MultilineTabBar::currentChanged,
        [this](int newIndex)
        {
            const auto playlistId = getPlaylistIdByTabIndex(newIndex);
            if(not playlistId)
            {
                qDebug() << "No playlist at tab index:" << newIndex;
                return;
            }

            const auto *playlist = playlistManager_.get(*playlistId);
            if(not playlist)
            {
                qCritical() << "No playlist with given playlistId:" << playlistId->value;
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

int MainWindow::getCurrentPlaylistTabIndex() const
{
    return ui.playlist->currentIndex();
}

PlaylistWidget *MainWindow::getPlaylistWidgetByTabIndex(int tabIndex)
{
    auto *widget = ui.playlist->widget(tabIndex);
    if(not widget)
    {
        return nullptr;
    }
    return qobject_cast<PlaylistWidget *>(widget);
}

const Playlist *MainWindow::getPlaylistByTabIndex(int tabIndex)
{
    auto playlistWidget = getPlaylistWidgetByTabIndex(tabIndex);
    if(not playlistWidget)
    {
        return nullptr;
    }
    return &playlistWidget->getPlaylist();
}

std::optional<PlaylistId> MainWindow::getPlaylistIdByTabIndex(int tabIndex)
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

std::optional<int> MainWindow::getTabIndexByPlaylistId(PlaylistId playlistId)
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

void MainWindow::onPlaylistSearchCanceled()
{
    emit updateSearchResult("");
    ui.playlistSearch->hide();
    ui.playlistSearch->clear();
}

void MainWindow::onPlaylistSearchTextChanged(const QString &query)
{
    emit updateSearchResult(query);
}
