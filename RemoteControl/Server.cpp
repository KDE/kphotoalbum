#include "Server.h"

#include <QUdpSocket>
#include <QTcpSocket>

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
        connect(m_socket, &QUdpSocket::readyRead, this, &Server::readIncommingUDP);
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
    if (qstrcmp(data,"SlideViewer") != 0) {
        // Hmmm not from a SlideViewer client
    }

    connectToTcpServer(address);
}

void Server::connectToTcpServer(const QHostAddress& address)
{
    m_tcpSocket = new QTcpSocket;
    connect(m_tcpSocket, &QTcpSocket::connected, this, &Server::connected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Server::dataReceived);
    m_tcpSocket->connectToHost(address, TCPPORT);
}

void Server::connected()
{
    m_isConnected = true;
    emit gotConnection();
}
