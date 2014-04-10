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
                 << new CategoryListCommand
                 << new SearchCommand
                 << new CategoryItemListCommand
                 << new SearchResultCommand
                 << new ThumbnailRequest;

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

ImageUpdateCommand::ImageUpdateCommand(const QString& fileName, const QImage& image, ViewType type)
    :RemoteCommand(id()), fileName(fileName), image(image), type(type)
{
}

QString ImageUpdateCommand::id()
{
    return QString::fromUtf8("Image Update");
}

void ImageUpdateCommand::encode(QDataStream& stream) const
{
    stream << fileName;
    encodeImage(stream,image);
    stream << (int) type;
}

void ImageUpdateCommand::decode(QDataStream& stream)
{
    stream >> fileName;
    image = decodeImage(stream);
    stream >> (int&) type;
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


SearchCommand::SearchCommand(SearchType type, const SearchInfo& searchInfo)
    :RemoteCommand(id()), type(type), searchInfo(searchInfo)
{
}

QString SearchCommand::id()
{
    return QString::fromUtf8("SearchCommand");
}

void SearchCommand::encode(QDataStream& stream) const
{
    stream << (int) type << searchInfo;
}

void SearchCommand::decode(QDataStream& stream)
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


SearchResultCommand::SearchResultCommand(SearchType type, const QStringList& relativeFileNames)
    :RemoteCommand(id()), type(type), relativeFileNames(relativeFileNames)
{
}

QString SearchResultCommand::id()
{
    return QString::fromUtf8("Image Search Result");
}

void SearchResultCommand::encode(QDataStream& stream) const
{
    stream << (int) type << relativeFileNames;
}

void SearchResultCommand::decode(QDataStream& stream)
{
    stream >> (int&) type >> relativeFileNames;
}


ThumbnailRequest::ThumbnailRequest(const QString& fileName, const QSize& size, ViewType type)
    :RemoteCommand(id()), fileName(fileName), size(size), type(type)
{
}

QString ThumbnailRequest::id()
{
    return QString::fromUtf8("ThumbnailRequest");
}

void ThumbnailRequest::encode(QDataStream& stream) const
{
    stream << fileName << size << (int) type;
}

void ThumbnailRequest::decode(QDataStream& stream)
{
    stream >> fileName >> size >> (int&) type;
}
