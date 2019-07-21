/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef CATEGORYCOLLECTION_H
#define CATEGORYCOLLECTION_H

#include "Category.h"
#include "CategoryPtr.h"

#include <QList>

namespace DB
{

/**
   \class CategoryCollection
   This class is the collection of categories. It is the basic anchor point to categories.
*/

class CategoryCollection : public QObject
{
    Q_OBJECT

public:
    virtual CategoryPtr categoryForName(const QString &name) const = 0;
    virtual QStringList categoryNames() const = 0;
    virtual QStringList categoryTexts() const = 0;
    virtual void removeCategory(const QString &name) = 0;
    virtual void rename(const QString &oldName, const QString &newName) = 0;
    virtual QList<CategoryPtr> categories() const = 0;
    virtual void addCategory(const QString &text, const QString &icon, Category::ViewType type,
                             int thumbnailSize, bool show, bool positionable = false)
        = 0;
    virtual CategoryPtr categoryForSpecial(const Category::CategoryType type) const = 0;

signals:
    void categoryCollectionChanged();
    void categoryRemoved(const QString &categoryName);
    void itemRenamed(DB::Category *category, const QString &oldName, const QString &newName);
    void itemRemoved(DB::Category *category, const QString &name);

protected slots:
    void itemRenamed(const QString &oldName, const QString &newName);
    void itemRemoved(const QString &item);
};

}

#endif /* CATEGORYCOLLECTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
