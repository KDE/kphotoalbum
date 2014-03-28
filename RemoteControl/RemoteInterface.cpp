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

#include <tuple>
#include <algorithm>

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
        if (requestCommand.type == RequestCategoryInfo::RequestCategoryNames)
            sendCategoryNames(requestCommand);
        else if (requestCommand.type == RequestCategoryInfo::RequestCategoryValues)
            sendCategoryValues(requestCommand);
        else
            sendImageSearchResult(requestCommand.searchInfo);
    }
}


void RemoteInterface::sendCategoryNames(const RequestCategoryInfo& search)
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

void RemoteInterface::sendCategoryValues(const RequestCategoryInfo& search)
{
    const DB::ImageSearchInfo dbSearchInfo = convert(search.searchInfo);
    Browser::FlatCategoryModel model(DB::ImageDB::instance()->categoryCollection()->categoryForName(search.searchInfo.currentCategory()),
                                     dbSearchInfo);

    CategoryItemListCommand result;
    for (int i=0; i<model.rowCount(QModelIndex());++i)
        result.addItem(model.data(model.index(i,0), Qt::DisplayRole).value<QString>(),
                       model.data(model.index(i,0), Qt::DecorationRole).value<QImage>());
    m_connection->sendCommand(result);
}

void RemoteInterface::sendImageSearchResult(const SearchInfo& search)
{
    const DB::FileNameList files = DB::ImageDB::instance()->search(convert(search), true /* Require on disk */);
    QStringList relativeFileNames;
    std::transform(files.begin(), files.end(), std::back_inserter(relativeFileNames),
                   [](const DB::FileName& fileName) { return fileName.relative(); });
    m_connection->sendCommand(ImageSearchResult(relativeFileNames));
}

