#pragma once

#include <memory>
#include <optional>

#include "Playlist.hpp"

#include "ui_MainWindow.h"

struct IAudioMetaDataProvider;

class QMediaPlayer;
class QSettings;
class QCloseEvent;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
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

    void setupMediaPlayer();
    void setupGlobalShortcuts();

    void setTheme(const QString &filename);

    void connectMediaPlayerToSeekbar();

    void playMediaFromCurrentPlaylist(int index);
    void togglePlayPause();

    void onMediaFinish();

    void loadPlaylists();

private:
    Ui::MainWindowForm ui;
    std::unique_ptr<QSettings> settings_;
    std::unique_ptr<QMediaPlayer> mediaPlayer_;

    // TODO: Shouldn't be a part of UI
    std::optional<Playlist> playlist;
    std::unique_ptr<IAudioMetaDataProvider> audioMetaDataProvider;
};
