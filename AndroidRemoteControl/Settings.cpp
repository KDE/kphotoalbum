#include "Settings.h"

#include <QSettings>

namespace RemoteControl {


Settings& Settings::instance()
{
    static Settings settings;
    return settings;
}

double Settings::thumbnailScale() const
{
    return QSettings().value(QStringLiteral("thumbnailScale"), 1.0).value<double>();
}

void Settings::setThumbnailScale(double scale)
{
    if (scale != thumbnailScale()) {
        QSettings().setValue(QStringLiteral("thumbnailScale"), scale);
        emit thumbnailScaleChanged(scale);
    }
}

} // namespace RemoteControl
