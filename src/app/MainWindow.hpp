#pragma once

#include <Playlist.hpp>
#include <memory>

#include "ui_MainWindow.h"

class Playlist;
class PlaylistManager;

class QMediaPlayer;
class QSettings;
class QCloseEvent;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QSettings &, PlaylistManager &);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *) override;

private:
    void setupMenu();
    void setupVolumeControl();
    void setupSeekbar();
    void setupPlaybackControlButtons();
    void setupAlbumsBrowser();
    void setupPlaylistWidget();
    void setupPlaylistTab(Playlist &);

    void setupMediaPlayer();
    void setupGlobalShortcuts();

    void setTheme(const QString &filename);

    void connectMediaPlayerToSeekbar();
    void disableSeekbar();
    void enableSeekbar(std::chrono::seconds trackDuration);
    void togglePlaylistRenameControl(int tabIndex);

    void playMediaFromPlaylist(std::uint32_t playlistId, std::size_t index);
    void togglePlayPause();
    void removeCurrentPlaylist();

    void onMediaFinish(std::uint32_t playlistId);
    void loadPlaylists();

    PlayMode getCurrentPlayMode();
    std::optional<std::uint32_t> getPlaylistIdByTabIndex(int tabIndex);

private:
    Ui::MainWindowForm ui;
    QSettings &settings_;
    PlaylistManager &playlistManager_;
    std::unique_ptr<QMediaPlayer> mediaPlayer_;
};
