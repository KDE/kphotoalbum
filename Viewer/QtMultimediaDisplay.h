// SPDX-FileCopyrightText: 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef QTMULTIMEDIADISPLAY_H
#define QTMULTIMEDIADISPLAY_H

#include "VideoDisplay.h"

#include <QMediaPlayer>

class QVideoWidget;

namespace Viewer
{

class VideoToolBar;

class QtMultimediaDisplay : public Viewer::VideoDisplay
{
    Q_OBJECT
public:
    QtMultimediaDisplay(QWidget *parent);
    ~QtMultimediaDisplay() override;
    bool setImageImpl(DB::ImageInfoPtr info, bool forward) override;
    bool isPaused() const override;
    bool isPlaying() const override;
    QImage screenShoot() override;
    void relativeSeek(int msec) override;

public Q_SLOTS:
    // void zoomIn() override;
    // void zoomOut() override;
    // void zoomFull() override;
    // void zoomPixelForPixel() override;
    void stop() override;
    void playPause() override;
    void restart() override;
    void rotate(const DB::ImageInfoPtr &info) override;

protected:
    void setup();
    void setVideoWidgetSize();
    void resizeEvent(QResizeEvent *) override;

protected Q_SLOTS:
    void updatePlaybackState(QMediaPlayer::PlaybackState newState);
    void updateDuration(qint64 duration);

private:
    QMediaPlayer *m_mediaPlayer;
    QVideoWidget *m_videoWidget;
    VideoToolBar *m_videoToolBar;
    void errorOccurred(QMediaPlayer::Error, const QString &errorString);
};

} // namespace Viewer

#endif // QTMULTIMEDIADISPLAY_H
// vi:expandtab:tabstop=4 shiftwidth=4:
