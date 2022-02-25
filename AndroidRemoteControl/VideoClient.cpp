/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "VideoClient.h"
#include "../RemoteControl/VideoSharedTypes.h"
#include "../Utilities/AlgorithmHelper.h"
#include "Tracer.h"
#include "VideoStore.h"
#include <QBuffer>
#include <QDataStream>
#include <QTcpServer>
#include <QTcpSocket>

RemoteControl::VideoClient::VideoClient(QObject *parent)
    : QObject(parent)
{
    TRACE
    setupTCPServer();
}

void RemoteControl::VideoClient::setupTCPServer()
{
    TRACE
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &VideoClient::acceptConnection);
    connect(m_server, &QTcpServer::acceptError, this, [](auto err) { qDebug() << "VideoServer error: " << err; });
    m_server->listen(QHostAddress::Any, VIDEOPORT);
}

void RemoteControl::VideoClient::acceptConnection()
{
    TRACE
    m_socket = m_server->nextPendingConnection();
    connect(m_socket, &QTcpSocket::disconnected, this, &VideoClient::disconnect);
    connect(m_socket, &QTcpSocket::readyRead, this, &VideoClient::dataReceived);
}

void RemoteControl::VideoClient::disconnect()
{
    TRACE
    m_socket = nullptr;
    // FIXME: I'm sure I need to cancel some sending here
}

void RemoteControl::VideoClient::dataReceived()
{
    TRACE
    // FIXME introduce AlignedBuffer to avoid code duplication
    QDataStream stream(m_socket);
    while (m_socket->bytesAvailable()) {
        if (m_state == WaitingForLength) {
            if (m_socket->bytesAvailable() < (qint64)sizeof(qint32))
                return;

            stream >> m_length;
            m_length -= sizeof(qint32);
            m_state = WaitingForData;
        }

        if (m_state == WaitingForData) {
            if (m_socket->bytesAvailable() < m_length)
                return;

            m_state = WaitingForLength;
            QByteArray data = m_socket->read(m_length);
            Q_ASSERT(data.length() == m_length);

            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);
            QDataStream stream(&buffer);

            PackageType type;
            stream >> type;
            switch (type) {
            case PackageType::Header: {
                quint64 size;
                stream >> m_imageId >> size >> m_suffix;
                m_fileSize = size;
                m_currentDataReceived = 0;
                break;
            }
            case PackageType::Data: {
                QByteArray data;
                stream >> data;
                VideoStore::instance().addSegment(m_imageId, m_currentDataReceived == 0, m_fileSize, m_suffix, data);
                m_currentDataReceived += data.size();
                break;
            }
            case PackageType::Cancel: {
                qDebug() << "======================================= GOT THE CANCEL ==============================================";
                ImageId imageId;
                stream >> imageId;
                VideoStore::instance().serverCanceledRequest(imageId);
            }
            }
        }
    }
}
