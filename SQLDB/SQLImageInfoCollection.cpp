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

#include "SQLImageInfoCollection.h"
#include "SQLImageInfo.h"
#include "QueryHelper.h"
#include "QueryErrors.h"

using namespace SQLDB;

SQLImageInfoCollection::SQLImageInfoCollection(/* DatabaseConnection* connection */)
{
}

SQLImageInfoCollection::~SQLImageInfoCollection()
{
    clearCache();
}

DB::ImageInfoPtr
SQLImageInfoCollection::getImageInfoOf(const QString& relativeFilename) const
{
    int fileId;
    try {
        fileId = QueryHelper::instance()->mediaItemId(relativeFilename);
    }
    catch (NotFoundError& e) {
        return 0;
    }

    // QMutexLocker locker(&_mutex);
    DB::ImageInfoPtr p = _infoPointers[fileId];
    if (!p) {
        p = new SQLImageInfo(fileId);
        _infoPointers.insert(fileId, p);
    }
    return p;
}

void SQLImageInfoCollection::clearCache()
{
    // QMutexLocker locker(&_mutex);
    for (QMap<int, DB::ImageInfoPtr>::iterator i = _infoPointers.begin();
         i != _infoPointers.end(); ++i) {

        // Check if only _infoPointers has reference to the pointer.
        if ((*i).count() == 1) {
            // Then it's not needed anymore, because new one could be
            // created easily by loading from the SQL database.
            _infoPointers.remove(i);
        }
    }
}

void SQLImageInfoCollection::deleteTag(DB::Category* category,
                                       const QString& item)
{
    if (category) {
        // QMutexLocker locker(&_mutex);
        for (QMap<int, DB::ImageInfoPtr>::iterator i = _infoPointers.begin();
             i != _infoPointers.end(); ++i)
            (*i)->removeOption(category->name(), item);
    }
}

void SQLImageInfoCollection::renameTag(DB::Category* category,
                                       const QString& oldName,
                                       const QString& newName)
{
    if (category) {
        // QMutexLocker locker(&_mutex);
        for (QMap<int, DB::ImageInfoPtr>::iterator i = _infoPointers.begin();
             i != _infoPointers.end(); ++i)
            (*i)->renameItem(category->name(), oldName, newName);
    }
}

#include "SQLImageInfoCollection.moc"
