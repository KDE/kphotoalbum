#include "RemoteCommand.h"

#include <QMap>

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
                 << new ImageCountUpdateCommand
                 << new CategoryListCommand
                 << new RequestCategoryInfo
                 << new CategoryItemListCommand
                 << new ImageSearchResult;

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

QImage RemoteCommand::decodeImage(QDataStream& stream) const
{
    QImage result;
    result.load(stream.device(), "JPEG");
    return result;
}

ImageUpdateCommand::ImageUpdateCommand(int index, const QImage& image)
    :RemoteCommand(id()), index(index), image(image)
{
}

QString ImageUpdateCommand::id()
{
    return QString::fromUtf8("Image Update");
}

void ImageUpdateCommand::encode(QDataStream& stream) const
{
    stream << index;
    encodeImage(stream,image);
}

void ImageUpdateCommand::decode(QDataStream& stream)
{
    stream >> index;
    image = decodeImage(stream);
}


ImageCountUpdateCommand::ImageCountUpdateCommand()
    :RemoteCommand(id())
{
}

QString ImageCountUpdateCommand::id()
{
    return QString::fromUtf8("Image Count Update");
}

void ImageCountUpdateCommand::encode(QDataStream& stream) const
{
    stream << count;
}

void ImageCountUpdateCommand::decode(QDataStream& stream)
{
    stream >> count;
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
    for (const Category& category : categories)
        stream << category.name << category.text << category.icon << category.enabled;
    stream << home << kphotoalbum;
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
        stream >> name >> text >> icon >> enabled;
        categories.append( {name, text, icon, enabled});
    }
    stream >> home >> kphotoalbum;
}


RequestCategoryInfo::RequestCategoryInfo(RequestType type, const SearchInfo& searchInfo)
    :RemoteCommand(id()), type(type), searchInfo(searchInfo)
{
}

QString RequestCategoryInfo::id()
{
    return QString::fromUtf8("Request Category Info");
}

void RequestCategoryInfo::encode(QDataStream& stream) const
{
    stream << (int) type << searchInfo;
}

void RequestCategoryInfo::decode(QDataStream& stream)
{
    stream >> (int&) type >> searchInfo;
}


CategoryItemListCommand::CategoryItemListCommand()
    :RemoteCommand(id())
{
}

QString CategoryItemListCommand::id()
{
    return QString::fromUtf8("Search Result");
}

void CategoryItemListCommand::encode(QDataStream& stream) const
{
    stream << items.count();
    for (const CategoryItem& item : items) {
        stream << item.text;
        encodeImage(stream,item.icon);
    }
}

void CategoryItemListCommand::decode(QDataStream& stream)
{
    items.clear();
    int count;
    QString text;
    QImage icon;
    stream >> count;
    for (int i=0; i<count; ++i) {
        stream >> text;
        icon = decodeImage(stream);
        items.append({text, icon});
    }
}

void CategoryItemListCommand::addItem(const QString& text, const QImage& icon)
{
    items.append({text,icon});
}


ImageSearchResult::ImageSearchResult(const QStringList& relativeFileNames)
    :RemoteCommand(id()), relativeFileNames(relativeFileNames)
{
}

QString ImageSearchResult::id()
{
    return QString::fromUtf8("Image Search Result");
}

void ImageSearchResult::encode(QDataStream& stream) const
{
    stream << relativeFileNames;
}

void ImageSearchResult::decode(QDataStream& stream)
{
    stream >> relativeFileNames;
}
