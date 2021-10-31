#pragma once

#include <QHBoxLayout>
#include <QSlider>
#include <QStackedWidget>
#include <QStringList>
#include <QTreeView>
#include <QWidget>

#include "MediaPlayer.hpp"
#include "Playlist.hpp"

#include <memory>

class Playlist;
class PlaylistWidget;
class PlaylistManager;
class LibraryManager;
class MultilineTabWidget;
class EscapableLineEdit;

class QMediaPlayer;
class QSettings;
class QCloseEvent;

class MainWindow final : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QSettings &, LibraryManager &, PlaylistManager &);
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

    EscapableLineEdit *createPlaylistSearchWidget();

    void setupMediaPlayer();
    void setupGlobalShortcuts();

    void setTheme(const QString &filename);

    void connectMediaPlayerToSeekbar();
    void disableSeekbar();
    void enableSeekbar(std::chrono::seconds trackDuration);
    void togglePlaylistRenameControl(int tabIndex);

    void playMediaFromPlaylist(PlaylistId playlistId, std::size_t index);
    void togglePlayPause();
    void removeCurrentPlaylist();

    void onMediaFinish(PlaylistId playlistId);
    void loadPlaylists();
    void enablePlaylistChangeTracking();

    void restoreLastPlaylist();

    PlayMode getCurrentPlayMode();

    int getCurrentPlaylistTabIndex() const;
    PlaylistWidget *getPlaylistWidgetByTabIndex(int tabIndex);
    const Playlist *getPlaylistByTabIndex(int tabIndex);
    std::optional<PlaylistId> getPlaylistIdByTabIndex(int tabIndex);
    std::optional<int> getTabIndexByPlaylistName(const QString &name);
    std::optional<int> getTabIndexByPlaylistId(PlaylistId playlistId);

signals:
    void removeDuplicates(PlaylistId);
    void updateSearchResult(QString);
    void playlistInsertRequest(PlaylistId, QStringList);

private slots:
    void onPlaylistSearchCanceled();
    void onPlaylistSearchTextChanged(const QString &);

private:
    struct
    {
        QHBoxLayout *menuLayout;
        QHBoxLayout *buttonsLayout;
        QSlider *volumeSlider;
        QSlider *seekbar;
        QTreeView *albums;
        QStackedWidget *mainStack;
        MultilineTabWidget *playlist;
        EscapableLineEdit *playlistSearch;

        EscapableLineEdit *playlistRenameWidget = nullptr;
    } ui;

    QSettings &settings_;
    LibraryManager &libraryManager_;
    PlaylistManager &playlistManager_;

    std::unique_ptr<MediaPlayer> mediaPlayer_;
};
