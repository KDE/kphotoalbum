/*
  Copyright (C) 2005-2006 Jesper K. Pedersen <blackie@kde.org>
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#include "SQLCategory.h"
#include "QueryHelper.h"

SQLDB::SQLCategory::SQLCategory(QueryHelper* queryHelper, int categoryId):
    _qh(queryHelper),
    _categoryId(categoryId)
{
}

QString SQLDB::SQLCategory::name() const
{
    return _qh->categoryName(_categoryId);
}

void SQLDB::SQLCategory::setName(const QString& name)
{
    _qh->changeCategoryName(_categoryId, name);
    emit changed();
}

QString SQLDB::SQLCategory::iconName() const
{
    return _qh->categoryIcon(_categoryId);
}

void SQLDB::SQLCategory::setIconName(const QString& name)
{
    _qh->changeCategoryIcon(_categoryId, name);
    emit changed();
}

DB::Category::ViewType SQLDB::SQLCategory::viewType() const
{
    return _qh->categoryViewType(_categoryId);
}

void SQLDB::SQLCategory::setViewType(ViewType type)
{
    _qh->changeCategoryViewType(_categoryId, type);
    emit changed();
}

bool SQLDB::SQLCategory::doShow() const
{
    return _qh->categoryVisible(_categoryId);
}

void SQLDB::SQLCategory::setDoShow(bool b)
{
    _qh->changeCategoryVisible(_categoryId, b);
    emit changed();
}

bool SQLDB::SQLCategory::isSpecialCategory() const
{
    // Special categories have their own classes, so return false here
    return false;
}

void SQLDB::SQLCategory::setSpecialCategory(bool b)
{
    // Can't be special category, so do nothing
    Q_UNUSED(b);
}

QStringList SQLDB::SQLCategory::items() const
{
    return _qh->tagNamesOfCategory(_categoryId);
}

void SQLDB::SQLCategory::setItems(const QStringList& items)
{
    _qh->executeStatement("DELETE FROM tag WHERE categoryId=%s",
                          QueryHelper::Bindings() << _categoryId);

    uint place = items.count();
    for (QStringList::const_iterator it = items.begin();
        it != items.end(); ++it ) {
        _qh->executeStatement("INSERT INTO tag(name, categoryId, place) "
                              "VALUES(%s, %s, %s)",
                              QueryHelper::Bindings() << *it <<
                              _categoryId << place);
        --place;
    }
}

void SQLDB::SQLCategory::addItem(const QString& item)
{
    _qh->insertTagFirst(_categoryId, item);
}

void SQLDB::SQLCategory::removeItem(const QString& item)
{
    _qh->removeTag(_categoryId, item);
    emit itemRemoved(item);
}

void SQLDB::SQLCategory::renameItem(const QString& oldValue, const QString& newValue)
{
    _qh->executeStatement("UPDATE tag SET name=%s "
                          "WHERE name=%s AND categoryId=%s",
                          QueryHelper::Bindings() <<
                          newValue << oldValue << _categoryId);
    emit itemRenamed(oldValue, newValue);
}

int SQLDB::SQLCategory::thumbnailSize() const
{
    return _qh->executeQuery("SELECT thumbsize FROM category WHERE id=%s",
                             QueryHelper::Bindings() <<
                             _categoryId).firstItem().toInt();
}

void SQLDB::SQLCategory::setThumbnailSize(int size)
{
    _qh->executeStatement("UPDATE category SET thumbsize=%s WHERE id=%s",
                          QueryHelper::Bindings() << size << _categoryId);
}

#include "SQLCategory.moc"
