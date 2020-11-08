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

#ifndef REMOTECONTROL_SETTINGS_H
#define REMOTECONTROL_SETTINGS_H

#include <QColor>
#include <QObject>

namespace RemoteControl
{

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int thumbnailSize READ thumbnailSize WRITE setThumbnailSize NOTIFY thumbnailSizeChanged)
    Q_PROPERTY(int categoryItemSize READ categoryItemSize WRITE setCategoryItemSize NOTIFY categoryItemSizeChanged)
    Q_PROPERTY(double overviewIconSize READ overviewIconSize WRITE setOverviewIconSize NOTIFY overviewIconSizeChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor textColor READ textColor NOTIFY foregroundColorChanged)

    double m_thumbnailScale;

public:
    static Settings &instance();
    int thumbnailSize() const;
    void setThumbnailSize(int size);
    int categoryItemSize() const;
    double overviewIconSize() const;

    QColor backgroundColor() const;

    QColor textColor() const;

public slots:
    void setCategoryItemSize(int size);
    void setOverviewIconSize(double size);

signals:
    void thumbnailSizeChanged();
    void categoryItemSizeChanged();
    void overviewIconSizeChanged();
    void backgroundColorChanged();
    void foregroundColorChanged();

private:
    explicit Settings() = default;
    int m_categoryItemSize;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SETTINGS_H
