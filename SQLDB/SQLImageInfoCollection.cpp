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

using namespace SQLDB;

SQLImageInfoCollection::SQLImageInfoCollection(/* DatabaseConnection* connection */)
{
}

DB::ImageInfoPtr
SQLImageInfoCollection::getImageInfoOf(const QString& relativeFilename) const
{
    int fileId;
    try {
        fileId = QueryHelper::instance()->idForFilename(relativeFilename);
    }
    catch (NotFoundError& e) {
        return 0;
    }

    _mutex.lock();
    DB::ImageInfoPtr p = _infoPointers[fileId];
    if (p) {
        p->saveChanges();
    }
    else {
        p = new SQLImageInfo(fileId);
        _infoPointers.insert(fileId, p);
    }
    _mutex.unlock();
    return p;
}

void SQLImageInfoCollection::clearCache()
{
    _mutex.lock();
    for (QMap<int, DB::ImageInfoPtr>::iterator i = _infoPointers.begin();
         i != _infoPointers.end(); ++i) {

        // Check if only _infoPointers has reference to the pointer.
        if ((*i).count() == 1) {
            // Then it's not needed anymore, because new one could be
            // created easily by loading from the SQL database.
            _infoPointers.remove(i);
        }
    }
    _mutex.unlock();
}
