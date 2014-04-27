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

#ifndef SERVER_H
#define SERVER_H

#include "RemoteConnection.h"
class QUdpSocket;
class QTcpSocket;

namespace RemoteControl
{
class Server : public RemoteConnection
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    bool isConnected() const override;
    void listen();
    QTcpSocket* socket() override;

signals:
    void gotConnection();

private slots:
    void readIncommingUDP();
    void connectToTcpServer(const QHostAddress& address);
    void connected();

private:
    QUdpSocket* m_socket = nullptr;
    QTcpSocket* m_tcpSocket = nullptr;
    bool m_isConnected = false;
};

}

#endif // SERVER_H
