/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Client.h"

#include "RemoteCommand.h"
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
    emit gotConnected();
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
    emit disconnected();
}
