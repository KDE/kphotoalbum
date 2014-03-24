#include "RemoteInterface.h"
#include "Server.h"
#include "RemoteCommand.h"

#include <QDebug>
#include <QDataStream>
#include <QTcpSocket>
#include <QImage>
#include <QBuffer>
#include <QPainter>
#include <kiconloader.h>
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "DB/CategoryPtr.h"
#include "DB/Category.h"

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
    if (command.id() == RequestCategoryInfo::id()) {
        const RequestCategoryInfo& requestCommand = static_cast<const RequestCategoryInfo&>(command);
        if (requestCommand.type == RequestCategoryInfo::RequestCategoryName)
            sendCategoryNames(requestCommand);
        else
            sendCategoryValues(requestCommand);
    }
}

void RemoteInterface::sendCategoryNames(const RequestCategoryInfo& searchInfo)
{
    const int THUMBNAILSIZE = 70;
    CategoryListCommand command;
    for (const DB::CategoryPtr& category : DB::ImageDB::instance()->categoryCollection()->categories()) {
        command.categories.append({category->name(), category->text(), category->icon(THUMBNAILSIZE).toImage()});
    }

    QPixmap homeIcon = KIconLoader::global()->loadIcon( QString::fromUtf8("go-home"), KIconLoader::Desktop, THUMBNAILSIZE);
    command.home = homeIcon.toImage();

    QPixmap kphotoalbumIcon = KIconLoader::global()->loadIcon( QString::fromUtf8("kphotoalbum"), KIconLoader::Desktop, THUMBNAILSIZE);
    command.kphotoalbum = kphotoalbumIcon.toImage();

    m_connection->sendCommand(command);
}

void RemoteInterface::sendCategoryValues(const RequestCategoryInfo& searchInfo)
{

}
