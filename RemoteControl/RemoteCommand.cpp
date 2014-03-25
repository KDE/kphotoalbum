#include "RemoteCommand.h"

#include <QBuffer>
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
                 << new CategoryItemListCommand;

        for (RemoteCommand* command : commands )
             map.insert(command->id(), command);
    }

    return *map[id];
}

void RemoteCommand::encodeImage(QBuffer& buffer, const QImage& image) const
{
    image.save(&buffer,"JPEG");
}

QImage RemoteCommand::decodeImage(QBuffer& buffer) const
{
    QImage result;
    result.load(&buffer, "JPEG");
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

void ImageUpdateCommand::encodeData(QBuffer& buffer) const
{
    QDataStream stream(&buffer);
    stream << index;
    encodeImage(buffer,image);
}

void ImageUpdateCommand::decodeData(QBuffer& buffer)
{
    QDataStream stream(&buffer);
    stream >> index;
    image = decodeImage(buffer);
}


ImageCountUpdateCommand::ImageCountUpdateCommand()
    :RemoteCommand(id())
{
}

QString ImageCountUpdateCommand::id()
{
    return QString::fromUtf8("Image Count Update");
}

void ImageCountUpdateCommand::encodeData(QBuffer& buffer) const
{
    QDataStream stream(&buffer);
    stream << count;
}

void ImageCountUpdateCommand::decodeData(QBuffer& buffer)
{
    QDataStream stream(&buffer);
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

void CategoryListCommand::encodeData(QBuffer& buffer) const
{
    QDataStream stream(&buffer);
    stream << categories.count();
    for (const Category& category : categories)
        stream << category.name << category.text << category.icon << category.enabled;
    stream << home << kphotoalbum;
}

void CategoryListCommand::decodeData(QBuffer& buffer)
{
    QDataStream stream(&buffer);
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

void RequestCategoryInfo::encodeData(QBuffer& buffer) const
{
    QDataStream stream(&buffer);
    stream << (int) type << searchInfo;
}

void RequestCategoryInfo::decodeData(QBuffer& buffer)
{
    QDataStream stream(&buffer);
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

void CategoryItemListCommand::encodeData(QBuffer& buffer) const
{
    QDataStream stream(&buffer);
    stream << items.count();
    for (const CategoryItem& item : items) {
        stream << item.text;
        encodeImage(buffer,item.icon);
    }
}

void CategoryItemListCommand::decodeData(QBuffer& buffer)
{
    QDataStream stream(&buffer);
    items.clear();
    int count;
    QString text;
    QImage icon;
    stream >> count;
    for (int i=0; i<count; ++i) {
        stream >> text;
        icon = decodeImage(buffer);
        items.append({text, icon});
    }
}

void CategoryItemListCommand::addItem(const QString& text, const QImage& icon)
{
    items.append({text,icon});
}
