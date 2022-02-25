/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "VideoServer.h"
#include "VideoSharedTypes.h"
#include <Utilities/AlgorithmHelper.h>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QTcpSocket>
#include <QThread>

namespace RemoteControl
{

VideoServer::VideoServer(QObject *parent)
    : QThread(parent)
{
    // FIXME kill?
}

void VideoServer::connectToTCPServer(const QHostAddress &address)
{
    Q_ASSERT(!address.isNull());
    m_remoteAddress = address;
    start();
}

void VideoServer::sendVideo(const DB::FileName &fileName, ImageId imageId)
{
    QMutexLocker dummy(&m_mutex);
    m_requests.append({ fileName, imageId });
    m_waitCondition.wakeOne();
}

void VideoServer::cancelRequest(ImageId imageId)
{
    QMutexLocker dummy(&m_mutex);
    if (m_loading == imageId)
        m_loading = -1;
}

void VideoServer::run()
{
    m_socket = new QTcpSocket;
    m_socket->connectToHost(m_remoteAddress, VIDEOPORT);
    m_socket->waitForConnected();

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);

    auto start = [&] {
        buffer.buffer().clear();
        stream.device()->seek(0);
        stream << (qint32)0;
    };

    auto send = [&] {
        stream.device()->seek(0);
        stream << (qint32)buffer.size();
        m_socket->write(buffer.data());
        m_socket->flush();
        m_socket->waitForBytesWritten();
    };

    Q_FOREVER
    {
        m_mutex.lock();
        if (m_requests.isEmpty())
            m_waitCondition.wait(&m_mutex);

        Request request = m_requests.takeFirst();
        m_loading = request.imageId;
        m_mutex.unlock();

        QFile file(request.fileName.absolute());
        file.open(QIODevice::ReadOnly);

        // First send header
        start();
        stream << PackageType::Header;
        send();

        int id = 0;
        for (qint64 offset = 0; offset < file.size(); offset += PACKAGESIZE) {
            start();
            stream << PackageType::Data;
            stream << request.imageId;
            stream << ++id;
            stream << file.read(PACKAGESIZE);
            send();
            QMutexLocker dummy(&m_mutex);
            if (m_loading != request.imageId) {
                break;
            }
        }
    }
}

} // namespace RemoteControl
