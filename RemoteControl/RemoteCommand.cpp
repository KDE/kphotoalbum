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

#include "RemoteCommand.h"

#include <QElapsedTimer>
#include <QMap>
#include <QDebug>
#include <QPainter>

using namespace RemoteControl;

RemoteCommand::RemoteCommand(const QString& id)
    :m_id(id)
{
}

QString RemoteCommand::id() const
{
    return m_id;
}

RemoteCommand& RemoteCommand::command(const QString& id)
{
    static QMap<QString, RemoteCommand*> map;
    if (map.isEmpty()) {
        QList<RemoteCommand*> commands;
        commands << new ImageUpdateCommand
                 << new CategoryListCommand
                 << new SearchCommand
                 << new SearchResultCommand
                 << new ThumbnailRequest
                 << new CancelRequestCommand
                 << new TimeCommand
                 << new RequestDetails
                 << new ImageDetailsCommand
                 << new CategoryItems
                 << new RequestHomePageImages
                 << new HomePageData
                 << new ToggleTokenCommand;
                // Remember to bounce the protocol version number

        for (RemoteCommand* command : commands )
             map.insert(command->id(), command);
    }

    Q_ASSERT(map.contains(id));
    return *map[id];
}

void RemoteCommand::encodeImage(QDataStream& stream, const QImage& image) const
{
    image.save(stream.device(),"JPEG");
}

void RemoteCommand::encodeImageWithTransparentPixels(QDataStream &stream, const QImage &image) const
{
    QImage result(image.width(), image.height(), QImage::Format_RGB32);
    result.fill(Qt::black);
    QPainter p(&result);
    p.drawImage(0,0, image);
    p.end();
    encodeImage(stream, result);
}

QImage RemoteCommand::decodeImage(QDataStream& stream) const
{
    QImage result;
    result.load(stream.device(), "JPEG");
    return result;
}

ImageUpdateCommand::ImageUpdateCommand(ImageId imageId, const QString& label, const QImage& image, ViewType type)
    :RemoteCommand(id()), imageId(imageId), label(label), image(image), type(type)
{
}

QString ImageUpdateCommand::id()
{
    return QString::fromUtf8("Image Update");
}

void ImageUpdateCommand::encode(QDataStream& stream) const
{
    stream << imageId << label;
    encodeImage(stream,image);
    stream << (int) type;
}

void ImageUpdateCommand::decode(QDataStream& stream)
{
    stream >> imageId >> label;
    image = decodeImage(stream);
    stream >> (int&) type;
}


CategoryListCommand::CategoryListCommand()
    : RemoteCommand(id())
{
}

QString CategoryListCommand::id()
{
    return QString::fromUtf8("Category List");
}

void CategoryListCommand::encode(QDataStream& stream) const
{
    stream << categories.count();
    for (const Category& category : categories) {
        stream << category.name << category.text << category.enabled << (int) category.viewType;
        encodeImageWithTransparentPixels(stream, category.icon);
    }
}

void CategoryListCommand::decode(QDataStream& stream)
{
    int count;
    stream >> count;
    categories.clear();
    for (int i=0; i<count; ++i) {
        QString name;
        QString text;
        QImage icon;
        bool enabled;
        CategoryViewType viewType;
        stream >> name >> text >> enabled >> (int&) viewType;
        icon = decodeImage(stream);
        categories.append({name, text, icon, enabled, viewType});
    }
}


SearchCommand::SearchCommand(SearchType type, const SearchInfo& searchInfo, int size)
    :RemoteCommand(id()), type(type), searchInfo(searchInfo), size(size)
{
}

QString SearchCommand::id()
{
    return QString::fromUtf8("SearchCommand");
}

void SearchCommand::encode(QDataStream& stream) const
{
    stream << (int) type << searchInfo << size;
}

void SearchCommand::decode(QDataStream& stream)
{
    stream >> (int&) type >> searchInfo >> size;
}


SearchResultCommand::SearchResultCommand(SearchType type, const QList<int>& result)
    :RemoteCommand(id()), type(type), result(result)
{
}

QString SearchResultCommand::id()
{
    return QString::fromUtf8("Image Search Result");
}

void SearchResultCommand::encode(QDataStream& stream) const
{
    stream << (int) type << result;
}

void SearchResultCommand::decode(QDataStream& stream)
{
    stream >> (int&) type >> result;
}


ThumbnailRequest::ThumbnailRequest(ImageId imageId, const QSize& size, ViewType type)
    :RemoteCommand(id()), imageId(imageId), size(size), type(type)
{
}

QString ThumbnailRequest::id()
{
    return QString::fromUtf8("ThumbnailRequest");
}

void ThumbnailRequest::encode(QDataStream& stream) const
{
    stream << imageId << size << (int) type;
}

void ThumbnailRequest::decode(QDataStream& stream)
{
    stream >> imageId >> size >> (int&) type;
}


RemoteControl::CancelRequestCommand::CancelRequestCommand(ImageId imageId, ViewType type)
    :RemoteCommand(id()), imageId(imageId), type(type)
{
}

QString CancelRequestCommand::id()
{
    return QString::fromUtf8("CancelRequest");
}

void CancelRequestCommand::encode(QDataStream& stream) const
{
    stream << imageId << (int) type;
}

void CancelRequestCommand::decode(QDataStream& stream)
{
    stream >> imageId >> (int&) type;
}


TimeCommand::TimeCommand()
    :RemoteCommand(id())
{
}

QString TimeCommand::id()
{
    return QString::fromUtf8("TimeDump");
}

static void printElapsed()
{
    static QElapsedTimer timer;
    qDebug() << "Time since last dump: " << timer.elapsed();
    timer.restart();
}

void TimeCommand::encode(QDataStream&) const
{
    printElapsed();
}

void TimeCommand::decode(QDataStream&)
{
    printElapsed();
}


RequestDetails::RequestDetails(ImageId imageId)
    :RemoteCommand(id()), imageId(imageId)
{
}

QString RequestDetails::id()
{
    return QString::fromUtf8("RequestDetails");
}

void RequestDetails::encode(QDataStream& stream) const
{
    stream << imageId;
}

void RequestDetails::decode(QDataStream& stream)
{
    stream >> imageId;
}


ImageDetailsCommand::ImageDetailsCommand()
    :RemoteCommand(id())
{
}

QString ImageDetailsCommand::id()
{
    return QString::fromUtf8("ImageDetailsCommand");
}

void ImageDetailsCommand::encode(QDataStream& stream) const
{
    stream << fileName << date << description << categories;
}

void ImageDetailsCommand::decode(QDataStream& stream)
{
    stream >> fileName >> date >> description >> categories;
}



CategoryItems::CategoryItems(const QStringList items)
    :RemoteCommand(id()), items(items)
{
}

QString CategoryItems::id()
{
    return QString::fromUtf8("CategoryItems");
}

void CategoryItems::encode(QDataStream& stream) const
{
    stream << items;
}

void CategoryItems::decode(QDataStream& stream)
{
    stream >> items;
}


RequestHomePageImages::RequestHomePageImages(int size)
    :RemoteCommand(id()), size(size)
{
}

QString RequestHomePageImages::id()
{
    return QString::fromUtf8("RequestHomePageImage");
}

void RequestHomePageImages::encode(QDataStream& stream) const
{
    stream << size;
}

void RequestHomePageImages::decode(QDataStream& stream)
{
    stream >> size;
}


HomePageData::HomePageData(const QImage& homeIcon, const QImage& kphotoalbumIcon, const QImage& discoverIcon)
    :RemoteCommand(id()), homeIcon(homeIcon), kphotoalbumIcon(kphotoalbumIcon), discoverIcon(discoverIcon)
{
}

QString HomePageData::id()
{
    return QString::fromUtf8("HomePageData");
}

void HomePageData::encode(QDataStream& stream) const
{
    encodeImageWithTransparentPixels(stream, homeIcon);
    encodeImageWithTransparentPixels(stream, kphotoalbumIcon);
    encodeImageWithTransparentPixels(stream, discoverIcon);
}

void HomePageData::decode(QDataStream& stream)
{
    homeIcon = decodeImage(stream);
    kphotoalbumIcon = decodeImage(stream);
    discoverIcon = decodeImage(stream);
}


ToggleTokenCommand::ToggleTokenCommand(ImageId imageId, const QString &token, State state)
    :RemoteCommand(id()), imageId(imageId), token(token), state(state)
{
}

QString ToggleTokenCommand::id()
{
    return QString::fromUtf8("SetTokenCommand");
}

void ToggleTokenCommand::encode(QDataStream &stream) const
{
    stream << imageId << token << (int) state;
}

void ToggleTokenCommand::decode(QDataStream &stream)
{
    stream >> imageId >> token >> (int&) state;
}
