// SPDX-FileCopyrightText: 2021 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2022-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "VideoDisplay.h"
#include <vlc/vlc.h>

class QTimer;

namespace Viewer
{
class VLCDisplay : public Viewer::VideoDisplay
{
    Q_OBJECT

public:
    VLCDisplay(QWidget *parent);
    ~VLCDisplay();
    bool isPaused() const override;
    bool isPlaying() const override;
    QImage screenShoot() override;
    void relativeSeek(int msec) override;
    void restart() override;

public Q_SLOTS:
    void updateInterface();
    void changeVolume(int newVolume);
    void setPosition(int newPosition);

    void playPause() override;
    void stop() final override;
    void rotate(const DB::ImageInfoPtr &info) override;

protected:
    bool setImageImpl(DB::ImageInfoPtr info, bool forward) override;
    virtual void closeEvent(QCloseEvent *event) override;

private:
    void releaseVLC();
    void setupVLC();

    QWidget *m_videoWidget = nullptr;
    class VideoToolBar *m_videoToolBar = nullptr;
    QTimer *m_poller = nullptr;
    libvlc_instance_t *m_vlcInstance = nullptr;
    libvlc_media_player_t *m_player = nullptr;
    libvlc_media_t *m_media = nullptr;
};

} // namespace Viewer
