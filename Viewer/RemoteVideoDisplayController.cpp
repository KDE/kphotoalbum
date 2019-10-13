#include "RemoteVideoDisplayController.h"
#include <DB/ImageInfo.h>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <RemoteControl/Server.h>

static constexpr int VIDEOTCPPORT = 43124;
static constexpr int VIDEOUDPPORT = 43123;

namespace Viewer
{

RemoteVideoDisplayController::RemoteVideoDisplayController()
{
    qDebug("Time to come alive!");
    connect(&m_server, &QTcpServer::newConnection, this, &RemoteVideoDisplayController::acceptConnection);
    connect(&m_timer, &QTimer::timeout, this, &RemoteVideoDisplayController::sendBroadcastPackage);
    m_server.listen(QHostAddress::Any, VIDEOTCPPORT);
    m_timer.start(500);
}

RemoteVideoDisplayController &RemoteVideoDisplayController::instance()
{
    static RemoteVideoDisplayController instance;
    return instance;
}

void RemoteVideoDisplayController::display(DB::ImageInfoPtr info)
{
    if (!m_tcpSocket) {
        m_pendingRequest = info;
        return;
    }
    //    qDebug("OK time to load for real %s", qPrintable(info->fileName().absolute()));
    QTextStream stream(m_tcpSocket);
    stream.setCodec("utf8");
    stream << info->fileName().absolute() << "\r\n";
}

void RemoteVideoDisplayController::sendBroadcastPackage()
{
    QUdpSocket socket;
    QByteArray data = QStringLiteral("KPhotoAlbum VideoConnection").toUtf8();
    socket.writeDatagram(data, QHostAddress::Broadcast, VIDEOUDPPORT);
}

void RemoteVideoDisplayController::acceptConnection()
{
    //    qDebug("Incomming TCP Connection");
    m_timer.stop();
    m_tcpSocket = m_server.nextPendingConnection();
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &RemoteVideoDisplayController::disconnect);
    if (m_pendingRequest)
        display(m_pendingRequest);
    m_pendingRequest = DB::ImageInfoPtr();
}

void RemoteVideoDisplayController::disconnect()
{
    //    qDebug("Disconnect");
    m_tcpSocket = nullptr;
    m_timer.start(500);
}

} // namespace Viewer
