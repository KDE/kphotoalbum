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
#include "QueryErrors.h"

using namespace SQLDB;

SQLImageInfoCollection::SQLImageInfoCollection(QueryHelper& queryHelper):
    _qh(queryHelper),
    _lockingScope(0)
{
    QValueList< QPair<int, QString> > l = _qh.mediaItemIdFileMap();
    for (QValueList< QPair<int, QString> >::const_iterator i = l.begin();
         i != l.end(); ++i) {
        _filenameIdMap.insert((*i).second, (*i).first);
        _idFilenameMap.insert((*i).first, (*i).second);
    }
}

SQLImageInfoCollection::~SQLImageInfoCollection()
{
    unsetLock();

    // FIXME: shouldn't be needed?
    clearCache();
}

DB::ImageInfoPtr
SQLImageInfoCollection::getImageInfoOf(const QString& relativeFilename) const
{
    int fileId = _filenameIdMap[relativeFilename];
    if (!fileId) {
        try {
            fileId = _qh.mediaItemId(relativeFilename);
        }
        catch (NotFoundError& e) {
            return 0;
        }
        _filenameIdMap.insert(relativeFilename, fileId);
    }

    // QMutexLocker locker(&_mutex);
    DB::ImageInfoPtr p = _infoPointers[fileId];
    if (!p) {
        p = new SQLImageInfo(const_cast<QueryHelper*>(&_qh), fileId);
        _infoPointers.insert(fileId, p);
        setLocking(p);
    }
    return p;
}

QString SQLImageInfoCollection::filenameForId(int id) const
{
    QString filename = _idFilenameMap[id];
    if (filename.isNull()) {
        try {
             filename = _qh.mediaItemFilename(id);
        }
        catch (NotFoundError& e) {
            return QString::null;
        }
        _idFilenameMap.insert(id, filename);
    }
    return filename;
}

void SQLImageInfoCollection::setLock(const DB::ImageSearchInfo& scope, bool invert)
{
    unsetLock();
    _lockingScope = new DB::ImageSearchInfo(scope);
    _invertLock = invert;
    updateLockingInfo();

    // TODO: some other way to prevent locked images to show up in searches
}

void SQLImageInfoCollection::unsetLock()
{
    if (_lockingScope) {
        delete _lockingScope;
        _lockingScope = 0;
        updateLockingInfo();
    }
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
            (*i)->removeCategoryInfo(category->name(), item);
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

void SQLImageInfoCollection::setLocking(DB::ImageInfoPtr p) const
{
    if (_lockingScope)
        p->setLocked(_lockingScope->match(p) ^ _invertLock);
    else
        p->setLocked(false);
}

void SQLImageInfoCollection::updateLockingInfo() const
{
    for (QMap<int, DB::ImageInfoPtr>::iterator i = _infoPointers.begin();
         i != _infoPointers.end(); ++i)
        setLocking(*i);
}

#include "SQLImageInfoCollection.moc"
