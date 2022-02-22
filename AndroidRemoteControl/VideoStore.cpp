/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "VideoStore.h"
#include "../RemoteControl/RemoteCommand.h"
#include "RemoteInterface.h"
#include "RemoteVideoInfo.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

namespace RemoteControl
{

VideoStore::VideoStore(QObject *parent)
    : QObject(parent)
{
}

VideoStore &VideoStore::instance()
{
    static VideoStore instance;
    return instance;
}

void VideoStore::requestVideo(RemoteVideoInfo *client, ImageId imageId)
{
    m_requests.insert(imageId, client);
    VideoRequest request(imageId);
    RemoteInterface::instance().sendCommand(request);
}

void VideoStore::setVideo(const VideoResult &result)
{
    // FIXME correct path?
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    QString outputFile = dir.filePath(QString("%1.mp4").arg(result.imageId));
    QFile out(outputFile);
    bool openOut = out.open(QIODevice::WriteOnly);
    auto written = out.write(result.data);
    // FIXME error handling
    m_requests.value(result.imageId)->setUrl(QString("file://%1").arg(outputFile));
}

void VideoStore::setVideos(const QVector<ImageId> &videos)
{
    m_videos = videos;
}

bool VideoStore::isVideo(ImageId imageID) const
{
    return m_videos.contains(imageID);
}

} // namespace RemoteControl
