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

#include "Serializer.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QMap>
#include <QPainter>
#include <functional>
#include <memory>

using namespace RemoteControl;

#define ENUMSTREAM(TYPE)                                     \
    QDataStream &operator<<(QDataStream &stream, TYPE type)  \
    {                                                        \
        stream << (qint32)type;                              \
        return stream;                                       \
    }                                                        \
                                                             \
    QDataStream &operator>>(QDataStream &stream, TYPE &type) \
    {                                                        \
        stream >> (qint32 &)type;                            \
        return stream;                                       \
    }

ENUMSTREAM(ViewType)
ENUMSTREAM(SearchType)
ENUMSTREAM(ToggleTokenRequest::State)

RemoteCommand::RemoteCommand(CommandType type)
    : m_type(type)
{
}

RemoteCommand::~RemoteCommand()
{
    qDeleteAll(m_serializers);
}

void RemoteCommand::encode(QDataStream &stream) const
{
    for (SerializerInterface *serializer : m_serializers)
        serializer->encode(stream);
}

void RemoteCommand::decode(QDataStream &stream)
{
    for (SerializerInterface *serializer : m_serializers)
        serializer->decode(stream);
}

CommandType RemoteCommand::commandType() const
{
    return m_type;
}

void RemoteCommand::addSerializer(SerializerInterface *serializer)
{
    m_serializers.append(serializer);
}

using CommandFacory = std::function<std::unique_ptr<RemoteCommand>()>;
#define ADDFACTORY(COMMAND)                \
    factories.insert(CommandType::COMMAND, \
                     []() { return std::unique_ptr<RemoteCommand>(new COMMAND); })

std::unique_ptr<RemoteCommand> RemoteCommand::create(CommandType id)
{
    static QMap<CommandType, CommandFacory> factories;
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

ThumbnailResult::ThumbnailResult(ImageId _imageId, const QString &_label, const QImage &_image, ViewType _type)
    : RemoteCommand(CommandType::ThumbnailResult)
    , imageId(_imageId)
    , label(_label)
    , image(_image)
    , type(_type)
{
    addSerializer(new Serializer<ImageId>(imageId));
    addSerializer(new Serializer<QString>(label));
    addSerializer(new Serializer<QImage>(image));
    addSerializer(new Serializer<ViewType>(type));
}

QDataStream &operator<<(QDataStream &stream, const Category &category)
{
    stream << category.name << category.enabled << (int)category.viewType;
    fastStreamImage(stream, category.icon, BackgroundType::Transparent);
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Category &category)
{
    int tmp;
    stream >> category.name >> category.enabled >> tmp;
    category.viewType = static_cast<RemoteControl::CategoryViewType>(tmp);
    category.icon.load(stream.device(), "JPEG");
    return stream;
}

CategoryListResult::CategoryListResult()
    : RemoteCommand(CommandType::CategoryListResult)
{
    addSerializer(new Serializer<QList<Category>>(categories));
}

SearchRequest::SearchRequest(SearchType _type, const SearchInfo &_searchInfo, int _size)
    : RemoteCommand(CommandType::SearchRequest)
    , type(_type)
    , searchInfo(_searchInfo)
    , size(_size)
{
    addSerializer(new Serializer<SearchType>(type));
    addSerializer(new Serializer<SearchInfo>(searchInfo));
    addSerializer(new Serializer<int>(size));
}

SearchResult::SearchResult(SearchType _type, const QList<int> &_result)
    : RemoteCommand(CommandType::SearchResult)
    , type(_type)
    , result(_result)
{
    addSerializer(new Serializer<SearchType>(type));
    addSerializer(new Serializer<QList<int>>(result));
}

ThumbnailRequest::ThumbnailRequest(ImageId _imageId, const QSize &_size, ViewType _type)
    : RemoteCommand(CommandType::ThumbnailRequest)
    , imageId(_imageId)
    , size(_size)
    , type(_type)
{
    addSerializer(new Serializer<ImageId>(imageId));
    addSerializer(new Serializer<QSize>(size));
    addSerializer(new Serializer<ViewType>(type));
}

RemoteControl::ThumbnailCancelRequest::ThumbnailCancelRequest(ImageId _imageId, ViewType _type)
    : RemoteCommand(CommandType::ThumbnailCancelRequest)
    , imageId(_imageId)
    , type(_type)
{
    addSerializer(new Serializer<ImageId>(imageId));
    addSerializer(new Serializer<ViewType>(type));
}

TimeCommand::TimeCommand()
    : RemoteCommand(CommandType::TimeCommand)
{
}

static void printElapsed()
{
    static QElapsedTimer timer;
    qDebug() << "Time since last dump: " << timer.elapsed();
    timer.restart();
}

void TimeCommand::encode(QDataStream &) const
{
    printElapsed();
}

void TimeCommand::decode(QDataStream &)
{
    printElapsed();
}

ImageDetailsRequest::ImageDetailsRequest(ImageId _imageId)
    : RemoteCommand(CommandType::ImageDetailsRequest)
    , imageId(_imageId)
{
    addSerializer(new Serializer<ImageId>(imageId));
}

ImageDetailsResult::ImageDetailsResult()
    : RemoteCommand(CommandType::ImageDetailsResult)
{
    addSerializer(new Serializer<QString>(fileName));
    addSerializer(new Serializer<QString>(date));
    addSerializer(new Serializer<QString>(description));
    addSerializer(new Serializer<QMap<QString, CategoryItemDetailsList>>(categories));
}

//// WHAT WHAT WHAT
QDataStream &operator<<(QDataStream &stream, const CategoryItemDetails &item)
{
    stream << item.name << item.age;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, CategoryItemDetails &item)
{
    stream >> item.name >> item.age;
    return stream;
}

CategoryItemsResult::CategoryItemsResult(const QStringList &_items)
    : RemoteCommand(CommandType::CategoryItemsResult)
    , items(_items)
{
    addSerializer(new Serializer<QStringList>(items));
}

StaticImageRequest::StaticImageRequest(int _size)
    : RemoteCommand(CommandType::StaticImageRequest)
    , size(_size)
{
    addSerializer(new Serializer<int>(size));
}

StaticImageResult::StaticImageResult(const QImage &_homeIcon, const QImage &_kphotoalbumIcon, const QImage &_discoverIcon)
    : RemoteCommand(CommandType::StaticImageResult)
    , homeIcon(_homeIcon)
    , kphotoalbumIcon(_kphotoalbumIcon)
    , discoverIcon(_discoverIcon)
{
    addSerializer(new Serializer<QImage>(homeIcon, BackgroundType::Transparent));
    addSerializer(new Serializer<QImage>(kphotoalbumIcon, BackgroundType::Transparent));
    addSerializer(new Serializer<QImage>(discoverIcon, BackgroundType::Transparent));
}

ToggleTokenRequest::ToggleTokenRequest(ImageId _imageId, const QString &_token, State _state)
    : RemoteCommand(CommandType::ToggleTokenRequest)
    , imageId(_imageId)
    , token(_token)
    , state(_state)
{
    addSerializer(new Serializer<ImageId>(imageId));
    addSerializer(new Serializer<QString>(token));
    addSerializer(new Serializer<State>(state));
}
