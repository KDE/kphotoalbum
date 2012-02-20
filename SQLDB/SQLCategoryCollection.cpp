/*
  Copyright (C) 2006-2010 Tuomas Suutari <thsuut@utu.fi>

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
#include "SQLCategoryCollection.h"

#include "QueryHelper.h"
#include <QList>
#include "SQLNormalCategory.h"
#include "SQLTokensCategory.h"
#include "SQLFolderCategory.h"
#include "QueryErrors.h"

SQLDB::SQLCategoryCollection::SQLCategoryCollection(QueryHelper& queryHelper):
    _qh(queryHelper)
{
    // Categories, which should not have an entry in category table.
    _specialCategoryNames << QString::fromLatin1("Folder");
}


DB::CategoryPtr SQLDB::SQLCategoryCollection::categoryForName( const QString& name ) const
{
    DB::CategoryPtr p;

    if (name == QString::fromLatin1("Folder")) {
        p = new SQLFolderCategory(const_cast<QueryHelper*>(&_qh));
    }
    else {
        int categoryId;
        try {
            categoryId = _qh.categoryId(name);
        }
        catch (NotFoundError&) {
            return DB::CategoryPtr(0);
        }
        if (name == QString::fromLatin1("Tokens")) {
            p = new SQLTokensCategory(const_cast<QueryHelper*>(&_qh), categoryId);
        }
        else {
            p = new SQLNormalCategory(const_cast<QueryHelper*>(&_qh), categoryId);
        }
    }

    connect(p.data(), SIGNAL(changed()), this, SIGNAL(categoryCollectionChanged()));
    connect(p.data(), SIGNAL(itemRemoved(const QString&)),
            this, SLOT(itemRemoved(const QString&)));
    connect(p.data(), SIGNAL(itemRenamed(const QString&, const QString&)),
            this, SLOT(itemRenamed(const QString&, const QString&)));

    return p;
}

QStringList SQLDB::SQLCategoryCollection::categoryNames() const
{
    return _specialCategoryNames + _qh.categoryNames();
}

void SQLDB::SQLCategoryCollection::removeCategory( const QString& name )
{
    if (_specialCategoryNames.contains(name))
        return;

    try {
        _qh.removeCategory(name);
    }
    catch (NotFoundError&) {
        return;
    }

    emit categoryCollectionChanged();
}

void SQLDB::SQLCategoryCollection::rename(const QString& oldName, const QString& newName)
{
    categoryForName(oldName)->setName(newName);
}

QList<DB::CategoryPtr> SQLDB::SQLCategoryCollection::categories() const
{
    const QStringList cats = categoryNames();
     QList<DB::CategoryPtr> result;
    for( QStringList::ConstIterator it = cats.constBegin(); it != cats.constEnd(); ++it ) {
        result.append( categoryForName( *it ) );
    }
    return result;
}

void SQLDB::SQLCategoryCollection::addCategory(const QString& category,
                                               const QString& icon,
                                               DB::Category::ViewType type,
                                               int thumbnailSize, bool showIt)
{
    if (_specialCategoryNames.contains(category))
        return;

    try {
        _qh.insertCategory(category, icon, showIt, type, thumbnailSize);
    }
    catch (Error& e) {
        // Check if error occurred, because category already exists
        try {
            _qh.categoryId(category);
        }
        catch (Error&) {
            throw e; // Throw the original error
        }
        return; // Don't overwrite existing category
    }
    emit categoryCollectionChanged();
}

#include "SQLCategoryCollection.moc"
