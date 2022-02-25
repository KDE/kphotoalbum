/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "VideoServer.h"
#include "VideoSharedTypes.h"
#include <Utilities/AlgorithmHelper.h>
#include <QApplication>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QTcpSocket>
#include <QThread>

#if 0
#define videoServerDebug qDebug
#else
#define videoServerDebug \
    if (false)           \
    qDebug
#endif

RemoteControl::VideoServer::~VideoServer()
{
    requestInterruption();
    m_waitCondition.wakeOne();
}

void RemoteControl::VideoServer::connectToTCPServer(const QHostAddress &address)
{
    Q_ASSERT(!address.isNull());
    m_remoteAddress = address;
    start();
}

void RemoteControl::VideoServer::sendVideo(const DB::FileName &fileName, ImageId imageId, bool isPriority)
{
    Q_ASSERT(thread() == qApp->thread());

    videoServerDebug() << "Requesting image id=" << imageId << " isPriority=" << isPriority;
    QMutexLocker dummy(&m_mutex);

    if (!m_requests.isEmpty() && m_requests.constFirst().imageId == imageId) {
        videoServerDebug() << "OK we are already loading that, so nothing more to do.";
    } else if (isPriority) {
        videoServerDebug() << "OK this is a priority request";
        // The request might be further down in the queue, lets remove that requests first
        removeRequest(imageId);
        m_requests.prepend({ fileName, imageId });
    } else
        m_requests.append({ fileName, imageId });
    videoServerDebug() << "Queue is now" << queue();
    m_waitCondition.wakeOne();
}

void RemoteControl::VideoServer::cancelRequest(ImageId imageId)
{
    Q_ASSERT(thread() == qApp->thread());

    videoServerDebug() << "Requesting to remove" << imageId;
    QMutexLocker dummy(&m_mutex);
    removeRequest(imageId);
}

void RemoteControl::VideoServer::removeRequest(ImageId imageId)
{
    // ===============================================================
    //       THE MUTEX MUSTY BE LOCKED WHEN CALLING THIS FUNCITON
    // ===============================================================
    auto it = std::remove_if(m_requests.begin(), m_requests.end(), [&](const auto &item) { return item.imageId == imageId; });
    m_requests.erase(it, m_requests.end());

    videoServerDebug() << "Queue after deleting " << imageId << ":" << queue();
}

QStringList RemoteControl::VideoServer::queue() const
{
    // ===============================================================
    //       THE MUTEX MUSTY BE LOCKED WHEN CALLING THIS FUNCITON
    // ===============================================================
    QStringList result;
    std::transform(m_requests.cbegin(), m_requests.cend(), std::back_inserter(result), [](const auto &item) { return QString::number(item.imageId); });
    return result;
}

void RemoteControl::VideoServer::run()
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
        if (m_socket->state() != QAbstractSocket::ConnectedState)
            return;
        m_socket->waitForBytesWritten();
    };

    Q_FOREVER
    {
        m_mutex.lock();

        if (m_requests.isEmpty())
            m_waitCondition.wait(&m_mutex);

        if (m_socket->state() != QAbstractSocket::ConnectedState || isInterruptionRequested()) {
            m_mutex.unlock();
            return;
        }

        const Request request = m_requests.constFirst();
        m_mutex.unlock();

        QFile file(request.fileName.absolute());
        file.open(QIODevice::ReadOnly);

        videoServerDebug() << "Reading image id" << request.imageId;

        // First send header
        start();
        stream << PackageType::Header << request.imageId << file.size() << QFileInfo(file.fileName()).suffix();
        send();

        for (qint64 offset = 0; offset < file.size(); offset += PACKAGESIZE) {
            {
                QMutexLocker dummy(&m_mutex);
                if (m_socket->state() != QAbstractSocket::ConnectedState || isInterruptionRequested())
                    return;
            }
            start();
            stream << PackageType::Data << file.read(PACKAGESIZE);
            send();
            if (m_socket->state() != QAbstractSocket::ConnectedState)
                break;

            if (offset + PACKAGESIZE >= file.size()) {
                videoServerDebug() << "Done reading" << request.imageId;
                removeRequest(request.imageId);
            } else {
                QMutexLocker dummy(&m_mutex);
                if (m_requests.isEmpty() || request.imageId != m_requests.constFirst().imageId) {
                    videoServerDebug() << "Stopped loading request before complete for imageId" << request.imageId;
                    start();
                    stream << PackageType::Cancel << request.imageId;
                    send();
                    break;
                }
            }
        }
    }
}
