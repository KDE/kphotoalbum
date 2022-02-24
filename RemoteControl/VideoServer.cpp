#include "VideoServer.h"
#include "VideoSharedTypes.h"
#include <Utilities/AlgorithmHelper.h>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QTcpSocket>

namespace RemoteControl
{

VideoServer::VideoServer(QObject *parent)
    : QObject(parent)
{
    // FIXME kill?
}

void VideoServer::connectToTCPServer(const QHostAddress &address)
{
    Q_ASSERT(!address.isNull());
    m_socket = new QTcpSocket;
    connect(m_socket, &QTcpSocket::connected, this, &VideoServer::gotConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &VideoServer::dataReceived);
    connect(m_socket, &QTcpSocket::disconnected, this, &VideoServer::lostConnection);
    m_socket->connectToHost(address, VIDEOPORT);
}

void VideoServer::sendVideo(const DB::FileName &fileName, ImageId imageId)
{
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
    };

    QFile file(fileName.absolute());
    file.open(QIODevice::ReadOnly);
    {
        start();
        stream << PackageType::Header;
        stream << imageId;
        stream << (qint64)file.size();
        send();
    }

    for (qint64 offset = 0; offset < file.size(); offset += PACKAGESIZE) {
        start();
        stream << PackageType::Data;
        stream << file.read(PACKAGESIZE);
        send();
    }
}

void VideoServer::gotConnected()
{
    // FIXME: implement
}

void VideoServer::dataReceived()
{
    // FIXME: Delete ?
}

void VideoServer::lostConnection()
{
    // FIXME: implement
}

} // namespace RemoteControl
