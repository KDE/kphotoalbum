/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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

#include "RemoteConnection.h"
#include "RemoteCommand.h"

#include <QBuffer>
#include <QTcpSocket>
#include <QApplication>
#include <QThread>
#include <QTime>

#if 0
#  define protocolDebug qDebug
#else
#  define protocolDebug if (false) qDebug
#endif

using namespace RemoteControl;

RemoteConnection::RemoteConnection(QObject *parent) :
    QObject(parent)
{
}

void RemoteConnection::sendCommand(const RemoteCommand& command)
{
    protocolDebug() << qPrintable(QTime::currentTime().toString(QString::fromUtf8("hh:mm:ss.zzz")))
                    << ": Sending " << QString::number((int) command.commandType());
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!isConnected())
        return;

    // Stream into a buffer so we can send length of buffer over
    // this is to ensure the remote side gets all data before it
    // starts to demarshal the data.
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);

    // stream a placeholder for the length
    stream << (qint32) 0;

    // Steam the id and the data
    stream << (qint32) command.commandType();
    command.encode(stream);

    // Wind back and stream the length
    stream.device()->seek(0);
    stream << (qint32) buffer.size();

    // Send the data.
    socket()->write(buffer.data());
    socket()->flush();
}

void RemoteConnection::dataReceived()
{
    QTcpSocket* socket = this->socket();
    if (!socket)
        return;

    QDataStream stream(socket);

    while (socket->bytesAvailable()) {
        if (m_state == WaitingForLength) {
            if (socket->bytesAvailable() < (qint64) sizeof(qint32))
                return;

            stream >> m_length;
            m_length -= sizeof(qint32);
            m_state = WaitingForData;
        }

        if (m_state == WaitingForData) {
            if (socket->bytesAvailable() < m_length)
                return;

            m_state = WaitingForLength;
            QByteArray data = socket->read(m_length);
            Q_ASSERT(data.length() == m_length);

            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);
            QDataStream stream(&buffer);
            qint32 id;
            stream >> id;

            std::unique_ptr<RemoteCommand> command = RemoteCommand::create(static_cast<CommandType>(id));
            command->decode(stream);
            protocolDebug() << qPrintable(QTime::currentTime().toString(QString::fromUtf8("hh:mm:ss.zzz")))
                               << ": Received " << id;

            emit gotCommand(*command);
        }
    }
}






