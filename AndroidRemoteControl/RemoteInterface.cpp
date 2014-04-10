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
#include "Action.h"
#include <QCoreApplication>
#include <memory>
using namespace RemoteControl;

RemoteInterface::RemoteInterface()
    : m_categories(new CategoryModel(this))
{
    m_connection = new Client;
    connect(m_connection, SIGNAL(gotCommand(RemoteCommand)), this, SLOT(handleCommand(RemoteCommand)));
    connect(m_connection, &Client::connectionChanged,this, &RemoteInterface::connectionChanged);
    connect(m_connection, &Client::gotConnected, this, &RemoteInterface::requestInitialData);
    qRegisterMetaType<RemoteControl::CategoryModel*>("RemoteControl::CategoryModel*");
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

QString RemoteInterface::currentCategory() const
{
    return m_search.currentCategory();
}

void RemoteInterface::goHome()
{
    requestInitialData();
}

void RemoteInterface::goBack()
{
    if(m_history.canGoBack())
        m_history.goBackward();
    else
        qApp->quit();
}

void RemoteInterface::goForward()
{
    if (m_history.canGoForward())
        m_history.goForward();
}

void RemoteInterface::selectCategory(const QString& category)
{
    m_search.addCategory(category);
    m_history.push(std::unique_ptr<Action>(new ShowCategoryValueAction(m_search)));
}

void RemoteInterface::selectCategoryValue(const QString& value)
{
    m_search.addValue(value);
    m_history.push(std::unique_ptr<Action>(new ShowOverviewAction(m_search)));
}

void RemoteInterface::showThumbnails()
{
    m_history.push(std::unique_ptr<Action>(new ShowThumbnailsAction(m_search)));
}

void RemoteInterface::showImage(const QString& fileName)
{
    m_history.push(std::unique_ptr<Action>(new ShowImagesAction(fileName, m_search)));
}

void RemoteInterface::setCurrentView(const QString& image)
{
    emit jumpToImage(m_thumbnails.indexOf(image));
}

void RemoteInterface::requestInitialData()
{
    m_history.push(std::unique_ptr<Action>(new ShowOverviewAction({})));
}

void RemoteInterface::handleCommand(const RemoteCommand& command)
{
    if (command.id() == ImageUpdateCommand::id())
        updateImage(static_cast<const ImageUpdateCommand&>(command));
    else if (command.id() == CategoryListCommand::id())
        updateCategoryList(static_cast<const CategoryListCommand&>(command));
    else if (command.id() == SearchResultCommand::id())
        gotSearchResult(static_cast<const SearchResultCommand&>(command));
    else
        qFatal("Unhandled command");
}

void RemoteInterface::updateImage(const ImageUpdateCommand& command)
{
    ImageStore::instance().updateImage(command.fileName, command.image, command.type);
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

void RemoteInterface::gotSearchResult(const SearchResultCommand& result)
{
    if (result.type == SearchType::Images) {
        if (m_thumbnails != result.values) {
            m_thumbnails = result.values;
            emit thumbnailsChanged();
        }
    }
    else if (result.type == SearchType::CategoryItems) {
        if (m_categoryItems != result.values) {
            m_categoryItems = result.values;
            emit categoryItemsChanged();
        }
    }
}
