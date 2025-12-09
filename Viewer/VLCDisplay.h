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
/**
 * @brief The VLCDisplay class
 *
 * \note To ensure proper cleanup, you need to stop playback before deleting the VLCDisplay!
 * The VLCDisplay is not in the position to do this, since it is not the main widget of the
 * window and therefore does not receive a QCloseEvent (where the window handle is still valid).
 * There is no way to call releaseVLC safely from the destructor otherwise.
 */
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
    bool canRotate() override;

public Q_SLOTS:
    void updateInterface();
    void changeVolume(int newVolume);
    void setPosition(int newPosition);

    void playPause() override;
    void stop() final override;
    void rotate(const DB::ImageInfoPtr &info) override;

protected:
    bool setImageImpl(DB::ImageInfoPtr info, bool forward) override;

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
