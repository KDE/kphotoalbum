/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "../RemoteControl/Types.h"
#include <QHash>
#include <QObject>

namespace RemoteControl
{

class VideoResult;
class RemoteVideoInfo;

class VideoStore : public QObject
{
    Q_OBJECT
public:
    static VideoStore &instance();
    void requestVideo(RemoteVideoInfo *client, ImageId imageId);
    void cancelRequestFromClient(ImageId imageId);
    void requestPreHeat(RemoteVideoInfo *client, ImageId imageId);
    void setVideos(const QVector<ImageId> &videos);
    bool isVideo(ImageId imageID) const;
    void addSegment(ImageId imageID, bool firstSegment, int totalSize, const QString &fileSuffix, const QByteArray &data);
    void cancelRequestFromServer(ImageId imageId);

private:
    explicit VideoStore(QObject *parent = nullptr);
    bool hasVideo(ImageId imageId) const;
    void sendURL(RemoteVideoInfo *client, ImageId imageId);
    void makeRequest(RemoteVideoInfo *client, ImageId imageId, bool isPriority);
    void cancelRequest(ImageId imageId);

    QHash<ImageId, RemoteVideoInfo *> m_requests;

    // FIXME should this be a set instead?
    QVector<ImageId> m_videos; // Will only be access on GUI
    QHash<ImageId, QString> m_fileNames;
};

} // namespace RemoteControl
