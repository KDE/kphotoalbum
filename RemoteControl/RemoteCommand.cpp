#include "RemoteCommand.h"

#include <QElapsedTimer>
#include <QMap>
#include <QDebug>

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
                 << new SearchResultCommand
                 << new ThumbnailRequest
                 << new CancelRequestCommand
                 << new TimeCommand;

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

ImageUpdateCommand::ImageUpdateCommand(int imageId, const QString& label, const QImage& image, ViewType type)
    :RemoteCommand(id()), imageId(imageId), label(label), image(image), type(type)
{
}

QString ImageUpdateCommand::id()
{
    return QString::fromUtf8("Image Update");
}

void ImageUpdateCommand::encode(QDataStream& stream) const
{
    stream << imageId << label;
    encodeImage(stream,image);
    stream << (int) type;
}

void ImageUpdateCommand::decode(QDataStream& stream)
{
    stream >> imageId >> label;
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


SearchResultCommand::SearchResultCommand(SearchType type, const QList<int>& result)
    :RemoteCommand(id()), type(type), result(result)
{
}

QString SearchResultCommand::id()
{
    return QString::fromUtf8("Image Search Result");
}

void SearchResultCommand::encode(QDataStream& stream) const
{
    stream << (int) type << result;
}

void SearchResultCommand::decode(QDataStream& stream)
{
    stream >> (int&) type >> result;
}


ThumbnailRequest::ThumbnailRequest(int imageId, const QSize& size, ViewType type)
    :RemoteCommand(id()), imageId(imageId), size(size), type(type)
{
}

QString ThumbnailRequest::id()
{
    return QString::fromUtf8("ThumbnailRequest");
}

void ThumbnailRequest::encode(QDataStream& stream) const
{
    stream << imageId << size << (int) type;
}

void ThumbnailRequest::decode(QDataStream& stream)
{
    stream >> imageId >> size >> (int&) type;
}


RemoteControl::CancelRequestCommand::CancelRequestCommand(int imageId, ViewType type)
    :RemoteCommand(id()), imageId(imageId), type(type)
{
}

QString CancelRequestCommand::id()
{
    return QString::fromUtf8("CancelRequest");
}

void CancelRequestCommand::encode(QDataStream& stream) const
{
    stream << imageId << (int) type;
}

void CancelRequestCommand::decode(QDataStream& stream)
{
    stream >> imageId >> (int&) type;
}


TimeCommand::TimeCommand()
    :RemoteCommand(id())
{
}

QString TimeCommand::id()
{
    return QString::fromUtf8("TimeDump");
}

static void printElapsed()
{
    static QElapsedTimer timer;
    qDebug() << "Time since last dump: " << timer.elapsed();
    timer.restart();
}

void TimeCommand::encode(QDataStream&) const
{
    printElapsed();
}

void TimeCommand::decode(QDataStream&)
{
    printElapsed();
}
