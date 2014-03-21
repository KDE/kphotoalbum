#ifndef CLIENT_H
#define CLIENT_H

#include "RemoteConnection.h"
#include <QTcpServer>
#include <QTimer>

namespace RemoteControl {

class Client : public RemoteConnection
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    bool isConnected() const override;

signals:
    void connectionChanged();

protected:
    QTcpSocket* socket() override;

private slots:
    void acceptConnection();
    void sendBroadcastPackage();
    void disconnect();

private:
    QTcpServer m_server;
    QTcpSocket* m_socket = nullptr;
    QTimer m_timer;

};

}
#endif // CLIENT_H
