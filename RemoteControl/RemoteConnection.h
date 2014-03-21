#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include <QObject>
#include <QHostAddress>

class QUdpSocket;
class QTcpSocket;
class RemoteCommand;

class RemoteConnection : public QObject
{
    Q_OBJECT
public:
    const int UDPPORT = 23455;
    const int TCPPORT = 23456;
    explicit RemoteConnection(QObject *parent = 0);
    virtual bool isConnected() const = 0;
    void sendCommand( const RemoteCommand& );

signals:
    void gotCommand(const RemoteCommand&);

protected slots:
    void dataReceived();

protected:
    virtual QTcpSocket* socket() = 0;

private:
    enum ReadingState {WaitingForLength, WaitingForData};
    ReadingState m_state = WaitingForLength;
    qint32 m_length;
};

#endif // REMOTECONNECTION_H
