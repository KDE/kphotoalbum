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
#ifndef XMLCATEGORYCOLLECTION_H
#define XMLCATEGORYCOLLECTION_H

#include "DB/CategoryCollection.h"
#include <QList>
#include <QMap>

namespace XMLDB
{

class XMLCategoryCollection : public DB::CategoryCollection
{
    Q_OBJECT

public:
    DB::CategoryPtr categoryForName(const QString &name) const override;
    void addCategory(DB::CategoryPtr);
    QStringList categoryNames() const override;
    QStringList categoryTexts() const override;
    void removeCategory(const QString &name) override;
    void rename(const QString &oldName, const QString &newName) override;
    QList<DB::CategoryPtr> categories() const override;
    void addCategory(const QString &text, const QString &icon, DB::Category::ViewType type,
                     int thumbnailSize, bool show, bool positionable = false) override;
    DB::CategoryPtr categoryForSpecial(const DB::Category::CategoryType type) const override;

    void initIdMap();

private:
    QList<DB::CategoryPtr> m_categories;
    QMap<DB::Category::CategoryType, DB::CategoryPtr> m_specialCategories;
};
}

#endif /* XMLCATEGORYCOLLECTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
