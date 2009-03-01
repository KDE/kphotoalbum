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
#include "DB/ResultId.h"
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
    QList<DB::RawId> mediaIds;
    QList<DB::RawId>* scope = 0;
    bool all = (searchInfo.query().count() == 0);
    if (!all) {
        mediaIds = _qh.searchMediaItems(searchInfo);
        scope = &mediaIds;
    }

    return DB::MediaCount(_qh.mediaItemCount(DB::Image, scope),
                          _qh.mediaItemCount(DB::Video, scope));
}

DB::Result SQLDB::Database::search(
    const DB::ImageSearchInfo& info,
    bool requireOnDisk) const
{
    QList<DB::RawId> matches = _qh.searchMediaItems(info);
    QList<DB::RawId> result;
    QString imageRoot = Settings::SettingsData::instance()->imageDirectory();
    for(QList<DB::RawId>::Iterator it = matches.begin(); it != matches.end(); ++it) {
        QString fullPath = imageRoot + _infoCollection.filenameForId(*it);
        if (requireOnDisk && !DB::ImageInfo::imageOnDisk(fullPath))
            continue;
        result.append(*it);
    }
    return DB::Result(result);
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

DB::Result SQLDB::Database::imageList()
{
    return DB::Result(_qh.mediaItemIds(DB::anyMediaType));
}


DB::Result SQLDB::Database::images()
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

void SQLDB::Database::renameImage( DB::ImageInfoPtr info, const QString& newName )
{
    info->setFileName( newName );
    info->delaySavingChanges(false);
}

// TODO: Rename this function
// This function also removes the files from the database
void SQLDB::Database::addToBlockList(const DB::Result& list)
{
    const QStringList absolutePaths = CONVERT(list);
    QStringList relativePaths;
    Q_FOREACH(QString absPath, absolutePaths)
        relativePaths.push_back(Utilities::stripImageDirectory(absPath));
    _qh.addIgnoredFiles(relativePaths);
    deleteList(list);
}

bool SQLDB::Database::isBlocking(const QString& fileName)
{
    return _qh.isIgnored(fileName);
}

void SQLDB::Database::deleteList(const DB::Result& filesToRemove)
{
#ifdef HAVE_EXIV2
    Q_FOREACH(QString fileName, this->CONVERT(filesToRemove))
        Exif::Database::instance()->remove(fileName);
#endif
    Q_FOREACH(const DB::ResultId id, filesToRemove)
        _qh.removeMediaItem(id.rawId());
    if (!filesToRemove.isEmpty()) {
        emit totalChanged(totalCount());
        emit imagesDeleted( filesToRemove );
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

void SQLDB::Database::reorder(
    const DB::ResultId& file,
    const DB::Result& selection,
    bool after)
{
    Q_ASSERT(!file.isNull());
    _qh.moveMediaItems(selection.rawIdList(), file.rawId(), after);
}

void SQLDB::Database::sortAndMergeBackIn(const DB::Result& files)
{
    _qh.sortFiles(files.rawIdList());
}

DB::ResultId SQLDB::Database::findFirstItemInRange(
    const DB::Result& files,
    const DB::ImageDate& range,
    bool includeRanges) const
{
    return _qh.findFirstFileInTimeRange(
        range, includeRanges, files.rawIdList());
}

QStringList SQLDB::Database::CONVERT(const DB::Result& result)
{
    QStringList files;
    Q_FOREACH(DB::ResultId id, result) {
        files.push_back(
            Utilities::imageFileNameToAbsolute(
                _qh.mediaItemFilename(id.rawId())));
        Q_ASSERT(!files[files.size() - 1].isNull());
    }
    return files;
}

DB::ResultId SQLDB::Database::ID_FOR_FILE(const QString& filename) const
{
    return DB::ResultId::createContextless(_qh.mediaItemId(Utilities::imageFileNameToRelative(filename)));
}

DB::ImageInfoPtr SQLDB::Database::info(const DB::ResultId& id) const
{
    Q_ASSERT(!id.isNull());
    return _infoCollection.getImageInfoOf(id);
}

bool SQLDB::Database::stack(const DB::Result& files)
{
    try {
        const DB::StackID newStackId = _qh.stackFiles(files.rawIdList());
        Q_FOREACH(DB::ImageInfoPtr info, files.fetchInfos()) {
            Q_ASSERT(!info.isNull());
            info->setStackId(newStackId);
        }
        return true;
    }
    catch (SQLDB::OperationNotPossible&) {
        return false;
    }
}

void SQLDB::Database::unstack(const DB::Result& files)
{
    _qh.unstackFiles(files.rawIdList());
    Q_FOREACH(DB::ImageInfoPtr info, files.fetchInfos()) {
        Q_ASSERT(!info.isNull());
        info->setStackId(DB::StackID());
    }
}

DB::Result SQLDB::Database::getStackFor(const DB::ResultId& referenceFile) const
{
    Q_ASSERT(!referenceFile.isNull());
    if (!referenceFile.isNull())
        return DB::Result(_qh.getStackOfFile(referenceFile.rawId()));
    else
        return DB::Result();
}

#include "Database.moc"
