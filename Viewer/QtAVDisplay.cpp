// SPDX-FileCopyrightText: 2021 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// QtAVWidgets/WidgetRenderer 1.13 is incompativle witch QT_DISABLE_DEPRECATED_BEFORE 5.15:
#undef QT_DISABLE_DEPRECATED_BEFORE

#include "QtAVDisplay.h"
#include "QtAVVideoToolBar.h"

#include <DB/ImageInfo.h>

#include <KLocalizedString>
#include <QGuiApplication>
#include <QMessageBox>
#include <QPainterPath>
#include <QScreen>
#include <QVBoxLayout>
#include <QtAV/AVPlayer.h>
#include <QtAV/LibAVFilter.h>
#include <QtAV/VideoRenderer.h>
#include <QtAVWidgets/WidgetRenderer.h>
#include <QtAVWidgets/global.h>

Viewer::QtAVDisplay::QtAVDisplay(QWidget *parent)
    : Viewer::VideoDisplay(parent)
{
    m_renderer = new QtAV::WidgetRenderer(this);
    if (!m_renderer) {
        QMessageBox::critical(this, i18n("Failed to set up video playback"), i18n("Failed to set up video playback (renderer wasn't created)"));
        return;
    }

    m_player = new QtAV::AVPlayer(this);

    // I need this for the use case where we click on the slider after end of video playback.
    // If I don't disable asynchronous loading the seek will have no effect.
    // See VideoDisplay::seekToPosition
    m_player->setAsyncLoad(false);

    m_videoWidget = m_renderer->widget();
    if (!m_videoWidget) {
        QMessageBox::critical(this, i18n("Failed to set up video playback"), i18n("Failed to set up video playback (widget wasn't created)"));
        return;
    }
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_videoWidget, 1);

    m_player->setRenderer(m_renderer);

    connect(m_player, &QtAV::AVPlayer::stopped, this, &QtAVDisplay::stopped);

    m_toolBar = new QtAVVideoToolBar(m_player, this);
    layout->addWidget(m_toolBar);

    connect(m_toolBar, &VideoToolBar::positionChanged, this, &QtAVDisplay::seekToPosition);
    connect(m_player, &QtAV::AVPlayer::positionChanged, this, &QtAVDisplay::displayPosition);
    connect(m_toolBar, &VideoToolBar::muted, m_player, [this](bool b) { m_player->audio()->setMute(b); });

    connect(m_toolBar, &VideoToolBar::volumeChanged, m_player, [this](int value) { m_player->audio()->setVolume(value / 100.0); });
    m_toolBar->setVolume(100 * m_player->audio()->volume());

    m_rotateFilter = new QtAV::LibAVFilterVideo(m_player);
}

bool Viewer::QtAVDisplay::setImageImpl(DB::ImageInfoPtr info, bool /*forward*/)
{
    if (!m_videoWidget)
        return false;

    rotate(m_info);

    m_player->play(info->fileName().absolute());
    m_toolBar->closePreview();

    return true;
}

Viewer::QtAVDisplay::~QtAVDisplay()
{
    stop();
}

void Viewer::QtAVDisplay::stop()
{
    if (m_player)
        m_player->stop();
}

void Viewer::QtAVDisplay::playPause()
{
    if (!m_player)
        return;
    m_player->togglePause();
}

QImage Viewer::QtAVDisplay::screenShoot()
{
    return QGuiApplication::primaryScreen()->grabWindow(m_videoWidget->winId()).toImage();
}

void Viewer::QtAVDisplay::restart()
{
    if (!m_player)
        return;

    m_player->seek(m_player->startPosition());
    m_player->play();
}

void Viewer::QtAVDisplay::relativeSeek(int msec)
{
    seekToPosition(msec + m_player->position());
}

void Viewer::QtAVDisplay::seekToPosition(qint64 pos)
{
    if (!m_player->isPlaying())
        m_player->play();
    m_player->setSeekType(QtAV::AccurateSeek);
    m_player->seek(pos);
}

void Viewer::QtAVDisplay::rotate(const DB::ImageInfoPtr &info)
{
    m_info = info;
    m_rotateFilter->uninstall();
    if (info->angle() != 0) {
        m_rotateFilter->installTo(m_player);
        // See https://ffmpeg.org/ffmpeg-filters.html
        m_rotateFilter->setOptions(QString::fromUtf8("rotate=PI*%1/180").arg(m_info->angle()));
    }
}

bool Viewer::QtAVDisplay::isPaused() const
{
    if (!m_player)
        return false;

    return m_player->isPaused();
}

bool Viewer::QtAVDisplay::isPlaying() const
{
    if (!m_player)
        return false;

    return m_player->isPlaying();
}

void Viewer::QtAVDisplay::displayPosition(qint64 pos)
{
    if (m_player->isSeekable())
        m_toolBar->setPosition(pos);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_QtAVDisplay.cpp"
