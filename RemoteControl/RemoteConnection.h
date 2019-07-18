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

#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include <QHostAddress>
#include <QObject>

class QUdpSocket;
class QTcpSocket;

namespace RemoteControl
{
class RemoteCommand;

class RemoteConnection : public QObject
{
    Q_OBJECT
public:
    const int UDPPORT = 23455;
    const int TCPPORT = 23456;
    explicit RemoteConnection(QObject *parent = 0);
    virtual bool isConnected() const = 0;
    void sendCommand(const RemoteCommand &);

signals:
    void gotCommand(const RemoteCommand &);

protected slots:
    void dataReceived();

protected:
    virtual QTcpSocket *socket() = 0;

private:
    enum ReadingState { WaitingForLength,
                        WaitingForData };
    ReadingState m_state = WaitingForLength;
    qint32 m_length;
};

}

#endif // REMOTECONNECTION_H
