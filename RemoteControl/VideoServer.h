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
    explicit VideoServer(QObject *parent = nullptr);
    void connectToTCPServer(const QHostAddress &address);
    void sendVideo(const DB::FileName &fileName, ImageId imageId);
    void cancelRequest(ImageId imageId);

protected:
    void run() override;

private:
    QTcpSocket *m_socket = nullptr;

    struct Request {
        DB::FileName fileName;
        ImageId imageId;
    };

    QVector<Request> m_requests;
    ImageId m_loading = -1;

    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    QHostAddress m_remoteAddress;
};
} // namespace RemoteControl
