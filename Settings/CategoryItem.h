/* Copyright (C) 2003-2014 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef SETTINGS_CATEGORYITEM_H
#define SETTINGS_CATEGORYITEM_H

// Qt includes
#include <QListWidgetItem>

// Local includes
#include "DB/Category.h"

namespace DB
{

// Local classes
class MemberMap;

}

namespace Settings
{

class CategoryItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    CategoryItem(
        const QString &category,
        const QString &icon,
        DB::Category::ViewType type,
        int thumbnailSize,
        QListWidget *parent,
        bool positionable = false);
    void setLabel(const QString &label);
    void setPositionable(bool positionable);
    void submit(DB::MemberMap *memberMap);
    void removeFromDatabase();
    bool positionable() const;
    int thumbnailSize() const;
    DB::Category::ViewType viewType() const;
    void setThumbnailSize(int size);
    void setViewType(DB::Category::ViewType type);
    QString icon() const;
    void setIcon(const QString &icon);
    QString originalName() const;
    void markAsNewCategory();

protected:
    void renameCategory(DB::MemberMap *memberMap);

private: // Variables
    QString m_categoryOrig;
    QString m_iconOrig;
    bool m_positionable;
    bool m_positionableOrig;
    QString m_category;
    QString m_text;
    QString m_icon;
    DB::Category::ViewType m_type;
    DB::Category::ViewType m_typeOrig;
    int m_thumbnailSize;
    int m_thumbnailSizeOrig;
};

}

#endif // SETTINGS_CATEGORYITEM_H

// vi:expandtab:tabstop=4 shiftwidth=4:
