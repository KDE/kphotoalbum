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
const int VERSION = 6;

class SerializerInterface;

class RemoteCommand
{
public:
    RemoteCommand(const QString& id);
    virtual ~RemoteCommand();
    virtual void encode(QDataStream&) const;
    virtual void decode(QDataStream&);
    QString id() const;

    void addSerializer(SerializerInterface* serializer);
    static std::unique_ptr<RemoteCommand> create(const QString& id);

private:
    QList<SerializerInterface*> m_serializers;
    QString m_id;
};

// ThumbnailResult
class ThumbnailResult :public RemoteCommand
{
public:
    ThumbnailResult(ImageId imageId = {}, const QString& label = {}, const QImage& image = QImage(), ViewType type = {});
    static QString id();
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

// CategoryListResult
class CategoryListResult :public RemoteCommand
{
public:
    CategoryListResult();
    static QString id();
    QList<Category> categories;
};

// SearchRequest
class SearchRequest :public RemoteCommand
{
public:
    SearchRequest(SearchType type = {}, const SearchInfo& searchInfo = {}, int size = {});
    static QString id();
    SearchType type;
    SearchInfo searchInfo;
    int size; // Only used for SearchType::Categories
};

// SearchResult
class SearchResult :public RemoteCommand
{
public:
    SearchResult(SearchType type = {}, const QList<int>& result = {});
    static QString id();
    SearchType type;
    QList<int> result;
};

class ThumbnailRequest :public RemoteCommand
{
public:
    ThumbnailRequest(ImageId imageId = {}, const QSize& size = {}, ViewType type = {});
    static QString id();
    ImageId imageId;
    QSize size;
    ViewType type;
};

// ThumbnailCancelRequest
class ThumbnailCancelRequest :public RemoteCommand
{
public:
    ThumbnailCancelRequest(ImageId imageId = {}, ViewType type = {});
    static QString id();
    ImageId imageId;
    ViewType type;
};

class TimeCommand :public RemoteCommand
{
public:
    TimeCommand();
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
};

// ImageDetailsRequest
class ImageDetailsRequest :public RemoteCommand
{
public:
    ImageDetailsRequest(ImageId imageId = {});
    static QString id();
    ImageId imageId;
};

struct CategoryItemDetails {
    CategoryItemDetails(const QString& name = {}, const QString& age = {})
        : name(name), age(age) {}
    QString name;
    QString age;
};

using CategoryItemDetailsList = QList<CategoryItemDetails>;

// ImageDetailsResult
class ImageDetailsResult :public RemoteCommand
{
public:
    ImageDetailsResult();
    static QString id();
    QString fileName;
    QString date;
    QString description;
    QMap<QString,CategoryItemDetailsList> categories;
};

class CategoryItemsResult :public RemoteCommand
{
public:
    CategoryItemsResult(const QStringList& items = {});
    static QString id();
    QStringList items;
};

// StaticImageRequest
class StaticImageRequest :public RemoteCommand
{
public:
    StaticImageRequest(int size = {});
    static QString id();
    int size;
};

// StaticImageResult
class StaticImageResult :public RemoteCommand
{
public:
    StaticImageResult(const QImage& homeIcon = {}, const QImage& kphotoalbumIcon = {}, const QImage& discoverIcon = {});
    static QString id();
    QImage homeIcon;
    QImage kphotoalbumIcon;
    QImage discoverIcon;
};

// ToggleTokenRequest
class ToggleTokenRequest :public RemoteCommand
{
public:
    enum State {On, Off};
    ToggleTokenRequest(ImageId imageId = {}, const QString& token = {}, State state = {});
    static QString id();
    ImageId imageId;
    QString token;
    State state;
};

}
#endif // REMOTECOMMAND_H
