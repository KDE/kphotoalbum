#ifndef REMOTECOMMAND_H
#define REMOTECOMMAND_H

#include "SearchInfo.h"

#include <QString>
#include <QDataStream>
#include <QImage>
#include <QBuffer>
#include <QStringList>

namespace RemoteControl
{

class RemoteCommand
{
public:
    RemoteCommand(const QString& id);
    virtual ~RemoteCommand() = default;
    virtual void encodeData(QBuffer&) const {};
    virtual void decodeData(QBuffer&) {};
    QString id() const;

    static RemoteCommand& command(const QString& id);

private:
    QString m_id;
};

class ImageUpdateCommand :public RemoteCommand
{
public:
    ImageUpdateCommand(int index = -1, const QImage& image = QImage());
    static QString id();
    void encodeData(QBuffer& buffer) const override;
    void decodeData(QBuffer& buffer) override;
    int index;
    QImage image;
};

class ImageCountUpdateCommand :public RemoteCommand
{
public:
    ImageCountUpdateCommand();
    static QString id();
    void encodeData(QBuffer& buffer) const override;
    void decodeData(QBuffer& buffer) override;
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
    void encodeData(QBuffer& buffer) const override;
    void decodeData(QBuffer& buffer) override;
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
    void encodeData(QBuffer& buffer) const override;
    void decodeData(QBuffer& buffer) override;
    RequestType type;
    SearchInfo searchInfo;
};

class SearchResult :public RemoteCommand
{
public:
    SearchResult(const QStringList& relativeFileNameList = {});
    static QString id();
    void encodeData(QBuffer& buffer) const override;
    void decodeData(QBuffer& buffer) override;
    QStringList relativeFileNameList;
};

}
#endif // REMOTECOMMAND_H
