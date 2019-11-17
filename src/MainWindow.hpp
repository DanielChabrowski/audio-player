#pragma once

#include <memory>

#include "ui_MainWindow.h"

struct Playlist;

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
    void setupSeekbar();
    void setupPlaybackControlButtons();
    void setupAlbumsBrowser();
    void setupPlaylistWidget();

    void setTheme(const QString &filename);

    void connectMediaPlayerToSeekbar();

    void playMediaFromCurrentPlaylist(int index);

    void onMediaFinish();

private:
    Ui::MainWindowForm ui;
    std::unique_ptr<QSettings> settings_;
    std::unique_ptr<Playlist> playlist;
    std::unique_ptr<QMediaPlayer> mediaPlayer_;
};
