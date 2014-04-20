#include "Server.h"

#include <QUdpSocket>
#include <QTcpSocket>
#include <QMessageBox>
#include "RemoteCommand.h"
#include <KLocale>

using namespace RemoteControl;

Server::Server(QObject *parent) :
    RemoteConnection(parent)
{
}

bool Server::isConnected() const
{
    return m_isConnected;
}

void Server::listen()
{
    if (!m_socket) {
        m_socket = new QUdpSocket(this);
        bool ok = m_socket->bind(UDPPORT); // PENDING Do we want to bind in a special way (see second argument to bind)
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(readIncommingUDP()));
    }
}

QTcpSocket*Server::socket()
{
    return m_tcpSocket;
}

void Server::readIncommingUDP()
{
    Q_ASSERT(m_socket->hasPendingDatagrams());
    char data[1000];

    QHostAddress address;
    qint64 len = m_socket->readDatagram(data,1000, &address);
    QString string = QString::fromUtf8(data).left(len);
    QStringList list = string.split(QChar::fromAscii(' '));
    if (list[0] != QString::fromUtf8("KPhotoAlbum")) {
        return;
    }
    if (list[1] != QString::number(RemoteControl::VERSION)) {
        QMessageBox::critical(0, i18n("Invalid Version"),
                              i18n("Version mismatch between Remote Client and KPhotoAlbum on the desktop.\n"
                                   "Desktop protocol version: %1\n"
                                   "Remote Control protocol version: %2\n"
                                   "SHUTTING DOWN LISTENING FOR REMOTE CONNECTIONS!")
                              .arg(RemoteControl::VERSION).arg(list[1]));
        deleteLater();
        return;
    }

    connectToTcpServer(address);
}

void Server::connectToTcpServer(const QHostAddress& address)
{
    m_tcpSocket = new QTcpSocket;
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    m_tcpSocket->connectToHost(address, TCPPORT);
}

void Server::connected()
{
    m_isConnected = true;
    emit gotConnection();
}
