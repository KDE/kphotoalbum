/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DB_CATEGORYITEMS_H
#define DB_CATEGORYITEMS_H

#include <QExplicitlySharedDataPointer>
#include <QList>
#include <qstring.h>

namespace DB
{
class CategoryItem : public QSharedData
{
public:
    explicit CategoryItem(const QString &name, bool isTop = false)
        : mp_name(name)
        , mp_isTop(isTop)
    {
    }
    ~CategoryItem();
    CategoryItem *clone() const;
    bool isDescendentOf(const QString &child, const QString &parent) const;

protected:
    bool hasChild(const QString &child) const;

public:
    QString mp_name;
    QList<CategoryItem *> mp_subcategories;
    bool mp_isTop;
};

typedef QExplicitlySharedDataPointer<CategoryItem> CategoryItemPtr;
}

#endif /* DB_CATEGORYITEMS_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
