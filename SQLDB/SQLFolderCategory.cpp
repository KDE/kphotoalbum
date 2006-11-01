/*
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

#include "SQLFolderCategory.h"
#include "QueryHelper.h"

using namespace SQLDB;

SQLFolderCategory::SQLFolderCategory(QueryHelper* queryHelper):
    _qh(queryHelper)
{
}

QStringList SQLFolderCategory::items() const
{
    return _qh->folders();
}

void SQLFolderCategory::setItems(const QStringList& items)
{
    Q_UNUSED(items);
}

void SQLFolderCategory::addOrReorderItems(const QStringList& items)
{
    Q_UNUSED(items);
}

void SQLFolderCategory::addItem(const QString& item)
{
    Q_UNUSED(item);
}

void SQLFolderCategory::removeItem(const QString& item)
{
    Q_UNUSED(item);
}

void SQLFolderCategory::renameItem(const QString& oldValue,
                                   const QString& newValue)
{
    Q_UNUSED(oldValue);
    Q_UNUSED(newValue);
}

QMap<QString, uint>
SQLFolderCategory::classify(const DB::ImageSearchInfo& scope,
                            DB::MediaType typemask) const
{
    // TODO: this
    QValueList<int>* scopePointer;
    QValueList<int> includedFiles;
    if (scope.isNull())
        scopePointer = 0;
    else {
        includedFiles = _qh->searchMediaItems(scope, typemask);
        scopePointer = &includedFiles;
    }

    return _qh->classify(name(), typemask, scopePointer);
}

#include "SQLFolderCategory.moc"
