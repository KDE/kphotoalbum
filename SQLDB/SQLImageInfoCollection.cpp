/*
  Copyright (C) 2006-2007 Tuomas Suutari <thsuut@utu.fi>

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
//Added by qt3to4:
#include <Q3ValueList>

using namespace SQLDB;

SQLImageInfoCollection::SQLImageInfoCollection(QueryHelper& queryHelper):
    _qh(queryHelper),
    _lockingScope(0)
{
    Q3ValueList< QPair<int, QString> > l = _qh.mediaItemIdFileMap();
    for (Q3ValueList< QPair<int, QString> >::const_iterator i = l.begin();
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
            return DB::ImageInfoPtr(0);
        }
        _filenameIdMap.insert(relativeFilename, fileId);
    }

    // QMutexLocker locker(&_mutex);
    DB::ImageInfoPtr p = _infoPointers[fileId];
    if (!p) {
        // TODO: Use real context for prefetching

        // make a dummy prefetch list
        QList<int> prefetchIdList;
        for (int i = fileId; i < fileId + 1317; ++i)
            prefetchIdList << i;

        typedef QMap<int, DB::ImageInfoPtr> IdInfoMap;
        const IdInfoMap fileInfos = _qh.getInfosOfFiles(prefetchIdList);

        p = fileInfos[fileId];
        for (IdInfoMap::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i) {
            _infoPointers.insert(i.key(), i.value());
            setLocking(i.value());
        }
    }
    return p;
}

DB::ImageInfoPtr SQLImageInfoCollection::getImageInfoOf(const DB::ResultId& id) const
{
    const int prefetchWindowSize = 1000;

    // QMutexLocker locker(&_mutex);
    DB::ImageInfoPtr p = _infoPointers[id.fileId()];
    if (!p) {
        const int fileIdIndex = id.context()->indexOf(id);
        const int firstIndex = qMax(0, fileIdIndex - prefetchWindowSize / 2);
        const int onePastLastIndex = qMin(id.context()->size(), fileIdIndex + prefetchWindowSize / 2);
        Q_ASSERT(firstIndex < onePastLastIndex);
        QList<int> prefetchIdList;
        for (int i = firstIndex; i < onePastLastIndex; ++i)
            prefetchIdList << id.context()->at(i).fileId();

        typedef QMap<int, DB::ImageInfoPtr> IdInfoMap;
        const IdInfoMap fileInfos = _qh.getInfosOfFiles(prefetchIdList);

        p = fileInfos[id.fileId()];
        for (IdInfoMap::const_iterator i = fileInfos.begin(); i != fileInfos.end(); ++i) {
            _infoPointers.insert(i.key(), i.value());
            setLocking(i.value());
        }
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
         i != _infoPointers.end();) {

        // Check if only _infoPointers has reference to the pointer.
        if ((*i).count() == 1) {
            // Then it's not needed anymore, because new one could be
            // created easily by loading from the SQL database.
            i = _infoPointers.erase(i);
        }
        else
            ++i;
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
