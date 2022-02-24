// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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
    void listen(const QHostAddress &address);
    void stopListening();
    QTcpSocket *socket() override;
    void connectToTcpServer(const QHostAddress &address);
    QHostAddress remoteAddress() const;

Q_SIGNALS:
    void connected();
    void disConnected();
    void stoppedListening();

private Q_SLOTS:
    void readIncommingUDP();
    void gotConnected();
    void lostConnection();

private:
    QUdpSocket *m_socket = nullptr;
    QTcpSocket *m_tcpSocket = nullptr;
    bool m_isConnected = false;
    QHostAddress m_remoteAddress;
};

}

#endif // SERVER_H
