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

#include "Executor.h"
#include "MainWindow.h"
#include "PathMapper.h"
#include <QApplication>
#include <QDesktopServices>
#include <QHostAddress>
#include <QMessageBox>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QUrl>

constexpr int VIDEOUDPPORT = 43123;
constexpr int VIDEOTCPPORT = 43124;

Executor::Executor(QObject *parent)
    : QTcpServer(parent)
{
    startListeningForUDPPackages();
}

void Executor::startListeningForUDPPackages()
{
    MainWindow::addMessage("Listening for UDP connection");
    m_udpSocket = new QUdpSocket(this);
    bool ok = m_udpSocket->bind(QHostAddress::Any, VIDEOUDPPORT);
    if (!ok) {
        QMessageBox::critical(nullptr, "Unable to bind to socket",
                              "Unable to listen for remote Video Display connections. "
                              "This is likely because you have another KPhotoAlbum application running.");
        qApp->quit();
    }
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &Executor::readIncommingUDP);
}

void Executor::readIncommingUDP()
{
    MainWindow::addMessage("Received UDP package");
    Q_ASSERT(m_udpSocket->hasPendingDatagrams());
    char data[1000];

    QHostAddress address;
    qlonglong len = m_udpSocket->readDatagram(data, 1000, &address);
    QString string = QString::fromUtf8(data).left(static_cast<int>(len));
    if (string != QString::fromUtf8("KPhotoAlbum VideoConnection")) {
        QMessageBox::critical(nullptr, "Weird incomming connection", "I'm slightly confused, I received a connection attempt, but it didn't seem to have been from KPhotoAlbum. Ohh will continouing to listen.", QMessageBox::Ok);
        return;
    }

    connectToTcpServer(address);
    m_udpSocket->close();
}

void Executor::connectToTcpServer(const QHostAddress &address)
{
    MainWindow::addMessage("Connecting to KPHotoAlbum");
    m_tcpSocket = new QTcpSocket;
    connect(m_tcpSocket, &QTcpSocket::connected, this, &Executor::gotConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Executor::dataReceived);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &Executor::lostConnection);
    m_tcpSocket->connectToHost(address, VIDEOTCPPORT);
}

void Executor::dataReceived()
{
    while (m_tcpSocket->canReadLine()) {
        QString line = m_tcpSocket->readLine();
        line = line.left(line.length() - 2);
        MainWindow::addMessage(QString("Requested %1").arg(line));

        QString hostPath = PathMapper::instance().map(line);
        if (!hostPath.isNull()) {
            MainWindow::addMessage(QString("Showing video: %1").arg(hostPath));
            QDesktopServices::openUrl(QUrl::fromLocalFile(hostPath));
        }
    }
}
void Executor::gotConnected()
{
    MainWindow::addMessage("Connected to KPhotoAlbum");
}

void Executor::lostConnection()
{
    MainWindow::addMessage("Lost Connection to KPhotoAlbum");
    startListeningForUDPPackages();
}
