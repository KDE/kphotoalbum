#include "Settings.h"

#include <QSettings>
#include <QDebug>
namespace RemoteControl {


Settings& Settings::instance()
{
    static Settings settings;
    return settings;
}

int Settings::thumbnailSize() const
{
    return QSettings().value(QStringLiteral("thumbnailSize"), 200).value<int>();
}

void Settings::setThumbnailSize(int size)
{
    if (size != thumbnailSize()) {
        QSettings().setValue(QStringLiteral("thumbnailSize"), size);
        emit thumbnailSizeChanged();
    }
}

} // namespace RemoteControl
