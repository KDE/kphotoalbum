/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "Types.h"
#include <QHostAddress>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <kpabase/FileName.h>

class QTcpSocket;

namespace RemoteControl
{
class VideoServer : public QThread
{
    Q_OBJECT
public:
    using QThread::QThread;
    ~VideoServer();
    void connectToTCPServer(const QHostAddress &address);
    void sendVideo(const DB::FileName &fileName, ImageId imageId, bool isPriority);
    void cancelRequest(ImageId imageId);
    void requestCloseDown();

protected:
    void run() override;

private:
    QTcpSocket *m_socket = nullptr;
    void removeRequest(ImageId imageId);
    QStringList queue() const;

    struct Request {
        DB::FileName fileName;
        ImageId imageId;
    };

    QVector<Request> m_requests;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    QHostAddress m_remoteAddress;
};
} // namespace RemoteControl
