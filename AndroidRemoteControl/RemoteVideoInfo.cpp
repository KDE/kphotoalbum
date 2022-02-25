/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "RemoteVideoInfo.h"
#include "Tracer.h"
#include "VideoStore.h"
#include <QDebug>

namespace RemoteControl
{

RemoteVideoInfo::RemoteVideoInfo(QObject *parent)
    : QObject(parent) {
        TRACE
    }

    RemoteVideoInfo::~RemoteVideoInfo()
{
    TRACE
    if (m_progress < 1)
        VideoStore::instance().cancelRequestFromClient(m_imageId);
}

bool RemoteVideoInfo::active() const
{
    TRACE
    return m_active;
}

void RemoteVideoInfo::setActive(bool newActive)
{
    TRACE
    if (m_active == newActive)
        return;

    if (newActive) {
        Q_ASSERT(m_imageId != -1);
        VideoStore::instance().requestVideo(this, m_imageId);
    }
    m_active = newActive;
    emit activeChanged();
}

const QString &RemoteVideoInfo::url() const
{
    TRACE
    return m_url;
}

void RemoteVideoInfo::setUrl(const QString &url)
{
    TRACE
    if (m_url == url)
        return;
    m_url = url;
    emit urlChanged();
}

int RemoteVideoInfo::imageId() const
{
    TRACE
    return m_imageId;
}

void RemoteVideoInfo::setImageId(int newImageId)
{
    TRACE
    if (m_imageId == newImageId)
        return;
    VideoStore::instance().requestPreHeat(this, newImageId);
    m_imageId = newImageId;
    emit imageIdChanged();
}

void RemoteVideoInfo::setProgress(double progres)
{
    TRACE
    if (m_progress == progres)
        return;
    m_progress = progres;
    emit progressChanged();
}

double RemoteVideoInfo::progress() const
{
    TRACE
    return m_progress;
}

} // namespace RemoteControl
