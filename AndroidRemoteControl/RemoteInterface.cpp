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
#include <QHostInfo>
#include <memory>
#include "ImageDetails.h"
#include <QNetworkInterface>

using namespace RemoteControl;

RemoteInterface::RemoteInterface()
    : m_categories(new CategoryModel(this)), m_categoryItems(new ThumbnailModel(this)), m_thumbnailModel(new ThumbnailModel(this)),
      m_discoveryModel(new DiscoveryModel(this))
{
    m_connection = new Client;
    connect(m_connection, SIGNAL(gotCommand(RemoteCommand)), this, SLOT(handleCommand(RemoteCommand)));
    connect(m_connection, &Client::gotConnected,this, &RemoteInterface::connectionChanged);
    connect(m_connection, &Client::gotConnected, this, &RemoteInterface::requestInitialData);
    connect(m_connection, &Client::disconnected, this, &RemoteInterface::gotDisconnected);
    connect(m_connection, &Client::disconnected, this, &RemoteInterface::connectionChanged);
    qRegisterMetaType<RemoteControl::CategoryModel*>("RemoteControl::CategoryModel*");
    qRegisterMetaType<RemoteControl::ThumbnailModel*>("ThumbnailModel*");
    qRegisterMetaType<RemoteControl::DiscoveryModel*>("DiscoveryModel*");

    QTimer::singleShot(1000, this, SLOT(pushAwayFromStartupState()));
}

void RemoteInterface::setCurrentPage(Page page)
{
    if (m_currentPage != page) {
        m_currentPage = page;
        emit currentPageChanged();
    }
}

void RemoteInterface::setListCategoryValues(const QStringList& values)
{
    if (m_listCategoryValues != values) {
        m_listCategoryValues = values;
        emit listCategoryValuesChanged();
    }
}

void RemoteInterface::requestHomePageImages()
{
    m_connection->sendCommand(RequestHomePageImages(ScreenInfo::instance().overviewIconSize()));
}

void RemoteInterface::gotDisconnected()
{
    setCurrentPage(Page::UnconnectedPage);
}

void RemoteInterface::setHomePageImages(const HomePageData& command)
{
    m_homeImage = command.homeIcon;
    emit homeImageChanged();

    m_kphotoalbumImage = command.kphotoalbumIcon;
    emit kphotoalbumImageChange();

    m_discoveryImage = command.discoverIcon;
    emit discoveryImageChanged();
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

QImage RemoteInterface::discoveryImage() const
{
    return m_discoveryImage;
}

void RemoteInterface::setActiveThumbnailModel(RemoteInterface::ModelType type)
{
    ThumbnailModel* newModel = (type == ModelType::Thumbnail ? m_thumbnailModel : m_discoveryModel);
    if (newModel != m_activeThumbnailModel) {
        m_activeThumbnailModel = newModel;
        activeThumbnailModelChanged();
    }
    m_activeThumbnailModel->setImages({});
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

void RemoteInterface::selectCategory(const QString& category, int type)
{
    m_search.addCategory(category);
    m_history.push(std::unique_ptr<Action>(new ShowCategoryValueAction(m_search, static_cast<CategoryViewType>(type))));
}

void RemoteInterface::selectCategoryValue(const QString& value)
{
    m_search.addValue(value);
    m_history.push(std::unique_ptr<Action>(new ShowThumbnailsAction(m_search)));
}

void RemoteInterface::showThumbnails()
{
    m_history.push(std::unique_ptr<Action>(new ShowThumbnailsAction(m_search)));
}

void RemoteInterface::showImage(int imageId)
{
    m_history.push(std::unique_ptr<Action>(new ShowImagesAction(imageId, m_search)));
}

void RemoteInterface::requestDetails(int imageId)
{
    m_connection->sendCommand(RequestDetails(imageId));
}

void RemoteInterface::activateSearch(const QString& search)
{
    QStringList list = search.split(";;;");
    QString category = list[0];
    QString item = list[1];
    SearchInfo result;
    result.addCategory(category);
    result.addValue(item);
    m_history.push(std::unique_ptr<Action>(new ShowThumbnailsAction(result)));
}

void RemoteInterface::doDiscovery()
{
    m_history.push(std::unique_ptr<Action>(new DiscoverAction(m_search, m_discoveryModel)));
}

void RemoteInterface::showOverviewPage()
{
    m_history.push(std::unique_ptr<Action>(new ShowOverviewAction(m_search)));
}

void RemoteInterface::setToken(int imageId, const QString &token)
{
    sendCommand(ToggleTokenCommand(imageId, token, ToggleTokenCommand::On));
}

void RemoteInterface::removeToken(int imageId, const QString &token)
{
    sendCommand(ToggleTokenCommand(imageId, token, ToggleTokenCommand::Off));
}

void RemoteInterface::rerequestOverviewPageData()
{
    requestHomePageImages();
    m_history.rerunTopItem();
}

void RemoteInterface::pushAwayFromStartupState()
{
    // Avoid that the "not connected page" show for a few milliseconds while the connection is being set up.
    if (!isConnected() && m_currentPage == Types::Startup)
        setCurrentPage(Types::UnconnectedPage);
}

void RemoteInterface::setCurrentView(int imageId)
{
    emit jumpToImage(m_activeThumbnailModel->indexOf(imageId));
}

QString RemoteInterface::networkAddress() const
{
    QStringList result;
    for (const QHostAddress& address : QNetworkInterface::allAddresses()) {
        if (address.isLoopback() || address.toIPv4Address() == 0)
            continue;
        result.append(address.toString());
    }
    return result.join(QStringLiteral(", "));
}

QStringList RemoteInterface::tokens() const
{
    return ImageDetails::instance().itemsOfCategory(QStringLiteral("Tokens"));
}

void RemoteInterface::requestInitialData()
{
    requestHomePageImages();
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
    else if (command.id() == TimeCommand::id())
        ; // Used for debugging, it will print time stamp when decoded
    else if (command.id() == ImageDetailsCommand::id()) {
        ImageDetails::instance().setData(static_cast<const ImageDetailsCommand&>(command));
        emit tokensChanged();
    }
    else if (command.id() == CategoryItems::id())
        setListCategoryValues(static_cast<const CategoryItems&>(command).items);
    else if (command.id() == HomePageData::id())
        setHomePageImages(static_cast<const HomePageData&>(command));
    else
        qFatal("Unhandled command");
}

void RemoteInterface::updateImage(const ImageUpdateCommand& command)
{
    ImageStore::instance().updateImage(command.imageId, command.image, command.label, command.type);
}

void RemoteInterface::updateCategoryList(const CategoryListCommand& command)
{
    ScreenInfo::instance().setCategoryCount(command.categories.count());
    m_categories->setCategories(command.categories);
}

void RemoteInterface::gotSearchResult(const SearchResultCommand& result)
{
    if (result.type == SearchType::Images) {
        m_activeThumbnailModel->setImages(result.result);
    }
    else if (result.type == SearchType::CategoryItems) {
        m_categoryItems->setImages(result.result);
    }
}
