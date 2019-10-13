#ifndef REMOTEVIDEODISPLAYCONTROLLER_H
#define REMOTEVIDEODISPLAYCONTROLLER_H

#include "DB/ImageInfoPtr.h"
#include <QObject>
#include <QTcpServer>
#include <QTimer>

namespace Viewer
{

class RemoteVideoDisplayController : public QObject
{
public:
    static RemoteVideoDisplayController &instance();
    void display(DB::ImageInfoPtr info);

private:
    RemoteVideoDisplayController();
    void sendBroadcastPackage();
    void acceptConnection();
    void disconnect();

    QTcpServer m_server;
    QTimer m_timer;
    QTcpSocket *m_tcpSocket = nullptr;
    DB::ImageInfoPtr m_pendingRequest;
};

} // namespace Viewer

#endif // REMOTEVIDEODISPLAYCONTROLLER_H
