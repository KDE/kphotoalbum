#include "VideoClient.h"
#include "../RemoteControl/VideoSharedTypes.h"
#include "../Utilities/AlgorithmHelper.h"
#include "VideoStore.h"
#include <QBuffer>
#include <QDataStream>
#include <QTcpServer>
#include <QTcpSocket>

namespace RemoteControl
{
VideoClient::VideoClient(QObject *parent)
    : QObject(parent)
{
    setupTCPServer();
}

void VideoClient::setupTCPServer()
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &VideoClient::acceptConnection);
    connect(m_server, &QTcpServer::acceptError, this, [](auto err) { qDebug() << "VideoServer error: " << err; });
    m_server->listen(QHostAddress::Any, VIDEOPORT);
}

void VideoClient::acceptConnection()
{
    m_socket = m_server->nextPendingConnection();
    connect(m_socket, &QTcpSocket::disconnected, this, &VideoClient::disconnect);
    connect(m_socket, &QTcpSocket::readyRead, this, &VideoClient::dataReceived);
}

void VideoClient::disconnect()
{
    m_socket = nullptr;
    // FIXME: I'm sure I need to cancel some sending here
}

void VideoClient::dataReceived()
{
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
            case PackageType::Header:
                stream >> m_imageId;
                quint64 size;
                stream >> size;
                m_fileSize = size;
                m_currentDataReceived = 0;
                break;
            case PackageType::Data:
                QByteArray data;
                stream >> data;
                VideoStore::instance().addSegment(m_imageId, m_currentDataReceived == 0, m_fileSize, data);
                m_currentDataReceived += data.size();
            }
        }
    }
}
}
