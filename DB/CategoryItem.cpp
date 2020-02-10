/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "CategoryItem.h"

#include <QList>

DB::CategoryItem::~CategoryItem()
{
    for (QList<CategoryItem *>::ConstIterator it = mp_subcategories.constBegin(); it != mp_subcategories.constEnd(); ++it) {
        delete *it;
    }
}

DB::CategoryItem *DB::CategoryItem::clone() const
{
    CategoryItem *result = new CategoryItem(mp_name);
    for (QList<CategoryItem *>::ConstIterator it = mp_subcategories.constBegin(); it != mp_subcategories.constEnd(); ++it) {
        result->mp_subcategories.append((*it)->clone());
    }
    return result;
}

bool DB::CategoryItem::isDescendentOf(const QString &child, const QString &parent) const
{
    for (QList<CategoryItem *>::ConstIterator it = mp_subcategories.begin(); it != mp_subcategories.end(); ++it) {
        if (mp_name == parent) {
            if ((*it)->hasChild(child))
                return true;
        } else {
            if ((*it)->isDescendentOf(child, parent))
                return true;
        }
    }
    return false;
}

bool DB::CategoryItem::hasChild(const QString &child) const
{
    if (mp_name == child)
        return true;

    for (const CategoryItem *subcategory : mp_subcategories) {
        if (subcategory->hasChild(child))
            return true;
    }
    return false;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
