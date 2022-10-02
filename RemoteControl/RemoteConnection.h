// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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

Q_SIGNALS:
    void gotCommand(const RemoteCommand &);

protected Q_SLOTS:
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
