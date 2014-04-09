#include "RemoteConnection.h"
#include "RemoteCommand.h"

#include <QBuffer>
#include <QTcpSocket>
#include <QApplication>
#include <QThread>

using namespace RemoteControl;

RemoteConnection::RemoteConnection(QObject *parent) :
    QObject(parent)
{
}

void RemoteConnection::sendCommand(const RemoteCommand& command)
{
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
    stream << command.id();
    command.encode(stream);

    // Wind back and stream the length
    stream.device()->seek(0);
    stream << (qint32) buffer.size();

    // Send the data.
    socket()->write(buffer.data());
}

void RemoteConnection::dataReceived()
{
    QDataStream stream(socket());

    while (socket()->bytesAvailable()) {
        if (m_state == WaitingForLength) {
            if (socket()->bytesAvailable() < (qint64) sizeof(qint32))
                return;

            stream >> m_length;
            m_length -= sizeof(qint32);
            m_state = WaitingForData;
        }

        if (m_state == WaitingForData) {
            if (socket()->bytesAvailable() < m_length)
                return;

            m_state = WaitingForLength;
            QByteArray data = socket()->read(m_length);
            Q_ASSERT(data.length() == m_length);

            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);
            QDataStream stream(&buffer);
            QString id;
            stream >> id;

            RemoteCommand& command = RemoteCommand::command(id);
            command.decode(stream);

            emit gotCommand(command);
        }
    }
}






