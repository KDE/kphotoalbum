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
#include "Database.h"
#include "DB/Result.h"
#include "SQLCategory.h"
#include "DB/ImageInfo.h"
#include "DB/ImageInfoPtr.h"
#include "Utilities/Util.h"
#include "DB/GroupCounter.h"
#include "SQLImageInfoCollection.h"
#include "Browser/BrowserWidget.h"
#include "SQLImageDateCollection.h"
#include "DB/MediaCount.h"
#include "DatabaseInitialization.h"
#include "QueryErrors.h"
#include <QList>
#ifdef HAVE_EXIV2
#   include "Exif/Database.h"
#endif

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
    QList<int> mediaIds;
    QList<int>* scope = 0;
    bool all = (searchInfo.query().count() == 0);
    if (!all) {
        mediaIds = _qh.searchMediaItems(searchInfo);
        scope = &mediaIds;
    }

    return DB::MediaCount(_qh.mediaItemCount(DB::Image, scope),
                          _qh.mediaItemCount(DB::Video, scope));
}

DB::ResultPtr SQLDB::Database::search( const DB::ImageSearchInfo& info, bool requireOnDisk ) const
{
    QList<int> matches = _qh.searchMediaItems(info);
    QList<int> result;
    QString imageRoot = Settings::SettingsData::instance()->imageDirectory();
    for(QList<int>::Iterator it = matches.begin(); it != matches.end(); ++it) {
        QString fullPath = imageRoot + _infoCollection.filenameForId(*it);
        if (requireOnDisk && !DB::ImageInfo::imageOnDisk(fullPath))
            continue;
        result.append(*it);
    }
    return DB::ResultPtr( new DB::Result(result) );
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

DB::ResultPtr SQLDB::Database::imageList()
{
    return DB::ResultPtr( new DB::Result(_qh.mediaItemIds(DB::anyMediaType) ) );
}


DB::ResultPtr SQLDB::Database::images()
{
    return imageList();
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
    _qh.addIgnoredFiles(relativePaths);
    deleteList(list);
}

bool SQLDB::Database::isBlocking(const QString& fileName)
{
    return _qh.isIgnored(fileName);
}

void SQLDB::Database::deleteList( const QStringList& list )
{
    if (!list.isEmpty()) {
        for (QStringList::const_iterator i = list.begin();
             i != list.end(); ++i) {
#ifdef HAVE_EXIV2
            Exif::Database::instance()->remove( Utilities::imageFileNameToAbsolute(*i) );
#endif
            _qh.removeMediaItem(Utilities::stripImageDirectory(*i));
        }
        emit totalChanged(totalCount());
    }
}

DB::ImageInfoPtr SQLDB::Database::info( const QString& fileName, DB::PathType type ) const
{
    // PENDING(blackie) This code first converts to absolute, and then strips, that's just plane stupid....
    QString name = fileName;
    if ( type == DB::RelativeToImageRoot )
        name = Settings::SettingsData::instance()->imageDirectory() + fileName;

    return _infoCollection.
        getImageInfoOf(Utilities::stripImageDirectory(name));
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

    for (QList< QPair<QString, uint> >::const_iterator i =
             _qh.queryTimes.begin(); i != _qh.queryTimes.end(); ++i) {
        timeQueryList <<
            QString::number((*i).second).rightJustified(8) +
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
    return KSharedPtr<DB::ImageDateCollection>
        (static_cast<DB::ImageDateCollection*>
         (new SQLImageDateCollection(_qh
                                     /*, search(Browser::instance()->currentContext(), false)*/
                                     )));
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

QString SQLDB::Database::findFirstItemInRange(const DB::ImageDate& range,
                                              bool includeRanges,
                                              const QStringList& images) const
{
    QList<int> idList;
    for (QStringList::const_iterator i = images.begin();
         i != images.end(); ++i) {
        idList << _qh.mediaItemId(Utilities::stripImageDirectory(*i));
    }
    return Settings::SettingsData::instance()->imageDirectory() +
        _qh.findFirstFileInTimeRange(range, includeRanges, idList);
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

QStringList SQLDB::Database::CONVERT( const DB::ResultPtr& )
{
    // PENDING(blackie) IMPLEMENT
    // QWERTY
    qFatal("Oppps better implement me!");
}

DB::ImageInfoPtr SQLDB::Database::info( const DB::ResultId& )
{
    // PENDING(blackie) implement //QWERTY
    return DB::ImageInfoPtr( new DB::ImageInfo() );
}

bool SQLDB::Database::stack(const QStringList& files)
{
    const QStringList relFiles = stripImageDirectoryFromList(files);
    try {
        int newStackId = _qh.stackFiles(relFiles);
        Q_FOREACH(QString file, relFiles)
            _infoCollection.getImageInfoOf(file)->setStackId(newStackId);
        return true;
    }
    catch (SQLDB::Error&)
    {
        return false;
    }
}

void SQLDB::Database::unstack(const QStringList& files)
{
    _qh.unstackFiles(stripImageDirectoryFromList(files));
}

QStringList SQLDB::Database::getStackFor(const QString& referenceFile) const
{
    return _qh.getStackOfFile(Utilities::stripImageDirectory(referenceFile));
}

#include "Database.moc"
