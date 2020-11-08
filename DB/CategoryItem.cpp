/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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

    for (const CategoryItem *subcategory : qAsConst(mp_subcategories)) {
        if (subcategory->hasChild(child))
            return true;
    }
    return false;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
