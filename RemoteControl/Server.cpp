// SPDX-FileCopyrightText: 2014-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Server.h"

#include "RemoteCommand.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QTcpSocket>
#include <QUdpSocket>

using namespace RemoteControl;

Server::Server(QObject *parent)
    : RemoteConnection(parent)
{
}

bool Server::isConnected() const
{
    return m_isConnected;
}

void Server::listen(QHostAddress address)
{
    if (!m_socket) {
        m_socket = new QUdpSocket(this);
        bool ok = m_socket->bind(address, UDPPORT);
        if (!ok) {
            QMessageBox::critical(0, i18n("Unable to bind to socket"),
                                  i18n("Unable to listen for remote Android connections. "
                                       "This is likely because you have another KPhotoAlbum application running."));
        }
        connect(m_socket, &QUdpSocket::readyRead, this, &Server::readIncommingUDP);
    }
}

void Server::stopListening()
{
    delete m_socket;
    m_socket = nullptr;
    delete m_tcpSocket;
    m_tcpSocket = nullptr;
    Q_EMIT stoppedListening();
}

QTcpSocket *Server::socket()
{
    return m_tcpSocket;
}

void Server::readIncommingUDP()
{
    Q_ASSERT(m_socket->hasPendingDatagrams());
    char data[1000];

    QHostAddress address;
    qint64 len = m_socket->readDatagram(data, 1000, &address);
    QString string = QString::fromUtf8(data).left(len);
    QStringList list = string.split(QChar::fromLatin1(' '));
    if (list[0] != QString::fromUtf8("KPhotoAlbum")) {
        return;
    }
    if (list[1] != QString::number(RemoteControl::VERSION)) {
        QMessageBox::critical(0, i18n("Invalid Version"),
                              i18n("Version mismatch between Remote Client and KPhotoAlbum on the desktop.\n"
                                   "Desktop protocol version: %1\n"
                                   "Remote Control protocol version: %2",
                                   RemoteControl::VERSION,
                                   list[1]));
        stopListening();
        return;
    }

    connectToTcpServer(address);
}

void Server::connectToTcpServer(const QHostAddress &address)
{
    m_tcpSocket = new QTcpSocket;
    connect(m_tcpSocket, &QTcpSocket::connected, this, &Server::gotConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Server::dataReceived);
    m_tcpSocket->connectToHost(address, TCPPORT);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &Server::lostConnection);
}

void Server::gotConnected()
{
    m_isConnected = true;
    Q_EMIT connected();
}

void Server::lostConnection()
{
    m_isConnected = false;
    Q_EMIT disConnected();
}

#include "moc_Server.cpp"
