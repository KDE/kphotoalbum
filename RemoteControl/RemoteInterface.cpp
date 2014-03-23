#include "RemoteInterface.h"
#include "Server.h"
#include "RemoteCommand.h"

#include <QDebug>
#include <QDataStream>
#include <QTcpSocket>
#include <QImage>
#include <QBuffer>
#include <QPainter>
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"

using namespace RemoteControl;

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

void RemoteInterface::sendImage(int index, const QImage& image)
{
    m_connection->sendCommand(ImageUpdateCommand(index, image));
}


void RemoteInterface::sendImageCount(int count)
{
    ImageCountUpdateCommand command;
    command.count = count;
    m_connection->sendCommand(command);
}

void RemoteInterface::handleCommand(const RemoteCommand& command)
{
//    if (command.id() == NextSlideCommand::id())
//        SlideDeckController::instance()->incrementPage(1);
//    else if (command.id() == PreviousSlideCommand::id())
//        SlideDeckController::instance()->incrementPage(-1);
}

void RemoteInterface::sendInitialData()
{
    m_connection->sendCommand(CategoryListCommand(DB::ImageDB::instance()->categoryCollection()->categoryNames()));
}
