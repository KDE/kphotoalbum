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

void VideoStore::setVideos(const QVector<ImageId> &videos)
{
    m_videos = videos;
}

bool VideoStore::isVideo(ImageId imageID) const
{
    return m_videos.contains(imageID);
}

void VideoStore::addSegment(ImageId imageID, bool firstSegment, int totalSize, const QByteArray &data)
{
    // FIXME correct path?
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    QString outputFile = dir.filePath(QString("%1.mp4").arg(imageID));
    QFile out(outputFile);
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    if (!firstSegment)
        mode.setFlag(QIODevice::Append, true);
    bool outOpen = out.open(mode);
    auto written = out.write(data);
    // FIXME error handling

    if (totalSize == out.size())
        m_requests.value(imageID)->setUrl(QString("file://%1").arg(outputFile));
}

} // namespace RemoteControl
