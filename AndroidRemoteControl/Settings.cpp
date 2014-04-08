#include "Settings.h"

#include <QSettings>
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

int Settings::categoryItemSize() const
{
    return QSettings().value(QStringLiteral("categoryItemSize"), 300).value<int>();
}

void Settings::setCategoryItemSize(int size)
{
    if (size != categoryItemSize()) {
        QSettings().setValue(QStringLiteral("categoryItemSize"), size);

        emit categoryItemSizeChanged();
    }
}

} // namespace RemoteControl
