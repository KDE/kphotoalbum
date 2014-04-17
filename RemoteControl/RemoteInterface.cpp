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

#include <tuple>
#include <algorithm>

#include "ImageManager/AsyncLoader.h"

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

void RemoteInterface::pixmapLoaded(const DB::FileName& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage& image, const bool loadedOK)
{
    Q_UNUSED(size);
    Q_UNUSED(fullSize);
    Q_UNUSED(angle);
    Q_UNUSED(loadedOK);
    m_connection->sendCommand(ImageUpdateCommand(m_imageNameStore[fileName], QString(), image, ViewType::Images)); // FIXME, could be ViewType::Thumbails too!
}

bool RemoteInterface::requestStillNeeded(const DB::FileName& fileName)
{
    return m_activeReuqest.contains(fileName);
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
}


void RemoteInterface::sendCategoryNames(const SearchCommand& search)
{
    const int THUMBNAILSIZE = 70;
    const DB::ImageSearchInfo dbSearchInfo = convert(search.searchInfo);

    CategoryListCommand command;
    for (const DB::CategoryPtr& category : DB::ImageDB::instance()->categoryCollection()->categories()) {
        QMap<QString, uint> images = DB::ImageDB::instance()->classify( dbSearchInfo, category->name(), DB::Image );
        QMap<QString, uint> videos = DB::ImageDB::instance()->classify( dbSearchInfo, category->name(), DB::Video );
        const bool enabled = (images.count() + videos.count() > 1);

        const QImage icon = category->icon(THUMBNAILSIZE, enabled ? KIconLoader::DefaultState : KIconLoader::DisabledState).toImage();
        command.categories.append({category->name(), category->text(), icon, enabled});
    }

    // PENDING(blackie) This ought to go into a separate request, no need to send this every time.
    QPixmap homeIcon = KIconLoader::global()->loadIcon( QString::fromUtf8("go-home"), KIconLoader::Desktop, THUMBNAILSIZE);
    command.home = homeIcon.toImage();

    QPixmap kphotoalbumIcon = KIconLoader::global()->loadIcon( QString::fromUtf8("kphotoalbum"), KIconLoader::Desktop, THUMBNAILSIZE);
    command.kphotoalbum = kphotoalbumIcon.toImage();

    m_connection->sendCommand(command);
}

void RemoteInterface::sendCategoryValues(const SearchCommand& search)
{
    const DB::ImageSearchInfo dbSearchInfo = convert(search.searchInfo);
    const QString category = search.searchInfo.currentCategory();

    Browser::FlatCategoryModel model(DB::ImageDB::instance()->categoryCollection()->categoryForName(search.searchInfo.currentCategory()),
                                     dbSearchInfo);

    QList<int> result;
    std::transform( model._items.begin(), model._items.end(), std::back_inserter(result),
                    [this,category] (const QString itemName) {
        return m_imageNameStore.idForCategory(category,itemName);
    });

    m_connection->sendCommand(SearchResultCommand(SearchType::CategoryItems, result));
}

void RemoteInterface::sendImageSearchResult(const SearchInfo& search)
{
    const DB::FileNameList files = DB::ImageDB::instance()->search(convert(search), true /* Require on disk */);
    QList<int> result;
    std::transform(files.begin(), files.end(), std::back_inserter(result),
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
        RemoteImageRequest* request
                = new RemoteImageRequest(fileName, command.size, angle, this);
        // PENDING(blackie) I need a way to store information about command.viewType!
        ImageManager::AsyncLoader::instance()->load(request);
    }
}

void RemoteInterface::cancelRequest(const CancelRequestCommand& command)
{
    m_activeReuqest.remove(m_imageNameStore[command.imageId]);
}
