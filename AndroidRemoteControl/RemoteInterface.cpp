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
    : m_categories(new CategoryModel(this))
{
    m_connection = new Client;
    connect(m_connection, &Client::gotCommand, this, &RemoteInterface::handleCommand);
    connect(m_connection, SIGNAL(connectionChanged()),this, SIGNAL(connectionChanged()));
    qRegisterMetaType<RemoteControl::CategoryModel*>("RemoteControl::CategoryModel*");
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

QImage RemoteInterface::image(int index) const
{
    if (m_imageMap.contains(index))
        return m_imageMap[index];
    else {
        QImage image(1024,768, QImage::Format_RGB32);
        image.fill(Qt::blue);
        return image;
    }
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
        updateImage(static_cast<const ImageUpdateCommand&>(command));
    else if (command.id() == ImageCountUpdateCommand::id())
        updateImageCount(static_cast<const ImageCountUpdateCommand&>(command));
    else if (command.id() == CategoryListCommand::id())
        updateCategoryList(static_cast<const CategoryListCommand&>(command));
    else
        qFatal("Unhandled command");
}

void RemoteInterface::updateImage(const ImageUpdateCommand& command)
{
    m_imageMap[command.index] = command.image;
    emit imageUpdated(command.index);
}

void RemoteInterface::updateImageCount(const ImageCountUpdateCommand& command)
{
    m_imageMap.clear();
    m_imageCount = command.count;
    emit imageCountChanged();

}

void RemoteInterface::updateCategoryList(const CategoryListCommand& command)
{
    m_categories->setCategories(command.categories);
    m_homeImage = command.home;
    emit homeImageChanged();
}
