/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "RemoteVideoInfo.h"
#include "VideoStore.h"
#include <QDebug>

namespace RemoteControl
{

RemoteVideoInfo::RemoteVideoInfo(QObject *parent)
    : QObject(parent)
{
}

bool RemoteVideoInfo::active() const
{
    return m_active;
}

void RemoteVideoInfo::setActive(bool newActive)
{
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
    return m_url;
}

void RemoteVideoInfo::setUrl(const QString &url)
{
    if (m_url == url)
        return;
    m_url = url;
    emit urlChanged();
}

int RemoteVideoInfo::imageId() const
{
    return m_imageId;
}

void RemoteVideoInfo::setImageId(int newImageId)
{
    if (m_imageId == newImageId)
        return;
    m_imageId = newImageId;
    emit imageIdChanged();
}

void RemoteVideoInfo::setProgress(double progres)
{
    if (m_progress == progres)
        return;
    m_progress = progres;
    emit progressChanged();
}

double RemoteVideoInfo::progress() const
{
    return m_progress;
}

} // namespace RemoteControl
