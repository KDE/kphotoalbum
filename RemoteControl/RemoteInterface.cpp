#include "RemoteInterface.h"
#include "Server.h"
#include "RemoteCommand.h"

#include <QDebug>
#include <QDataStream>
#include <QTcpSocket>
#include <QImage>
#include <QBuffer>
#include <QPainter>

RemoteInterface& RemoteInterface::instance()
{
    static RemoteInterface instance;
    return instance;
}

RemoteInterface::RemoteInterface(QObject *parent) :
    QObject(parent), m_connection(new Server(this))
{
    m_connection->listen();
    connect(m_connection, SIGNAL(gotCommand(RemoteCommand)), this, SLOT(handleCommand(RemoteCommand)));
    connect(m_connection, SIGNAL(gotConnection()), this, SLOT(sendInitialData()));
}

void RemoteInterface::sendImage(const QImage& image)
{
    ImageUpdateCommand imageCommand;
    imageCommand.image = image;
    m_connection->sendCommand(imageCommand);
}

void RemoteInterface::sendPage()
{
//    // PENDING(blackie) This crashes if the window is not visible
//    QImage image = View::instance()->grabWindow();
//    QImage res(1024,768,image.format());
//    QPainter p(&res);
//    p.drawImage(0,0,image);
//    p.end();
//    sendImage(res);
}

void RemoteInterface::handleCommand(const RemoteCommand& command)
{
    qDebug("Got remote command!");
//    if (command.id() == NextSlideCommand::id())
//        SlideDeckController::instance()->incrementPage(1);
//    else if (command.id() == PreviousSlideCommand::id())
//        SlideDeckController::instance()->incrementPage(-1);
}

void RemoteInterface::sendInitialData()
{
    qDebug("GOT CONNECTION");
    sendPage();
}
