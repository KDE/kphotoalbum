/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOTECOMMAND_H
#define REMOTECOMMAND_H

#include "SearchInfo.h"
#include "Types.h"

#include <QBuffer>
#include <QDataStream>
#include <QDate>
#include <QHash>
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QPair>
#include <QString>
#include <QStringList>
#include <memory>

namespace RemoteControl
{
Q_NAMESPACE
class SerializerInterface;

const int VERSION = 8;

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
    ToggleTokenRequest,
    ImageInfosResult,
    VideoRequest,
};
Q_ENUM_NS(CommandType)

class RemoteCommand
{
public:
    RemoteCommand(CommandType type);
    virtual ~RemoteCommand();
    virtual void encode(QDataStream &) const;
    virtual void decode(QDataStream &);
    CommandType commandType() const;

    void addSerializer(SerializerInterface *serializer);
    static std::unique_ptr<RemoteCommand> create(CommandType commandType);
    void clear()
    {
        // FIXME remove once I've figured out why it crashes in ~ImageInfosResult
        m_serializers.clear();
    }

private:
    QList<SerializerInterface *> m_serializers;
    CommandType m_type;
};

class ThumbnailResult : public RemoteCommand
{
public:
    ThumbnailResult(ImageId imageId = {}, const QString &label = {}, const QImage &image = QImage(), ViewType type = {});
    ImageId imageId;
    QString label;
    QImage image;
    ViewType type;
};

struct Category {
    QString name;
    QImage icon;
    bool enabled;
    CategoryViewType viewType;
};

class CategoryListResult : public RemoteCommand
{
public:
    CategoryListResult();
    QList<Category> categories;
};

class SearchRequest : public RemoteCommand
{
public:
    SearchRequest(SearchType type = {}, const SearchInfo &searchInfo = {}, int size = {});
    SearchType type;
    SearchInfo searchInfo;
    int size; // Only used for SearchType::Categories
    ImageId focusImage = -1;
};

class SearchResult : public RemoteCommand
{
public:
    SearchResult(SearchType type = {}, const QList<int> &result = {}, ImageId focusImage = {});
    SearchType type;
    QList<int> result;
    ImageId focusImage = -1;
};

class ThumbnailRequest : public RemoteCommand
{
public:
    ThumbnailRequest(ImageId imageId = {}, const QSize &size = {}, ViewType type = {});
    ImageId imageId;
    QSize size;
    ViewType type;
};

class ThumbnailCancelRequest : public RemoteCommand
{
public:
    ThumbnailCancelRequest(ImageId imageId = {}, ViewType type = {});
    ImageId imageId;
    ViewType type;
};

class TimeCommand : public RemoteCommand
{
public:
    TimeCommand();
    void encode(QDataStream &stream) const override;
    void decode(QDataStream &stream) override;
};

class ImageDetailsRequest : public RemoteCommand
{
public:
    ImageDetailsRequest(ImageId imageId = {});
    ImageId imageId;
};

struct CategoryItemDetails {
    CategoryItemDetails(const QString &name = {}, const QString &age = {})
        : name(name)
        , age(age)
    {
    }
    QString name;
    QString age;
};

using CategoryItemDetailsList = QList<CategoryItemDetails>;

class ImageDetailsResult : public RemoteCommand
{
public:
    ImageDetailsResult();
    QString fileName;
    QString date;
    QString description;
    QMap<QString, CategoryItemDetailsList> categories;
};

class CategoryItemsResult : public RemoteCommand
{
public:
    CategoryItemsResult(const QStringList &items = {});
    QStringList items;
};

class StaticImageRequest : public RemoteCommand
{
public:
    StaticImageRequest(int size = {});
    int size;
};

class StaticImageResult : public RemoteCommand
{
public:
    StaticImageResult();
    QImage homeIcon;
    QImage kphotoalbumIcon;
    QImage discoverIcon;
    QImage info;
    QImage slideShow;
    QImage slideShowSpeed;
    QImage stop;
};

class ToggleTokenRequest : public RemoteCommand
{
public:
    enum State { On,
                 Off };
    ToggleTokenRequest(ImageId imageId = {}, const QString &token = {}, State state = {});
    ImageId imageId;
    QString token;
    State state;
};

class ImageInfosResult : public RemoteCommand
{
public:
    ImageInfosResult(const QHash<int, QDate> &imageDates = {}, const QVector<int> &videos = {});
    ~ImageInfosResult();
    QHash<int, QDate> imageDates;
    QVector<int> videos; // ID's of all videos
};

class VideoRequest : public RemoteCommand
{
public:
    VideoRequest(ImageId imageId = {});
    ImageId imageId;
};

}
#endif // REMOTECOMMAND_H
