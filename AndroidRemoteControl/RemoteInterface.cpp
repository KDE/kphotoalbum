#include "RemoteInterface.h"
#include "Client.h"
#include "RemoteCommand.h"
#include "ImageStore.h"

#include <QTcpSocket>
#include <qimage.h>
#include <QLabel>
#include <QBuffer>
#include <QDataStream>
#include <ScreenInfo.h>

using namespace RemoteControl;

RemoteInterface::RemoteInterface()
    : m_categories(new CategoryModel(this)), m_categoryItems(new CategoryItemsModel(this))
{
    m_connection = new Client;
    connect(m_connection, SIGNAL(gotCommand(RemoteCommand)), this, SLOT(handleCommand(RemoteCommand)));
    connect(m_connection, &Client::connectionChanged,this, &RemoteInterface::connectionChanged);
    connect(m_connection, &Client::gotConnected, this, &RemoteInterface::requestInitialData);
    qRegisterMetaType<RemoteControl::CategoryModel*>("RemoteControl::CategoryModel*");
    qRegisterMetaType<RemoteControl::CategoryItemsModel*>("RemoteControl::CategoryItemsModel*");
}

void RemoteInterface::setCurrentPage(const QString& page)
{
    if (m_currentPage != page) {
        m_currentPage = page;
        emit currentPageChanged();
    }
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

void RemoteInterface::sendCommand(const RemoteCommand& command)
{
    m_connection->sendCommand(command);
}

void RemoteInterface::goHome()
{
    m_search.clear();
    requestInitialData();
}

void RemoteInterface::goBack()
{
    // PENDING(blackie) Need to implement a stack of actions
    goHome();
}

void RemoteInterface::selectCategory(const QString& category)
{
    m_search.addCategory(category);
    m_connection->sendCommand(RequestCategoryInfo(RequestCategoryInfo::RequestCategoryValues, m_search));
    m_categoryItems->setItems({});
    setCurrentPage(QStringLiteral("CategoryItems"));
}

void RemoteInterface::selectCategoryValue(const QString& value)
{
    m_search.addValue(value);
    m_connection->sendCommand(RequestCategoryInfo(RequestCategoryInfo::RequestCategoryNames, m_search));
    setCurrentPage(QStringLiteral("Overview"));
}

void RemoteInterface::showThumbnails()
{
    m_connection->sendCommand(RequestCategoryInfo(RequestCategoryInfo::ImageSearch, m_search));
    m_thumbnails = {};
    setCurrentPage(QString::fromUtf8("Thumbnails"));
}

void RemoteInterface::showImage(const QString& fileName)
{
    setCurrentPage(QStringLiteral("ImageViewer"));
    emit jumpToImage(m_thumbnails.indexOf(fileName));
}

void RemoteInterface::requestInitialData()
{
    m_connection->sendCommand(RequestCategoryInfo(RequestCategoryInfo::RequestCategoryNames, SearchInfo()));
    setCurrentPage(QStringLiteral("Overview"));
}

void RemoteInterface::handleCommand(const RemoteCommand& command)
{
    if (command.id() == ImageUpdateCommand::id())
        updateImage(static_cast<const ImageUpdateCommand&>(command));
    else if (command.id() == ImageCountUpdateCommand::id())
        updateImageCount(static_cast<const ImageCountUpdateCommand&>(command));
    else if (command.id() == CategoryListCommand::id())
        updateCategoryList(static_cast<const CategoryListCommand&>(command));
    else if (command.id() == CategoryItemListCommand::id())
        gotCategoryItems(static_cast<const CategoryItemListCommand&>(command));
    else if (command.id() == ImageSearchResult::id())
        gotImageSearchResult(static_cast<const ImageSearchResult&>(command));
    else
        qFatal("Unhandled command");
}

void RemoteInterface::updateImage(const ImageUpdateCommand& command)
{
    ImageStore::instance().updateImage(command.fileName, command.image);
}

void RemoteInterface::updateImageCount(const ImageCountUpdateCommand& command)
{
    Q_ASSERT(false && "Unused code");
    //m_imageMap.clear();
    m_imageCount = command.count;
    emit imageCountChanged();

}

void RemoteInterface::updateCategoryList(const CategoryListCommand& command)
{
    ScreenInfo::instance().setCategoryCount(command.categories.count());

    m_categories->setCategories(command.categories);
    int size = ScreenInfo::instance().overviewIconSize();
    m_homeImage = command.home.scaled(size,size);
    emit homeImageChanged();

    m_kphotoalbumImage = command.kphotoalbum.scaled(size,size);
    emit kphotoalbumImageChange();
}

void RemoteInterface::gotCategoryItems(const CategoryItemListCommand& result)
{
    m_categoryItems->setItems(result.items);
}

void RemoteInterface::gotImageSearchResult(const ImageSearchResult& result)
{
    if (m_thumbnails != result.relativeFileNames) {
        m_thumbnails = result.relativeFileNames;
        emit thumbnailsChanged();
    }
}
