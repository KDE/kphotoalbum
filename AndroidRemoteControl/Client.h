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

#ifndef CLIENT_H
#define CLIENT_H

#include "RemoteConnection.h"
#include <QTcpServer>
#include <QTimer>

namespace RemoteControl
{

class Client : public RemoteConnection
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    bool isConnected() const override;

signals:
    void gotConnected();
    void disconnected();

protected:
    QTcpSocket *socket() override;

private slots:
    void acceptConnection();
    void sendBroadcastPackage();
    void disconnect();

private:
    QTcpServer m_server;
    QTcpSocket *m_socket = nullptr;
    QTimer m_timer;
};

}
#endif // CLIENT_H
