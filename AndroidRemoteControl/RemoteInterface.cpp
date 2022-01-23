// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "RemoteInterface.h"
#include "ImageProvider.h"

#include "../RemoteControl/RemoteCommand.h"
#include "Action.h"
#include "Client.h"
#include "ImageDetails.h"
#include "ImageStore.h"
#include "ScreenInfo.h"
#include "Settings.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDataStream>
#include <QHostInfo>
#include <QLabel>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <qimage.h>

#include <memory>

using namespace RemoteControl;

RemoteInterface::RemoteInterface()
    : m_categories(new CategoryModel(this))
    , m_categoryItems(new ThumbnailModel(this))
    , m_thumbnailModel(new ThumbnailModel(this))
    , m_discoveryModel(new DiscoveryModel(this))
{
    m_connection = new Client;
    connect(m_connection, SIGNAL(gotCommand(RemoteCommand)), this, SLOT(handleCommand(RemoteCommand)));
    connect(m_connection, &Client::gotConnected, this, &RemoteInterface::connectionChanged);
    connect(m_connection, &Client::gotConnected, this, &RemoteInterface::requestInitialData);
    connect(m_connection, &Client::disconnected, this, &RemoteInterface::gotDisconnected);
    connect(m_connection, &Client::disconnected, this, &RemoteInterface::connectionChanged);
    qRegisterMetaType<RemoteControl::CategoryModel *>("RemoteControl::CategoryModel*");
    qRegisterMetaType<RemoteControl::ThumbnailModel *>("ThumbnailModel*");
    qRegisterMetaType<RemoteControl::DiscoveryModel *>("DiscoveryModel*");

    QTimer::singleShot(1000, this, SLOT(pushAwayFromStartupState()));
}

void RemoteInterface::setCurrentPage(Page page)
{
    if (m_currentPage != page) {
        m_currentPage = page;
        Q_EMIT currentPageChanged();
    }
}

void RemoteInterface::setListCategoryValues(const QStringList &values)
{
    if (m_listCategoryValues != values) {
        m_listCategoryValues = values;
        Q_EMIT listCategoryValuesChanged();
    }
}

void RemoteInterface::requestHomePageImages()
{
    m_connection->sendCommand(StaticImageRequest(ScreenInfo::instance().overviewIconSize()));
}

void RemoteInterface::gotDisconnected()
{
    setCurrentPage(Page::UnconnectedPage);
}

void RemoteInterface::setHomePageImages(const StaticImageResult &command)
{
    m_homeImage = command.homeIcon;
    Q_EMIT homeImageChanged();

    m_kphotoalbumImage = command.kphotoalbumIcon;
    Q_EMIT kphotoalbumImageChange();

    m_discoveryImage = command.discoverIcon;
    Q_EMIT discoveryImageChanged();

    ImageProvider::instance().m_info = command.info;
    ImageProvider::instance().m_slideShow = command.slideShow;
    ImageProvider::instance().m_search = command.discoverIcon;
}

RemoteInterface &RemoteInterface::instance()
{
    static RemoteInterface interface;
    return interface;
}

bool RemoteInterface::isConnected() const
{
    return m_connection->isConnected();
}

void RemoteInterface::sendCommand(const RemoteCommand &command)
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
    ThumbnailModel *newModel = (type == ModelType::Thumbnail ? m_thumbnailModel : m_discoveryModel);
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
    if (m_history.canGoBack())
        m_history.goBackward();
    else
        qApp->quit();
}

void RemoteInterface::goForward()
{
    if (m_history.canGoForward())
        m_history.goForward();
}

void RemoteInterface::selectCategory(const QString &category, int type)
{
    m_search.addCategory(category);
    m_history.push(std::unique_ptr<Action>(new ShowCategoryValueAction(m_search, static_cast<CategoryViewType>(type))));
}

void RemoteInterface::selectCategoryValue(const QString &value)
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
    m_connection->sendCommand(ImageDetailsRequest(imageId));
}

void RemoteInterface::activateSearch(const QString &search)
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
    sendCommand(ToggleTokenRequest(imageId, token, ToggleTokenRequest::On));
}

void RemoteInterface::removeToken(int imageId, const QString &token)
{
    sendCommand(ToggleTokenRequest(imageId, token, ToggleTokenRequest::Off));
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
    Q_EMIT jumpToImage(m_activeThumbnailModel->indexOf(imageId));
}

QString RemoteInterface::networkAddress() const
{
    QStringList result;
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.isLoopback() || address.toIPv4Address() == 0)
            continue;
        result.append(address.toString());
    }
    return result.join(QStringLiteral(", "));
}

QStringList RemoteInterface::tokens() const
{
    // FIXME: in KPA the tokens category is now retrieved using categoryForSpecial
    return ImageDetails::instance().itemsOfCategory(QStringLiteral("Tokens"));
}

void RemoteInterface::requestInitialData()
{
    requestHomePageImages();
    m_history.push(std::unique_ptr<Action>(new ShowOverviewAction({})));
}

void RemoteInterface::handleCommand(const RemoteCommand &command)
{
    if (command.commandType() == CommandType::ThumbnailResult)
        updateImage(static_cast<const ThumbnailResult &>(command));
    else if (command.commandType() == CommandType::CategoryListResult)
        updateCategoryList(static_cast<const CategoryListResult &>(command));
    else if (command.commandType() == CommandType::SearchResult)
        gotSearchResult(static_cast<const SearchResult &>(command));
    else if (command.commandType() == CommandType::TimeCommand)
        ; // Used for debugging, it will print time stamp when decoded
    else if (command.commandType() == CommandType::ImageDetailsResult) {
        ImageDetails::instance().setData(static_cast<const ImageDetailsResult &>(command));
        Q_EMIT tokensChanged();
    } else if (command.commandType() == CommandType::CategoryItemsResult)
        setListCategoryValues(static_cast<const CategoryItemsResult &>(command).items);
    else if (command.commandType() == CommandType::StaticImageResult)
        setHomePageImages(static_cast<const StaticImageResult &>(command));
    else
        qFatal("Unhandled command");
}

void RemoteInterface::updateImage(const ThumbnailResult &command)
{
    ImageStore::instance().updateImage(command.imageId, command.image, command.label, command.type);
}

void RemoteInterface::updateCategoryList(const CategoryListResult &command)
{
    ScreenInfo::instance().setCategoryCount(command.categories.count());
    m_categories->setCategories(command.categories);
}

void RemoteInterface::gotSearchResult(const SearchResult &result)
{
    if (result.type == SearchType::Images) {
        m_activeThumbnailModel->setImages(result.result);
    } else if (result.type == SearchType::CategoryItems) {
        m_categoryItems->setImages(result.result);
    }
}

#include "moc_RemoteInterface.cpp"
