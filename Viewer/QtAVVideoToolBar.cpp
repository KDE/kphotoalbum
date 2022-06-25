/* SPDX-FileCopyrightText: 2021 The KPhotoAlbum Development Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "QtAVVideoToolBar.h"

#include <QtAV/AVPlayer.h>
#include <QtAVWidgets/VideoPreviewWidget.h>

namespace Viewer
{

QtAVVideoToolBar::QtAVVideoToolBar(QtAV::AVPlayer *player, QWidget *parent)
    : VideoToolBar(parent)
    , m_player(player)
{
    connect(m_player, &QtAV::AVPlayer::started, this, &QtAVVideoToolBar::slotVideoStarted);
}

void QtAVVideoToolBar::onTimeSliderHover(const QPoint &pos, int value)
{
    VideoToolBar::onTimeSliderHover(pos, value);

    if (!m_preview)
        m_preview = new QtAV::VideoPreviewWidget();
    m_preview->setFile(m_player->file());
    m_preview->setTimestamp(value);
    m_preview->preview();

    const int width = 150;
    const int height = [&] {
        const int w = m_player->statistics().video_only.coded_width;
        const int h = m_player->statistics().video_only.coded_height;
        if (w != 0 && h != 0)
            return static_cast<int>(1.0 * width * h / w);
        else
            return width;
    }();

    m_preview->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_preview->resize(width, height);
    m_preview->move(pos - QPoint(width / 2, height));
    m_preview->show();
}

void QtAVVideoToolBar::closePreview()
{
    if (!m_preview)
        return;

    if (m_preview->isVisible())
        m_preview->close();

    delete m_preview;
    m_preview = nullptr;
}

void QtAVVideoToolBar::slotVideoStarted()
{
    setRange(m_player->mediaStartPosition(), m_player->mediaStopPosition());
    setPosition(m_player->mediaStopPosition());
    setSeekSliderEnabled(m_player->isSeekable());
}

} // namespace Viewer

#include "moc_QtAVVideoToolBar.cpp"
