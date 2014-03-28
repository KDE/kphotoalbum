#ifndef REMOTECOMMAND_H
#define REMOTECOMMAND_H

#include "SearchInfo.h"

#include <QString>
#include <QDataStream>
#include <QImage>
#include <QBuffer>
#include <QStringList>
#include <QPair>

namespace RemoteControl
{

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
    QImage decodeImage(QDataStream& buffer) const;
private:
    QString m_id;
};

class ImageUpdateCommand :public RemoteCommand
{
public:
    ImageUpdateCommand(int index = -1, const QImage& image = QImage());
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    int index;
    QImage image;
};

class ImageCountUpdateCommand :public RemoteCommand
{
public:
    ImageCountUpdateCommand();
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    int count;
};

struct Category {
    QString name;
    QString text;
    QImage icon;
    bool enabled;
};

class CategoryListCommand :public RemoteCommand
{
public:
    CategoryListCommand();
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    QList<Category> categories;
    QImage home;
    QImage kphotoalbum;
};

class RequestCategoryInfo :public RemoteCommand
{
public:
    enum RequestType { RequestCategoryNames, RequestCategoryValues };
    RequestCategoryInfo(RequestType type = {}, const SearchInfo& searchInfo = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    RequestType type;
    SearchInfo searchInfo;
};

struct CategoryItem {
    QString text;
    QImage icon;
};

using CategoryItemsList = QList<CategoryItem>;
class CategoryItemListCommand :public RemoteCommand
{
public:
    CategoryItemListCommand();
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    void addItem(const QString& text, const QImage& icon);
    CategoryItemsList items;
};

}
#endif // REMOTECOMMAND_H
