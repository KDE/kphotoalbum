#include "RemoteInterface.h"
#include "Client.h"
#include "RemoteCommand.h"

#include <QTcpSocket>
#include <qimage.h>
#include <QLabel>
#include <QBuffer>
#include <QDataStream>

using namespace RemoteControl;

RemoteInterface::RemoteInterface()
{
    m_connection = new Client;
    connect(m_connection, &Client::gotCommand, this, &RemoteInterface::handleCommand);
    connect(m_connection, SIGNAL(connectionChanged()),this, SIGNAL(connectionChanged()));
}

RemoteInterface& RemoteInterface::instance()
{
    static RemoteInterface interface;
    return interface;
}

bool RemoteInterface::isConnected() const
{
    return m_connection->isConnected();
}

void RemoteInterface::previousSlide()
{
    m_connection->sendCommand(PreviousSlideCommand());
}

void RemoteInterface::nextSlide()
{
    m_connection->sendCommand(NextSlideCommand());
}

void RemoteInterface::handleCommand(const RemoteCommand& command)
{
    if (command.id() == ImageUpdateCommand::id())
        emit newImage(static_cast<const ImageUpdateCommand&>(command).image);
    else
        qFatal("Unhandled command");
}
