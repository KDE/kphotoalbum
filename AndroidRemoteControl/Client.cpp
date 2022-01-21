// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Client.h"

#include "../RemoteControl/RemoteCommand.h"
#include <QTcpSocket>
#include <QUdpSocket>

using namespace RemoteControl;

Client::Client(QObject *parent)
    : RemoteConnection(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &Client::acceptConnection);
    connect(&m_timer, &QTimer::timeout, this, &Client::sendBroadcastPackage);
    m_server.listen(QHostAddress::Any, TCPPORT);
    m_timer.start(500);
}

bool Client::isConnected() const
{
    return m_socket != nullptr;
}

QTcpSocket *Client::socket()
{
    return m_socket;
}

void Client::acceptConnection()
{
    m_timer.stop();
    m_socket = m_server.nextPendingConnection();
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::disconnect);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::dataReceived);
    Q_EMIT gotConnected();
}

void Client::sendBroadcastPackage()
{
    QUdpSocket socket;
    QByteArray data = QStringLiteral("KPhotoAlbum %1").arg(RemoteControl::VERSION).toUtf8();
    socket.writeDatagram(data, QHostAddress::Broadcast, UDPPORT);
}

void Client::disconnect()
{
    m_timer.start(500);
    m_socket->deleteLater();
    m_socket = nullptr;
    Q_EMIT disconnected();
}

#include "moc_Client.cpp"
