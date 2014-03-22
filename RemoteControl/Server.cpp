#include "Server.h"

#include <QUdpSocket>
#include <QTcpSocket>

using namespace RemoteControl;

Server::Server(QObject *parent) :
    RemoteConnection(parent)
{
}

bool Server::isConnected() const
{
    return m_isConnected;
}

void Server::listen()
{
    if (!m_socket) {
        m_socket = new QUdpSocket(this);
        bool ok = m_socket->bind(UDPPORT); // PENDING Do we want to bind in a special way (see second argument to bind)
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(readIncommingUDP()));
    }
}

QTcpSocket*Server::socket()
{
    return m_tcpSocket;
}

void Server::readIncommingUDP()
{
    Q_ASSERT(m_socket->hasPendingDatagrams());
    char data[12];

    QHostAddress address;
    m_socket->readDatagram(data,12, &address);
    if (qstrcmp(data,"KPhotoAlbum") != 0) {
        // Hmmm not from a KPhotoAlbum client
    }

    connectToTcpServer(address);
}

void Server::connectToTcpServer(const QHostAddress& address)
{
    m_tcpSocket = new QTcpSocket;
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    m_tcpSocket->connectToHost(address, TCPPORT);
}

void Server::connected()
{
    m_isConnected = true;
    emit gotConnection();
}
