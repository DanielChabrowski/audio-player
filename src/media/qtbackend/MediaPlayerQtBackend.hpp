#pragma once

#include "MediaPlayer.hpp"

#include <memory>

class MediaPlayerQtBackend final : public MediaPlayer
{
    Q_OBJECT

public:
    explicit MediaPlayerQtBackend(QObject *parent = nullptr);
    ~MediaPlayerQtBackend();

    MediaPlayerQtBackend(const MediaPlayerQtBackend &) = delete;
    MediaPlayerQtBackend &operator=(const MediaPlayerQtBackend &) = delete;

    qint64 position() const override;
    PlaybackState playbackState() const override;

public slots:
    void setPosition(qint64 position) override;
    void setTrack(const PlaylistTrack &) override;

    void setVolume(float volume, const char *eventSource) override;
    float volume() const override;

    void play() override;
    void pause() override;
    void stop() override;

private slots:
    void priv_positionChanged(qint64 position);

private:
    struct Private;
    std::unique_ptr<Private> impl;
};
