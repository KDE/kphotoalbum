#pragma once

#include <QObject>
#include <QTcpServer>

class Executor : public QTcpServer
{
    Q_OBJECT

public:
    Executor(QObject *parent = nullptr);

signals:
    void newPathFound(const QString &path);

private:
    void startListeningForUDPPackages();
    void readIncommingUDP();
    void connectToTcpServer(const QHostAddress &address);
    void gotConnected();
    void dataReceived();
    void lostConnection();

    class QUdpSocket *m_udpSocket = nullptr;
    QTcpSocket *m_tcpSocket = nullptr;
};
