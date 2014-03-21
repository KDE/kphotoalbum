#ifndef SERVER_H
#define SERVER_H

#include "RemoteConnection.h"
class QUdpSocket;
class QTcpSocket;

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

#endif // SERVER_H
