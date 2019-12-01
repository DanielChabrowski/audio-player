#pragma once

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
    void setupPlaylistWidget(Playlist *);

    void setupMediaPlayer();
    void setupGlobalShortcuts();

    void setTheme(const QString &filename);

    void connectMediaPlayerToSeekbar();

    void playMediaFromCurrentPlaylist(Playlist *, std::size_t index);
    void togglePlayPause();

    void onMediaFinish(Playlist *);

    void loadPlaylists();

private:
    Ui::MainWindowForm ui;
    QSettings &settings_;
    PlaylistManager &playlistManager_;
    std::unique_ptr<QMediaPlayer> mediaPlayer_;
};
