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
#include <functional>
#include <memory>
#include "Serializer.h"

using namespace RemoteControl;

#define ENUMSTREAM(TYPE) \
QDataStream &operator<<(QDataStream &stream, TYPE type) \
{\
    stream << (qint32) type;\
    return stream;\
}\
\
QDataStream &operator>>(QDataStream &stream, TYPE& type)\
{\
    stream >> (qint32&) type;\
    return stream;\
}\

ENUMSTREAM(ViewType)
ENUMSTREAM(SearchType)
ENUMSTREAM(ToggleTokenRequest::State)

RemoteCommand::RemoteCommand(const QString& id)
    :m_id(id)
{
}

RemoteCommand::~RemoteCommand()
{
    qDeleteAll(m_serializers);
}

void RemoteCommand::encode(QDataStream& stream) const
{
    for (SerializerInterface* serializer : m_serializers)
        serializer->encode(stream);
}

void RemoteCommand::decode(QDataStream& stream)
{
    for (SerializerInterface* serializer : m_serializers)
        serializer->decode(stream);
}

QString RemoteCommand::id() const
{
    return m_id;
}

void RemoteCommand::addSerializer(SerializerInterface *serializer)
{
    m_serializers.append(serializer);
}

using CommandFacory = std::function<std::unique_ptr<RemoteCommand>()>;
#define ADDFACTORY(COMMAND)\
   factories.insert(COMMAND::id(), \
       []() { return std::unique_ptr<RemoteCommand>(new COMMAND);} )

std::unique_ptr<RemoteCommand> RemoteCommand::create(const QString& id)
{
    static QMap<QString, CommandFacory> factories;
    if (factories.isEmpty()) {
        ADDFACTORY(ThumbnailResult);
        ADDFACTORY(CategoryListResult);
        ADDFACTORY(SearchRequest);
        ADDFACTORY(SearchResult);
        ADDFACTORY(ThumbnailRequest);
        ADDFACTORY(ThumbnailCancelRequest);
        ADDFACTORY(TimeCommand);
        ADDFACTORY(ImageDetailsRequest);
        ADDFACTORY(ImageDetailsResult);
        ADDFACTORY(CategoryItemsResult);
        ADDFACTORY(StaticImageRequest);
        ADDFACTORY(StaticImageResult);
        ADDFACTORY(ToggleTokenRequest);
    }
    Q_ASSERT(factories.contains(id));
    return factories[id]();
}

ThumbnailResult::ThumbnailResult(ImageId _imageId, const QString& _label, const QImage& _image, ViewType _type)
    :RemoteCommand(id()), imageId(_imageId), label(_label), image(_image), type(_type)
{
    addSerializer(new Serializer<ImageId>(imageId));
    addSerializer(new Serializer<QString>(label));
    addSerializer(new Serializer<QImage>(image));
    addSerializer(new Serializer<ViewType>(type));
}

QString ThumbnailResult::id()
{
    return QString::fromUtf8("Image Update");
}

QDataStream& operator<<(QDataStream& stream, const Category& category)
{
    stream << category.name << category.text << category.enabled << (int) category.viewType;
    fastStreamImage(stream, category.icon, BackgroundType::Transparent);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Category& category)
{
    stream >> category.name >> category.text >> category.enabled >> (int&) category.viewType;
    category.icon.load(stream.device(), "JPEG");
    return stream;
}

CategoryListResult::CategoryListResult()
    : RemoteCommand(id())
{
    addSerializer(new Serializer<QList<Category>>(categories));
}

QString CategoryListResult::id()
{
    return QString::fromUtf8("Category List");
}


SearchRequest::SearchRequest(SearchType _type, const SearchInfo& _searchInfo, int _size)
    :RemoteCommand(id()), type(_type), searchInfo(_searchInfo), size(_size)
{
    addSerializer(new Serializer<SearchType>(type));
    addSerializer(new Serializer<SearchInfo>(searchInfo));
    addSerializer(new Serializer<int>(size));
}

QString SearchRequest::id()
{
    return QString::fromUtf8("SearchCommand");
}

SearchResult::SearchResult(SearchType _type, const QList<int>& _result)
    :RemoteCommand(id()), type(_type), result(_result)
{
    addSerializer(new Serializer<SearchType>(type));
    addSerializer(new Serializer<QList<int>>(result));
}

QString SearchResult::id()
{
    return QString::fromUtf8("Image Search Result");
}

ThumbnailRequest::ThumbnailRequest(ImageId _imageId, const QSize& _size, ViewType _type)
    :RemoteCommand(id()), imageId(_imageId), size(_size), type(_type)
{
    addSerializer(new Serializer<ImageId>(imageId));
    addSerializer(new Serializer<QSize>(size));
    addSerializer(new Serializer<ViewType>(type));
}

QString ThumbnailRequest::id()
{
    return QString::fromUtf8("ThumbnailRequest");
}

RemoteControl::ThumbnailCancelRequest::ThumbnailCancelRequest(ImageId _imageId, ViewType _type)
    :RemoteCommand(id()), imageId(_imageId), type(_type)
{
    addSerializer(new Serializer<ImageId>(imageId));
    addSerializer(new Serializer<ViewType>(type));
}

QString ThumbnailCancelRequest::id()
{
    return QString::fromUtf8("CancelRequest");
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


ImageDetailsRequest::ImageDetailsRequest(ImageId _imageId)
    :RemoteCommand(id()), imageId(_imageId)
{
    addSerializer(new Serializer<ImageId>(imageId));
}

QString ImageDetailsRequest::id()
{
    return QString::fromUtf8("RequestDetails");
}

ImageDetailsResult::ImageDetailsResult()
    :RemoteCommand(id())
{
    addSerializer(new Serializer<QString>(fileName));
    addSerializer(new Serializer<QString>(date));
    addSerializer(new Serializer<QString>(description));
    addSerializer(new Serializer<QMap<QString,CategoryItemDetailsList>>(categories));
}

QString ImageDetailsResult::id()
{
    return QString::fromUtf8("ImageDetailsCommand");
}

QDataStream& operator<<(QDataStream& stream, const CategoryItemDetails& item)
{
    stream << item.name << item.age;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, CategoryItemDetails& item)
{
    stream >> item.name >> item.age;
    return stream;
}

CategoryItemsResult::CategoryItemsResult(const QStringList& _items)
    :RemoteCommand(id()), items(_items)
{
    addSerializer(new Serializer<QStringList>(items));
}

QString CategoryItemsResult::id()
{
    return QString::fromUtf8("CategoryItems");
}


StaticImageRequest::StaticImageRequest(int _size)
    :RemoteCommand(id()), size(_size)
{
    addSerializer(new Serializer<int>(size));
}

QString StaticImageRequest::id()
{
    return QString::fromUtf8("RequestHomePageImage");
}

StaticImageResult::StaticImageResult(const QImage& _homeIcon, const QImage& _kphotoalbumIcon, const QImage& _discoverIcon)
    :RemoteCommand(id()), homeIcon(_homeIcon), kphotoalbumIcon(_kphotoalbumIcon), discoverIcon(_discoverIcon)
{
    addSerializer(new Serializer<QImage>(homeIcon, BackgroundType::Transparent));
    addSerializer(new Serializer<QImage>(kphotoalbumIcon, BackgroundType::Transparent));
    addSerializer(new Serializer<QImage>(discoverIcon, BackgroundType::Transparent));
}

QString StaticImageResult::id()
{
    return QString::fromUtf8("HomePageData");
}

ToggleTokenRequest::ToggleTokenRequest(ImageId _imageId, const QString& _token, State _state)
    :RemoteCommand(id()), imageId(_imageId), token(_token), state(_state)
{
    addSerializer(new Serializer<ImageId>(imageId));
    addSerializer(new Serializer<QString>(token));
    addSerializer(new Serializer<State>(state));
}

QString ToggleTokenRequest::id()
{
    return QString::fromUtf8("SetTokenCommand");
}
