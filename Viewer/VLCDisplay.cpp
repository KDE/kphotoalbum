// SPDX-FileCopyrightText: 2021 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2021 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VLCDisplay.h"
#include "VideoToolBar.h"
#include <DB/ImageInfo.h>
#include <QDebug>
#include <QGuiApplication>
#include <QLabel>
#include <QScreen>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

#include <vlc/libvlc_media_player.h>
#include <vlc/libvlc_version.h>

Viewer::VLCDisplay::VLCDisplay(QWidget *parent)
    : Viewer::VideoDisplay(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_videoWidget = new QWidget(this);
    layout->addWidget(m_videoWidget, 1);

    m_videoToolBar = new VideoToolBar;
    layout->addWidget(m_videoToolBar);

    m_poller = new QTimer(this);

    setupVLC();

    connect(m_poller, &QTimer::timeout, this, &Viewer::VLCDisplay::updateInterface);
    connect(m_videoToolBar, &VideoToolBar::positionChanged, this, &VLCDisplay::setPosition);
    connect(m_videoToolBar, &VideoToolBar::volumeChanged, this, &VLCDisplay::changeVolume);
    connect(m_videoToolBar, &VideoToolBar::muted, this, [this](bool b) {
        libvlc_audio_set_mute(m_player, b);
    });
}

Viewer::VLCDisplay::~VLCDisplay()
{
    releaseVLC();
}

void Viewer::VLCDisplay::changeVolume(int newVolume)
{
    libvlc_audio_set_volume(m_player, newVolume);
}

void Viewer::VLCDisplay::setPosition(int newPosition)
{
    // It's possible that the vlc doesn't play anything
    // so check before
    libvlc_media_t *curMedia = libvlc_media_player_get_media(m_player);
    if (curMedia == NULL)
        return;

    float pos = (float)(newPosition) / (float)m_videoToolBar->maximum();
#if LIBVLC_VERSION_INT >= 0x04000000
    libvlc_media_player_set_position(m_player, pos, /*fastseek=*/true);
#else
    libvlc_media_player_set_position(m_player, pos);
#endif
}

void Viewer::VLCDisplay::playPause()
{
    libvlc_media_player_set_pause(m_player, isPaused() ? 0 : 1);
}

void Viewer::VLCDisplay::updateInterface()
{
    libvlc_media_t *curMedia = libvlc_media_player_get_media(m_player);
    if (!curMedia || libvlc_media_get_state(m_media) == libvlc_Ended) {
        m_poller->stop();
        emit stopped();
        return;
    }

    const int currentPos
        = libvlc_media_player_get_position(m_player) * m_videoToolBar->maximum();
    const auto length = libvlc_media_player_get_length(m_player);
    m_videoToolBar->setRange(0, length);
    m_videoToolBar->setPosition(currentPos);
    m_videoToolBar->setVolume(libvlc_audio_get_volume(m_player));
    m_videoToolBar->setMuted(libvlc_audio_get_mute(m_player));
}

bool Viewer::VLCDisplay::setImageImpl(DB::ImageInfoPtr info, bool /*forward*/)
{
    m_media = libvlc_media_new_path(m_vlcInstance, info->fileName().absolute().toUtf8().data());

    libvlc_media_player_set_media(m_player, m_media);

    int windid = m_videoWidget->winId();
    libvlc_media_player_set_xwindow(m_player, windid);

    libvlc_media_player_play(m_player);
    m_poller->start(100);

    return true;
}

bool Viewer::VLCDisplay::isPaused() const
{
    return libvlc_media_get_state(m_media) == libvlc_Paused;
}

void Viewer::VLCDisplay::stop()
{
#if LIBVLC_VERSION_INT >= 0x04000000
    libvlc_media_player_stop_async(m_player);
#else
    libvlc_media_player_stop(m_player);
#endif
    m_poller->stop();
}

void Viewer::VLCDisplay::rotate(const DB::ImageInfoPtr &info)
{
    m_info = info;
    libvlc_time_t offset = 0;
    if (m_vlcInstance)
        offset = libvlc_media_player_get_time(m_player);

    releaseVLC();
    setupVLC();

    if (m_info)
        setImage(m_info, true);

    setPosition(offset);
}

void Viewer::VLCDisplay::releaseVLC()
{
    if (!m_vlcInstance)
        return;

    stop();
    libvlc_media_player_release(m_player);
    libvlc_release(m_vlcInstance);
}

void Viewer::VLCDisplay::setupVLC()
{
    if (m_info && !m_info->isNull() && m_info->angle() != 0) {
        constexpr int BUF_SIZE = 64;
        char buf[BUF_SIZE];
        snprintf(buf, BUF_SIZE, "--video-filter=transform{type=%d}", m_info->angle());

        const char *vlc_args[] = { "--verbose=2", // be much more verbose then normal for debugging purpose
                                   buf };
        m_vlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    } else
        m_vlcInstance = libvlc_new(0, {});

    m_player = libvlc_media_player_new(m_vlcInstance);
}

bool Viewer::VLCDisplay::isPlaying() const
{
    return libvlc_media_get_state(m_media) == libvlc_Playing;
}

QImage Viewer::VLCDisplay::screenShoot()
{
    return QGuiApplication::primaryScreen()->grabWindow(m_videoWidget->winId()).toImage();
}

void Viewer::VLCDisplay::relativeSeek(int msec)
{
    setPosition(libvlc_media_player_get_time(m_player) + msec);
}

void Viewer::VLCDisplay::restart()
{
    setImageImpl(m_info, true);
}

#include "moc_VLCDisplay.cpp"
