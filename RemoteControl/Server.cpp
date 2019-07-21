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
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(readIncommingUDP()));
    }
}

void Server::stopListening()
{
    delete m_socket;
    m_socket = nullptr;
    delete m_tcpSocket;
    m_tcpSocket = nullptr;
    emit stoppedListening();
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
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(gotConnected()));
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    m_tcpSocket->connectToHost(address, TCPPORT);
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(lostConnection()));
}

void Server::gotConnected()
{
    m_isConnected = true;
    emit connected();
}

void Server::lostConnection()
{
    m_isConnected = false;
    emit disConnected();
}
