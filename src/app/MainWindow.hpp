#pragma once

#include <QHBoxLayout>
#include <QSlider>
#include <QTreeView>
#include <QWidget>

#include <Playlist.hpp>
#include <memory>

class Playlist;
class PlaylistManager;
class MultilineTabWidget;
class EscapableLineEdit;

class QMediaPlayer;
class QSettings;
class QCloseEvent;

class MainWindow final : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QSettings &, PlaylistManager &);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *) override;
    bool eventFilter(QObject *, QEvent *) override;

private:
    void setupWindow();
    void setupMenu();
    void setupVolumeControl();
    void setupSeekbar();
    void setupPlaybackControlButtons();
    void setupAlbumsBrowser();
    void setupPlaylistWidget();
    int setupPlaylistTab(Playlist &);

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
    void enablePlaylistChangeTracking();

    void restoreLastPlaylist();

    PlayMode getCurrentPlayMode();
    const Playlist *getPlaylistByTabIndex(int tabIndex);
    std::optional<std::uint32_t> getPlaylistIdByTabIndex(int tabIndex);
    std::optional<int> getTabIndexByPlaylistName(const QString &name);
    std::optional<int> getTabIndexByPlaylistId(std::uint32_t playlistId);

signals:
    void removeDuplicates(std::uint32_t);

private:
    struct
    {
        QHBoxLayout *menuLayout;
        QHBoxLayout *buttonsLayout;
        QSlider *volumeSlider;
        QSlider *seekbar;
        QTreeView *albums;
        MultilineTabWidget *playlist;

        EscapableLineEdit *playlistRenameWidget = nullptr;
    } ui;

    QSettings &settings_;
    PlaylistManager &playlistManager_;
    std::unique_ptr<QMediaPlayer> mediaPlayer_;
};
