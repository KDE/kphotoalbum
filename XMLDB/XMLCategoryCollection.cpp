/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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

#include "XMLCategoryCollection.h"

#include "XMLCategory.h"

#include <DB/ImageDB.h>

#include <QList>

DB::CategoryPtr XMLDB::XMLCategoryCollection::categoryForName(const QString &name) const
{
    for (QList<DB::CategoryPtr>::ConstIterator it = m_categories.begin(); it != m_categories.end(); ++it) {
        if ((*it)->name() == name)
            return *it;
    }
    return DB::CategoryPtr();
}

void XMLDB::XMLCategoryCollection::addCategory(DB::CategoryPtr category)
{
    m_categories.append(category);
    if (category->isSpecialCategory()) {
        m_specialCategories[category->type()] = category;
    }
    connect(category.data(), &DB::Category::changed, this, &XMLCategoryCollection::categoryCollectionChanged);
    connect(category.data(), &DB::Category::itemRemoved, this, &XMLCategoryCollection::slotItemRemoved);
    connect(category.data(), &DB::Category::itemRenamed, this, &XMLCategoryCollection::slotItemRenamed);
    emit categoryCollectionChanged();
}

QStringList XMLDB::XMLCategoryCollection::categoryNames() const
{
    QStringList res;
    for (QList<DB::CategoryPtr>::ConstIterator it = m_categories.begin(); it != m_categories.end(); ++it) {
        res.append((*it)->name());
    }
    return res;
}

QStringList XMLDB::XMLCategoryCollection::categoryTexts() const
{
    QStringList res;
    for (QList<DB::CategoryPtr>::ConstIterator it = m_categories.begin(); it != m_categories.end(); ++it) {
        res.append((*it)->name());
    }
    return res;
}

void XMLDB::XMLCategoryCollection::removeCategory(const QString &name)
{
    for (QList<DB::CategoryPtr>::iterator it = m_categories.begin(); it != m_categories.end(); ++it) {
        if ((*it)->name() == name) {
            m_categories.erase(it);
            emit categoryRemoved(name);
            emit categoryCollectionChanged();
            return;
        }
    }
    Q_ASSERT_X(false, "removeCategory", "trying to remove non-existing category");
}

void XMLDB::XMLCategoryCollection::rename(const QString &oldName, const QString &newName)
{
    categoryForName(oldName)->setName(newName);
    DB::ImageDB::instance()->renameCategory(oldName, newName);
    emit categoryCollectionChanged();
}

QList<DB::CategoryPtr> XMLDB::XMLCategoryCollection::categories() const
{
    return m_categories;
}

void XMLDB::XMLCategoryCollection::addCategory(const QString &text, const QString &icon,
                                               DB::Category::ViewType type, int thumbnailSize, bool show, bool positionable)
{
    addCategory(DB::CategoryPtr(new XMLCategory(text, icon, type, thumbnailSize, show, positionable)));
}

DB::CategoryPtr XMLDB::XMLCategoryCollection::categoryForSpecial(const DB::Category::CategoryType type) const
{
    return m_specialCategories[type];
}

void XMLDB::XMLCategoryCollection::initIdMap()
{
    Q_FOREACH (DB::CategoryPtr categoryPtr, m_categories) {
        static_cast<XMLCategory *>(categoryPtr.data())->initIdMap();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
