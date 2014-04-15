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
    ImageUpdateCommand(int imageId = {}, const QString& label = {}, const QImage& image = QImage(), ViewType type = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    int imageId;
    QString label;
    QImage image;
    ViewType type;
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

class SearchCommand :public RemoteCommand
{
public:
    SearchCommand(SearchType type = {}, const SearchInfo& searchInfo = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    SearchType type;
    SearchInfo searchInfo;
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
    // FIXME: Multiple constructors?
    ThumbnailRequest(int imageId = {}, const QSize& size = {}, ViewType type = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    int imageId;
    QSize size;
    ViewType type;
    QString category;
};

class CancelRequestCommand :public RemoteCommand
{
public:
    CancelRequestCommand(int imageId = {}, ViewType type = {});
    static QString id();
    void encode(QDataStream& stream) const override;
    void decode(QDataStream& stream) override;
    int imageId;
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
}
#endif // REMOTECOMMAND_H
