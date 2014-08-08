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

#ifndef REMOTECOMMAND_H
#define REMOTECOMMAND_H

#include "SearchInfo.h"

#include <QString>
#include <QDataStream>
#include <QImage>
#include <QBuffer>
#include <QStringList>
#include <QPair>
#include "Types.h"
#include <QMap>
#include <memory>
#include <QPainter>

namespace RemoteControl
{
class SerializerInterface;

const int VERSION = 7;

enum class CommandType {
    ThumbnailResult,
    CategoryListResult,
    SearchRequest,
    SearchResult,
    ThumbnailRequest,
    ThumbnailCancelRequest,
    TimeCommand,
    ImageDetailsRequest,
    ImageDetailsResult,
    CategoryItemsResult,
    StaticImageRequest,
    StaticImageResult,
    ToggleTokenRequest
};


class RemoteCommand
{
public:
    RemoteCommand(CommandType type);
    virtual ~RemoteCommand();
    virtual void encode(QDataStream&) const;
    virtual void decode(QDataStream&);
    CommandType commandType() const;

    void addSerializer(SerializerInterface* serializer);
    static std::unique_ptr<RemoteCommand> create(CommandType commandType);

private:
    QList<SerializerInterface*> m_serializers;
    CommandType m_type;
};

class ThumbnailResult :public RemoteCommand
{
public:
    ThumbnailResult(ImageId imageId = {}, const QString& label = {}, const QImage& image = QImage(), ViewType type = {});
    ImageId imageId;
    QString label;
    QImage image;
    ViewType type;
};

struct Category {
    QString name;
    QString text;
    QImage icon;
    bool enabled;
    CategoryViewType viewType;
};

class CategoryListResult :public RemoteCommand
{
public:
    CategoryListResult();
    QList<Category> categories;
};

class SearchRequest :public RemoteCommand
{
public:
    SearchRequest(SearchType type = {}, const SearchInfo& searchInfo = {}, int size = {});
    SearchType type;
    SearchInfo searchInfo;
    int size; // Only used for SearchType::Categories
};

class SearchResult :public RemoteCommand
{
public:
    SearchResult(SearchType type = {}, const QList<int>& result = {});
    SearchType type;
    QList<int> result;
};

class ThumbnailRequest :public RemoteCommand
{
public:
    ThumbnailRequest(ImageId imageId = {}, const QSize& size = {}, ViewType type = {});
    ImageId imageId;
    QSize size;
    ViewType type;
};

class ThumbnailCancelRequest :public RemoteCommand
{
public:
    ThumbnailCancelRequest(ImageId imageId = {}, ViewType type = {});
    ImageId imageId;
    ViewType type;
};

class TimeCommand :public RemoteCommand
{
public:
    TimeCommand();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
};

class ImageDetailsRequest :public RemoteCommand
{
public:
    ImageDetailsRequest(ImageId imageId = {});
    ImageId imageId;
};

struct CategoryItemDetails {
    CategoryItemDetails(const QString& name = {}, const QString& age = {})
        : name(name), age(age) {}
    QString name;
    QString age;
};

using CategoryItemDetailsList = QList<CategoryItemDetails>;

class ImageDetailsResult :public RemoteCommand
{
public:
    ImageDetailsResult();
    QString fileName;
    QString date;
    QString description;
    QMap<QString,CategoryItemDetailsList> categories;
};

class CategoryItemsResult :public RemoteCommand
{
public:
    CategoryItemsResult(const QStringList& items = {});
    QStringList items;
};

class StaticImageRequest :public RemoteCommand
{
public:
    StaticImageRequest(int size = {});
    int size;
};

class StaticImageResult :public RemoteCommand
{
public:
    StaticImageResult(const QImage& homeIcon = {}, const QImage& kphotoalbumIcon = {}, const QImage& discoverIcon = {});
    QImage homeIcon;
    QImage kphotoalbumIcon;
    QImage discoverIcon;
};

class ToggleTokenRequest :public RemoteCommand
{
public:
    enum State {On, Off};
    ToggleTokenRequest(ImageId imageId = {}, const QString& token = {}, State state = {});
    ImageId imageId;
    QString token;
    State state;
};

}
#endif // REMOTECOMMAND_H
