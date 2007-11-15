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
#include "Database.h"
#include "SQLCategory.h"
#include "DB/ImageInfo.h"
#include "Utilities/Util.h"
#include "DB/GroupCounter.h"
#include "SQLImageInfoCollection.h"
#include "Browser/BrowserWidget.h"
#include "SQLImageDateCollection.h"
#include "DB/MediaCount.h"
#include "DatabaseInitialization.h"
#include "QueryErrors.h"

namespace
{
    QStringList stripImageDirectoryFromList(const QStringList& absolutePaths)
    {
        QStringList relativePaths = absolutePaths;
        for (QStringList::iterator i = relativePaths.begin();
             i != relativePaths.end(); ++i) {
        *i = Utilities::stripImageDirectory(*i);
        }
        return relativePaths;
    }
}

SQLDB::Database::Database(const DatabaseAddress& address):
    _address(address),
    _connection(initializeKPhotoAlbumDatabase(_address)),
    _qh(*_connection),
    _categoryCollection(_qh),
    _members(_qh),
    _infoCollection(_qh),
    _md5map(_qh)
{
    connect(categoryCollection(),
            SIGNAL(itemRemoved(DB::Category*, const QString&)),
            &_infoCollection,
            SLOT(deleteTag(DB::Category*, const QString&)));
    connect(categoryCollection(),
            SIGNAL(itemRenamed(DB::Category*, const QString&, const QString&)),
            &_infoCollection,
            SLOT(renameTag(DB::Category*, const QString&, const QString&)));
}

bool SQLDB::Database::operator==(const DB::ImageDB& other) const
{
    const SQLDB::Database* sqlOther =
        dynamic_cast<const SQLDB::Database*>(&other);
    if (!sqlOther)
        return false;

    return _address == sqlOther->_address;
}

uint SQLDB::Database::totalCount() const
{
    return _qh.mediaItemCount();
}

DB::MediaCount SQLDB::Database::count(const DB::ImageSearchInfo& searchInfo)
{
    QValueList<int> mediaIds;
    QValueList<int>* scope = 0;
    bool all = (searchInfo.query().count() == 0);
    if (!all) {
        mediaIds = _qh.searchMediaItems(searchInfo);
        scope = &mediaIds;
    }

    return DB::MediaCount(_qh.mediaItemCount(DB::Image, scope),
                          _qh.mediaItemCount(DB::Video, scope));
}

QStringList SQLDB::Database::search( const DB::ImageSearchInfo& info, bool requireOnDisk ) const
{
    QValueList<int> matches = _qh.searchMediaItems(info);
    QStringList result;
    QString imageRoot = Settings::SettingsData::instance()->imageDirectory();
    for( QValueList<int>::Iterator it = matches.begin(); it != matches.end(); ++it ) {
        QString fullPath = imageRoot + _infoCollection.filenameForId(*it);
        if (requireOnDisk && !DB::ImageInfo::imageOnDisk(fullPath))
            continue;
        result.append(fullPath);
    }
    return result;
}

void SQLDB::Database::renameCategory(const QString& /*oldName*/, const QString /*newName*/)
{
    // TODO: this
}

QMap<QString, uint> SQLDB::Database::classify(const DB::ImageSearchInfo& info,
                                              const QString& category,
                                              DB::MediaType typemask)
{
    DB::CategoryPtr catptr = categoryCollection()->categoryForName(category);
    SQLCategory* sqlcatptr = static_cast<SQLCategory*>(catptr.data());
    if (sqlcatptr)
        return sqlcatptr->classify(info, typemask);
    else
        return QMap<QString, uint>();
}

QStringList SQLDB::Database::imageList( bool withRelativePath )
{
    QStringList relativePaths = _qh.filenames();
    if (withRelativePath)
        return relativePaths;
    else {
        QString imageRoot = Settings::SettingsData::instance()->imageDirectory();
        QStringList absolutePaths;
        for (QStringList::const_iterator i = relativePaths.begin();
             i != relativePaths.end(); ++i) {
            absolutePaths << imageRoot + (*i);
        }
        return absolutePaths;
    }
}


QStringList SQLDB::Database::images()
{
    return imageList( false );
}

void SQLDB::Database::addImages( const DB::ImageInfoList& images )
{
    if (!images.isEmpty()) {
        _qh.insertMediaItemsLast(images);
        emit totalChanged(totalCount());
    }
}

void SQLDB::Database::addToBlockList(const QStringList& list)
{
    QStringList relativePaths = list;
    for (QStringList::iterator i = relativePaths.begin();
         i != relativePaths.end(); ++i) {
        *i = Utilities::stripImageDirectory(*i);
    }
    _qh.addBlockItems(relativePaths);
    deleteList(list);
}

bool SQLDB::Database::isBlocking(const QString& fileName)
{
    return _qh.isBlocked(fileName);
}

void SQLDB::Database::deleteList( const QStringList& list )
{
    if (!list.isEmpty()) {
        for (QStringList::const_iterator i = list.begin();
             i != list.end(); ++i) {
            _qh.removeMediaItem(Utilities::stripImageDirectory(*i));
        }
        emit totalChanged(totalCount());
    }
}

DB::ImageInfoPtr SQLDB::Database::info( const QString& fileName ) const
{
    return _infoCollection.
        getImageInfoOf(Utilities::stripImageDirectory(fileName));
}

DB::MemberMap& SQLDB::Database::memberMap()
{
    return _members;
}

void SQLDB::Database::save(const QString& /*fileName*/, bool isAutoSave)
{
    qDebug("NYI: void SQLDB::Database::save( const QString& fileName )" );
    _infoCollection.clearCache();

#ifndef DEBUG_QUERY_TIMES
    Q_UNUSED(isAutoSave)
#else
    if (isAutoSave)
        return;

    QStringList timeQueryList;

    for (QValueList< QPair<QString, uint> >::const_iterator i =
             _qh.queryTimes.begin(); i != _qh.queryTimes.end(); ++i) {
        timeQueryList <<
            QString::number((*i).second).rightJustify(8) +
            QString::fromLatin1(" ") + (*i).first;
    }

    timeQueryList.sort();

    for (QStringList::const_iterator i = timeQueryList.begin();
         i != timeQueryList.end(); ++i)
        qDebug("%s", (*i).local8Bit().data());
#endif
}

DB::MD5Map* SQLDB::Database::md5Map()
{
    return &_md5map;
}

void SQLDB::Database::lockDB(bool lock, bool exclude)
{
    if (lock)
        _infoCollection.setLock(Settings::SettingsData::instance()->
                                currentLock(), !exclude);
    else
        _infoCollection.unsetLock();
}

DB::CategoryCollection* SQLDB::Database::categoryCollection()
{
    return &_categoryCollection;
}

KSharedPtr<DB::ImageDateCollection> SQLDB::Database::rangeCollection()
{
    return new SQLImageDateCollection(_qh
                                      /*, search(Browser::instance()->currentContext(), false)*/);
}

void SQLDB::Database::reorder(const QString& item,
                              const QStringList& selection, bool after)
{
    _qh.moveMediaItems(stripImageDirectoryFromList(selection),
                       Utilities::stripImageDirectory(item), after);
}

void SQLDB::Database::sortAndMergeBackIn(const QStringList& fileList)
{
    _qh.sortMediaItems(stripImageDirectoryFromList(fileList));
}

QString
SQLDB::Database::findFirstItemInRange(const DB::ImageDate& range,
                                      bool includeRanges,
                                      const QValueVector<QString>& images) const
{
    if (images.count() == totalCount()) {
        return Settings::SettingsData::instance()->imageDirectory() +
            _qh.findFirstFileInTimeRange(range, includeRanges);
    }
    else {
        QValueList<int> idList;
        for (QValueVector<QString>::const_iterator i = images.begin();
             i != images.end(); ++i) {
            idList << _qh.mediaItemId(Utilities::stripImageDirectory(*i));
        }
        return Settings::SettingsData::instance()->imageDirectory() +
            _qh.findFirstFileInTimeRange(range, includeRanges, idList);
    }
}


void SQLDB::Database::cutToClipboard( const QStringList& /*list*/ )
{
    // Not used yet anywhere, so not implemented either. ;)
}

QStringList SQLDB::Database::pasteFromCliboard( const QString& /*afterFile*/ )
{
    // Not implemented.
    return QStringList();
}

bool SQLDB::Database::isClipboardEmpty()
{
    // Not implemented.
    return true;
}

#include "Database.moc"
