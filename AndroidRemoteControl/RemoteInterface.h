#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include <QObject>

class RemoteConnection;
class QTcpSocket;
class RemoteCommand;

class RemoteInterface : public QObject
{
    Q_OBJECT

public:
    static RemoteInterface& instance();
    bool isConnected() const;

public slots:
    void previousSlide();
    void nextSlide();

signals:
    void newImage(const QImage&);
    void connectionChanged();

private slots:
    void handleCommand(const RemoteCommand&);

private:
    RemoteInterface();

    RemoteConnection* m_connection = nullptr;
};

#endif // REMOTEINTERFACE_H
