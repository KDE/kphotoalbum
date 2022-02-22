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
    void setVideo(const VideoResult &result);
    void setVideos(const QVector<ImageId> &videos);
    bool isVideo(ImageId imageID) const;

private:
    explicit VideoStore(QObject *parent = nullptr);
    QHash<ImageId, RemoteVideoInfo *> m_requests;

    // FIXME should this be a set instead?
    QVector<ImageId> m_videos; // Will only be access on GUI
};

} // namespace RemoteControl
