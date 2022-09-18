#pragma once

#include <QDBusConnection>
#include <QDBusObjectPath>

#include "MprisTypes.hpp"

class MediaPlayer;

namespace plugins
{
class MprisPlugin final : QObject
{
    Q_OBJECT

public:
    explicit MprisPlugin(MediaPlayer &mediaPlayer);
    ~MprisPlugin();

public: // PROPERTIES
    Q_PROPERTY(bool CanControl READ canControl)
    bool canControl() const;

    Q_PROPERTY(bool CanGoNext READ canGoNext)
    bool canGoNext() const;

    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
    bool canGoPrevious() const;

    Q_PROPERTY(bool CanPause READ canPause)
    bool canPause() const;

    Q_PROPERTY(bool CanPlay READ canPlay)
    bool canPlay() const;

    Q_PROPERTY(bool CanSeek READ canSeek)
    bool canSeek() const;

    Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus)
    QString loopStatus() const;
    void setLoopStatus(const QString &value);

    Q_PROPERTY(double MaximumRate READ maximumRate)
    double maximumRate() const;

    Q_PROPERTY(QVariantMap Metadata READ metadata)
    QVariantMap metadata() const;

    Q_PROPERTY(double MinimumRate READ minimumRate)
    double minimumRate() const;

    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    QString playbackStatus() const;

    Q_PROPERTY(qlonglong Position READ position)
    qlonglong position() const;

    Q_PROPERTY(double Rate READ rate WRITE setRate)
    double rate() const;
    void setRate(double value);

    Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)
    bool shuffle() const;
    void setShuffle(bool value);

    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    double volume() const;
    void setVolume(double value);

public slots: // METHODS
    void Next();
    void OpenUri(const QString &Uri);
    void Pause();
    void Play();
    void PlayPause();
    void Previous();
    void Seek(qlonglong Offset);
    void SetPosition(const QDBusObjectPath &TrackId, qlonglong Position);
    void Stop();

signals: // SIGNALS
    void Seeked(qlonglong Position);

public: // org.mpris.MediaPlayer2 PROPERTIES
    Q_PROPERTY(bool CanQuit READ canQuit)
    bool canQuit() const;

    Q_PROPERTY(bool CanRaise READ canRaise)
    bool canRaise() const;

    Q_PROPERTY(bool CanSetFullscreen READ canSetFullscreen)
    bool canSetFullscreen() const;

    Q_PROPERTY(QString DesktopEntry READ desktopEntry)
    QString desktopEntry() const;

    Q_PROPERTY(bool Fullscreen READ fullscreen WRITE setFullscreen)
    bool fullscreen() const;
    void setFullscreen(bool value);

    Q_PROPERTY(bool HasTrackList READ hasTrackList)
    bool hasTrackList() const;

    Q_PROPERTY(QString Identity READ identity)
    QString identity() const;

    Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)
    QStringList supportedMimeTypes() const;

    Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes)
    QStringList supportedUriSchemes() const;

public slots: // METHODS
    void Quit();
    void Raise();

public: // PROPERTIES
    Q_PROPERTY(MprisMaybePlaylist ActivePlaylist READ activePlaylist)
    MprisMaybePlaylist activePlaylist() const;

    Q_PROPERTY(QStringList Orderings READ orderings)
    QStringList orderings() const;

    Q_PROPERTY(uint PlaylistCount READ playlistCount)
    uint playlistCount() const;

public slots: // METHODS
    void ActivatePlaylist(const QDBusObjectPath &playlistId);
    MprisPlaylistList GetPlaylists(uint index, uint maxCount, const QString &order, bool reverseOrder);

signals: // SIGNALS
    void PlaylistChanged(MprisPlaylist playlist);

private:
    MediaPlayer &mediaPlayer_;
};
} // namespace plugins
