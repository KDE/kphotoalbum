/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

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
        emit thumbnailSizeChanged();
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
        emit categoryItemSizeChanged();
    }
}

void Settings::setOverviewIconSize(double size)
{
    if (overviewIconSize() != size) {
        QSettings().setValue(QStringLiteral("overviewIconSize"), size);
        emit overviewIconSizeChanged();
    }
}

} // namespace RemoteControl
