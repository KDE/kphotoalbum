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

namespace RemoteControl
{

const int VERSION = 4;

class RemoteCommand
{
public:
    RemoteCommand(const QString& id);
    virtual ~RemoteCommand() = default;
    virtual void encode(QDataStream&) const = 0;
    virtual void decode(QDataStream&) = 0;
    QString id() const;

    static RemoteCommand& command(const QString& id);

protected:
    void encodeImage(QDataStream& stream, const QImage& image) const;
    void encodeImageWithTransparentPixels(QDataStream& stream, const QImage& image) const;
    QImage decodeImage(QDataStream& stream) const;

private:
    QString m_id;
};

class ImageUpdateCommand :public RemoteCommand
{
public:
    ImageUpdateCommand(ImageId imageId = {}, const QString& label = {}, const QImage& image = QImage(), ViewType type = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
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

class CategoryListCommand :public RemoteCommand
{
public:
    CategoryListCommand();
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    QList<Category> categories;
};

class SearchCommand :public RemoteCommand
{
public:
    SearchCommand(SearchType type = {}, const SearchInfo& searchInfo = {}, int size = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    SearchType type;
    SearchInfo searchInfo;
    int size; // Only used for SearchType::Categories
};

class SearchResultCommand :public RemoteCommand
{
public:
    SearchResultCommand(SearchType type = {}, const QList<int>& result = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    SearchType type;
    QList<int> result;
};

class ThumbnailRequest :public RemoteCommand
{
public:
    ThumbnailRequest(ImageId imageId = {}, const QSize& size = {}, ViewType type = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    ImageId imageId;
    QSize size;
    ViewType type;
};

class CancelRequestCommand :public RemoteCommand
{
public:
    CancelRequestCommand(ImageId imageId = {}, ViewType type = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
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

class RequestDetails :public RemoteCommand
{
public:
    RequestDetails(ImageId imageId = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;

    ImageId imageId;
};

class ImageDetailsCommand :public RemoteCommand
{
public:
    ImageDetailsCommand();
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;

    QString fileName;
    QString date;
    QString description;
    QMap<QString,QStringList> categories;
};

class CategoryItems :public RemoteCommand
{
public:
    CategoryItems(const QStringList items = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;

    QStringList items;
};

class RequestHomePageImages :public RemoteCommand
{
public:
    RequestHomePageImages(int size = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;

    int size;
};

class HomePageData :public RemoteCommand
{
public:
    HomePageData(const QImage& homeIcon = {}, const QImage& kphotoalbumIcon = {}, const QImage& discoverIcon = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;

    QImage homeIcon;
    QImage kphotoalbumIcon;
    QImage discoverIcon;
};

class ToggleTokenCommand :public RemoteCommand
{
public:
    enum State {On, Off};
    ToggleTokenCommand(ImageId imageId = {}, const QString& token = {}, State state = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;

    ImageId imageId;
    QString token;
    State state;
};

}
#endif // REMOTECOMMAND_H
