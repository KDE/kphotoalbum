// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Settings.h"
#include <QSettings>
namespace RemoteControl
{

Settings &Settings::instance()
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
        Q_EMIT thumbnailSizeChanged();
    }
}

int Settings::categoryItemSize() const
{
    return QSettings().value(QStringLiteral("categoryItemSize"), 300).value<int>();
}

double Settings::overviewIconSize() const
{
    return QSettings().value(QStringLiteral("overviewIconSize"), 20).value<double>();
}

QColor Settings::backgroundColor() const
{
    return Qt::black;
}

QColor Settings::textColor() const
{
    return Qt::white;
}

void Settings::setCategoryItemSize(int size)
{
    if (size != categoryItemSize()) {
        QSettings().setValue(QStringLiteral("categoryItemSize"), size);
        Q_EMIT categoryItemSizeChanged();
    }
}

void Settings::setOverviewIconSize(double size)
{
    if (overviewIconSize() != size) {
        QSettings().setValue(QStringLiteral("overviewIconSize"), size);
        Q_EMIT overviewIconSizeChanged();
    }
}

} // namespace RemoteControl

#include "moc_Settings.cpp"
