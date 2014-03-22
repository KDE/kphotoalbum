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
        commands << new NextSlideCommand
                 << new PreviousSlideCommand
                 << new ImageUpdateCommand;

        for (RemoteCommand* command : commands )
             map.insert(command->id(), command);
    }

    return *map[id];
}


NextSlideCommand::NextSlideCommand()
    :RemoteCommand(id())
{
}

QString NextSlideCommand::id()
{
    return QString::fromUtf8("Next Slide");
}


PreviousSlideCommand::PreviousSlideCommand()
    :RemoteCommand(id())
{
}

QString PreviousSlideCommand::id()
{
    return QString::fromUtf8("Previous Slide");
}


ImageUpdateCommand::ImageUpdateCommand()
    :RemoteCommand(id())
{
}

QString ImageUpdateCommand::id()
{
    return QString::fromUtf8("Image Update");
}

void ImageUpdateCommand::encodeData(QBuffer& buffer) const
{
    image.save(&buffer,"JPEG");
}

void ImageUpdateCommand::decodeData(QBuffer& buffer)
{
    image.load(&buffer, "JPEG");
}
