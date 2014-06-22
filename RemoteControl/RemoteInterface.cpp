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
#include "DB/ImageSearchInfo.h"
#include "Browser/FlatCategoryModel.h"
#include "DB/ImageInfoPtr.h"
#include "RemoteImageRequest.h"
#include "DB/ImageInfoPtr.h"
#include "DB/ImageInfo.h"
#include "DB/Category.h"
#include "Types.h"

#include <tuple>
#include <algorithm>

#include "ImageManager/AsyncLoader.h"
#include "MainWindow/DirtyIndicator.h"

using namespace RemoteControl;

RemoteInterface& RemoteInterface::instance()
{
    static RemoteInterface instance;
    return instance;
}

RemoteInterface::RemoteInterface(QObject *parent) :
    QObject(parent), m_connection(new Server(this))
{
    connect(m_connection, SIGNAL(gotCommand(RemoteCommand)), this, SLOT(handleCommand(RemoteCommand)));
    connect(m_connection, SIGNAL(connected()), this, SIGNAL(connected()));
    connect(m_connection, SIGNAL(disConnected()), this, SIGNAL(disConnected()));
    connect(m_connection, SIGNAL(stoppedListening()), this, SIGNAL(stoppedListening()));
}

DB::ImageSearchInfo RemoteInterface::convert(const SearchInfo& searchInfo) const
{
    DB::ImageSearchInfo dbSearchInfo;
    QString category;
    QString value;
    for (auto item : searchInfo.values()) {
        std::tie(category, value) = item;
        dbSearchInfo.addAnd(category, value);
    }
    return dbSearchInfo;
}

void RemoteInterface::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image)
{
    m_connection->sendCommand(ImageUpdateCommand(m_imageNameStore[request->databaseFileName()], QString(),
                              image, static_cast<RemoteImageRequest*>(request)->type()));
}

bool RemoteInterface::requestStillNeeded(const DB::FileName& fileName)
{
    return m_activeReuqest.contains(fileName);
}

void RemoteInterface::listen()
{
    m_connection->listen();
    emit listening();
}

void RemoteInterface::stopListening()
{
    m_connection->stopListening();
}

void RemoteInterface::connectTo(const QHostAddress& address)
{
    m_connection->connectToTcpServer(address);
}

void RemoteInterface::handleCommand(const RemoteCommand& command)
{
    if (command.id() == SearchCommand::id()) {
        const SearchCommand& searchCommand = static_cast<const SearchCommand&>(command);
        if (searchCommand.type == SearchType::Categories)
            sendCategoryNames(searchCommand);
        else if (searchCommand.type == SearchType::CategoryItems)
            sendCategoryValues(searchCommand);
        else
            sendImageSearchResult(searchCommand.searchInfo);
    }
    else if (command.id() == ThumbnailRequest::id())
        requestThumbnail(static_cast<const ThumbnailRequest&>(command));
    else if (command.id() == CancelRequestCommand::id())
        cancelRequest(static_cast<const CancelRequestCommand&>(command));
    else if (command.id() == RequestDetails::id())
        sendImageDetails(static_cast<const RequestDetails&>(command));
    else if (command.id() == RequestHomePageImages::id())
        sendHomePageImages(static_cast<const RequestHomePageImages&>(command));
    else if (command.id() == ToggleTokenCommand::id())
        setToken(static_cast<const ToggleTokenCommand&>(command));
}


void RemoteInterface::sendCategoryNames(const SearchCommand& search)
{
    const DB::ImageSearchInfo dbSearchInfo = convert(search.searchInfo);

    CategoryListCommand command;
    for (const DB::CategoryPtr& category : DB::ImageDB::instance()->categoryCollection()->categories()) {
        if (category->name() == QString::fromLatin1("Media Type"))
            continue;
        QMap<QString, uint> images = DB::ImageDB::instance()->classify( dbSearchInfo, category->name(), DB::Image );

        QMap<QString, uint> videos = DB::ImageDB::instance()->classify( dbSearchInfo, category->name(), DB::Video );
        const bool enabled = (images.count() /*+ videos.count()*/ > 1);
        CategoryViewType type =
                (category->viewType() == DB::Category::IconView || category->viewType() == DB::Category::ThumbedIconView)
                ? Types::CategoryIconView : Types::CategoryListView;

        const QImage icon = category->icon(search.size, enabled ? KIconLoader::DefaultState : KIconLoader::DisabledState).toImage();
        command.categories.append({category->name(), category->text(), icon, enabled, type});
    }
    m_connection->sendCommand(command);
}

void RemoteInterface::sendCategoryValues(const SearchCommand& search)
{
    const DB::ImageSearchInfo dbSearchInfo = convert(search.searchInfo);
    const QString categoryName = search.searchInfo.currentCategory();

    const DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(search.searchInfo.currentCategory());

    Browser::FlatCategoryModel model(category, dbSearchInfo);

    if (category->viewType() == DB::Category::IconView || category->viewType() == DB::Category::ThumbedIconView) {
        QList<int> result;
        std::transform( model._items.begin(), model._items.end(), std::back_inserter(result),
                        [this,categoryName] (const QString itemName) {
            return m_imageNameStore.idForCategory(categoryName,itemName);
        });
        m_connection->sendCommand(SearchResultCommand(SearchType::CategoryItems, result));
    }
    else {
        m_connection->sendCommand(CategoryItems(model._items));
    }
}

void RemoteInterface::sendImageSearchResult(const SearchInfo& search)
{
    const DB::FileNameList files = DB::ImageDB::instance()->search(convert(search), true /* Require on disk */);
    DB::FileNameList stacksRemoved;
    QList<int> result;

    std::remove_copy_if(files.begin(), files.end(), std::back_inserter(stacksRemoved),
                        [] (const DB::FileName& file) {
        // Only include unstacked images, and the top of stacked images.
        // And also exclude videos
        return DB::ImageDB::instance()->info(file)->stackOrder() > 1 ||
                DB::ImageDB::instance()->info(file)->isVideo();
    });

    std::transform(stacksRemoved.begin(), stacksRemoved.end(), std::back_inserter(result),
                   [this](const DB::FileName& fileName) {
        return m_imageNameStore[fileName];
    });

    m_connection->sendCommand(SearchResultCommand(SearchType::Images, result));
}

void RemoteInterface::requestThumbnail(const ThumbnailRequest& command)
{
    if (command.type == ViewType::CategoryItems) {
        auto tuple = m_imageNameStore.categoryForId(command.imageId);
        QString categoryName = tuple.first;
        QString itemName = tuple.second;

        const DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(categoryName);
        QImage image = category->categoryImage( categoryName, itemName, command.size.width(), command.size.height()).toImage();
        m_connection->sendCommand(ImageUpdateCommand(command.imageId, itemName, image,ViewType::CategoryItems));
    }
    else {
        const DB::FileName fileName = m_imageNameStore[command.imageId];
        const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
        const int angle = info->angle();

        m_activeReuqest.insert(fileName);

        QSize size = command.size;
        if (!size.isValid()) {
            // Request for full screen image.
            size = info->size();
        }
        RemoteImageRequest* request
                = new RemoteImageRequest(fileName, size, angle, command.type, this);

        ImageManager::AsyncLoader::instance()->load(request);
    }
}

void RemoteInterface::cancelRequest(const CancelRequestCommand& command)
{
    m_activeReuqest.remove(m_imageNameStore[command.imageId]);
}

void RemoteInterface::sendImageDetails(const RequestDetails& command)
{
    const DB::FileName fileName = m_imageNameStore[command.imageId];
    const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
    ImageDetailsCommand result;
    result.fileName = fileName.relative();
    result.date = info->date().toString();
    result.description = info->description();
    result.categories.clear();
    for (const QString& category : info->availableCategories()) {
        result.categories[category] = info->itemsOfCategory(category).toList();
    }

    m_connection->sendCommand(result);
}

void RemoteInterface::sendHomePageImages(const RequestHomePageImages& command)
{
    const int size = command.size;

    QPixmap homeIcon = KIconLoader::global()->loadIcon( QString::fromUtf8("go-home"), KIconLoader::Desktop, size);
    QPixmap kphotoalbumIcon = KIconLoader::global()->loadIcon( QString::fromUtf8("kphotoalbum"), KIconLoader::Desktop, size);
    QPixmap discoverIcon = KIconLoader::global()->loadIcon( QString::fromUtf8("edit-find"), KIconLoader::Desktop, size);

    m_connection->sendCommand(HomePageData(homeIcon.toImage(), kphotoalbumIcon.toImage(), discoverIcon.toImage()));
}

void RemoteInterface::setToken(const ToggleTokenCommand& command)
{
    const DB::FileName fileName = m_imageNameStore[command.imageId];
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
    if (command.state == ToggleTokenCommand::On)
        info->addCategoryInfo(QString::fromUtf8("Tokens"), command.token);
    else
        info->removeCategoryInfo(QString::fromUtf8("Tokens"), command.token);
    MainWindow::DirtyIndicator::markDirty();
}
