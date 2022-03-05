/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "VideoStore.h"
#include "../RemoteControl/RemoteCommand.h"
#include "RemoteInterface.h"
#include "RemoteVideoInfo.h"
#include "Tracer.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

namespace RemoteControl
{

VideoStore::VideoStore(QObject *parent)
    : QObject(parent)
{
    purgeCache();
}

bool VideoStore::hasVideo(ImageId imageId) const
{
    TRACE
    return m_fileNames.contains(imageId) && !m_requests.contains(imageId);
}

void VideoStore::sendURL(RemoteVideoInfo *client, ImageId imageId)
{
    TRACE
    client->setUrl(QString("file://%1").arg(m_fileNames.value(imageId)));
    client->setProgress(1);
}

void VideoStore::makeRequest(RemoteVideoInfo *client, ImageId imageId, bool isPriority)
{
    TRACE
    if (hasVideo(imageId)) {
        sendURL(client, imageId);
        return;
    }
    if (m_requests.contains(imageId) && !isPriority)
        return;

    m_requests[imageId] = client;
    VideoRequest request(imageId, isPriority);
    RemoteInterface::instance().sendCommand(request);
}

VideoStore &VideoStore::instance()
{
    TRACE
    static VideoStore instance;
    return instance;
}

void VideoStore::requestVideo(RemoteVideoInfo *client, ImageId imageId)
{
    TRACE
    makeRequest(client, imageId, true);
}

void VideoStore::cancelRequestFromClient(ImageId imageId)
{
    TRACE
    if (m_requests.contains(imageId))
        RemoteInterface::instance().cancelVideoRequest(imageId);
    cancelRequest(imageId);
    // Actual clean up will happen when the server responds.
}

void VideoStore::cancelRequestFromServer(ImageId imageId)
{
    TRACE
    cancelRequest(imageId);
}

void VideoStore::cancelRequest(ImageId imageId)
{
    TRACE
    if (!m_requests.contains(imageId))
        return;
    m_requests.remove(imageId);
    const QString fileName = m_fileNames.value(imageId);
    if (!fileName.isNull())
        QFile::remove(fileName);
    m_fileNames.remove(imageId);
}

void VideoStore::purgeCache()
{
    TRACE
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    for (const auto &file : dir.entryList({ "_KPhotoAlbum_*" }, QDir::Files)) {
        dir.remove(file);
    }
}

void VideoStore::requestPreHeat(RemoteVideoInfo *client, ImageId imageId)
{
    TRACE
    makeRequest(client, imageId, false);
}

void VideoStore::setVideos(const QVector<ImageId> &videos)
{
    TRACE
    m_videos = videos;
}

bool VideoStore::isVideo(ImageId imageID) const
{
    TRACE
    return m_videos.contains(imageID);
}

void VideoStore::addSegment(ImageId imageID, bool firstSegment, int totalSize, const QString &fileSuffix, const QByteArray &data)
{
    TRACE
    RemoteVideoInfo *client = m_requests.value(imageID, nullptr);
    if (!client) {
        // This is a segment for that no longer is visible. This happens if I ask for a preload and then switches somewhere else.
        return;
    }

    // FIXME correct path?
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    QString outputFile = dir.filePath(QString("_KPhotoAlbum_%1.%2").arg(imageID).arg(fileSuffix));
    QFile out(outputFile);
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    if (!firstSegment)
        mode.setFlag(QIODevice::Append, true);
    bool outOpen = out.open(mode);
    auto written = out.write(data);
    // FIXME error handling

    client->setProgress(out.size() * 1.0 / totalSize);
    m_fileNames.insert(imageID, outputFile);
    if (totalSize == out.size()) {
        sendURL(m_requests.value(imageID), imageID);
        m_requests.remove(imageID);
    }
}

} // namespace RemoteControl
