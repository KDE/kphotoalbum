// SPDX-FileCopyrightText: 2021-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "VideoDisplay.h"

namespace QtAV
{
class VideoRenderer;
class AVPlayer;
class LibAVFilterVideo;
}

namespace Viewer
{
class QtAVVideoToolBar;

class QtAVDisplay final : public Viewer::VideoDisplay
{
    Q_OBJECT

public:
    explicit QtAVDisplay(QWidget *parent);
    ~QtAVDisplay() override;
    bool isPaused() const override;
    bool isPlaying() const override;
    QImage screenShoot() override;

public Q_SLOTS:
    void stop() override;
    void playPause() override;
    void restart() override;
    void relativeSeek(int msec) override;
    void seekToPosition(qint64 pos);
    void rotate(const DB::ImageInfoPtr &info) override;

protected:
    bool setImageImpl(DB::ImageInfoPtr info, bool forward) override;
    void displayPosition(quint64 pos);

private:
    QWidget *m_videoWidget = nullptr;
    QtAVVideoToolBar *m_toolBar = nullptr;
    QtAV::VideoRenderer *m_renderer = nullptr;
    QtAV::AVPlayer *m_player = nullptr;
    QtAV::LibAVFilterVideo *m_rotateFilter = nullptr;
};

}

// vi:expandtab:tabstop=4 shiftwidth=4:
