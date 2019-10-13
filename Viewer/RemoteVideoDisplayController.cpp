/* Copyright (C) 2003-2019 Jesper K. Pedersen <blackie@kde.org>

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
