#include "Database.h"
#include "DB/MemberMap.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageInfo.h"
#include "Utilities/Util.h"
#include "DB/GroupCounter.h"
#include "SQLImageInfoCollection.h"
#include "Browser/BrowserWidget.h"
#include "SQLImageDateCollection.h"
#include "DB/MediaCount.h"
#include "DatabaseHandler.h"
#include "QueryHelper.h"
#include "QueryErrors.h"
#include <qfileinfo.h>

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

SQLDB::Database::Database(const QString& username, const QString& password)
{
    _dbhandler = DatabaseHandler::getMySQLHandler(username, password);
    _dbhandler->openDatabase("kphotoalbum");
    KexiDB::Connection* connection = _dbhandler->connection();
    if (!connection)
        throw Error(/* TODO: message */);
    QueryHelper::setup(*connection);
    loadMemberGroups();

    connect(categoryCollection(),
            SIGNAL(itemRemoved(DB::Category*, const QString&)),
            &_infoCollection,
            SLOT(deleteTag(DB::Category*, const QString&)));
    connect(categoryCollection(),
            SIGNAL(itemRenamed(DB::Category*, const QString&, const QString&)),
            &_infoCollection,
            SLOT(renameTag(DB::Category*, const QString&, const QString&)));
}

SQLDB::Database::~Database()
{
    delete _dbhandler;
}

int SQLDB::Database::totalCount() const
{
    return QueryHelper::instance()->mediaItemCount();
}

int SQLDB::Database::totalCount(int typemask) const
{
    return QueryHelper::instance()->mediaItemCount(typemask);
}

DB::MediaCount SQLDB::Database::count(const DB::ImageSearchInfo& searchInfo)
{
    QValueList<int> mediaIds;
    QValueList<int>* scope = 0;
    bool all = (searchInfo.query().count() == 0);
    if (!all) {
        mediaIds = QueryHelper::instance()->searchMediaItems(searchInfo);
        scope = &mediaIds;
    }

    return DB::MediaCount(QueryHelper::instance()->
                          mediaItemCount(DB::Image, scope),
                          QueryHelper::instance()->
                          mediaItemCount(DB::Video, scope));
}

QStringList SQLDB::Database::search( const DB::ImageSearchInfo& info, bool requireOnDisk ) const
{
    QValueList<int> matches = QueryHelper::instance()->searchMediaItems(info);
    QStringList result;
    QString imageRoot = Settings::SettingsData::instance()->imageDirectory();
    for( QValueList<int>::Iterator it = matches.begin(); it != matches.end(); ++it ) {
        QString fullPath = imageRoot + QueryHelper::instance()->filenameForId(*it);
        if (requireOnDisk && !QFileInfo(fullPath).exists())
            continue;
        result.append(fullPath);
    }
    return result;
}

void SQLDB::Database::renameCategory( const QString& oldName, const QString newName )
{
    // TODO: this
}

QMap<QString,int> SQLDB::Database::classify(const DB::ImageSearchInfo& info,
                                            const QString& category,
                                            int typemask)
{
    bool allFiles = true;
    QValueList<int> includedFiles;
    if ( !info.isNull() ) {
        includedFiles = QueryHelper::instance()->searchMediaItems(info, typemask);
        allFiles = false;
    }

    QMap<QString,int> result;
    DB::GroupCounter counter( category );
    QDict<void> alreadyMatched = info.findAlreadyMatched( category );

    QValueList< QPair<int, QString> > mediaIdTagPairs =
        QueryHelper::instance()->mediaIdTagPairs(category, typemask);

    QMap<int,QStringList> itemMap;
    for (QValueList< QPair<int, QString> >::const_iterator
             i = mediaIdTagPairs.begin(); i != mediaIdTagPairs.end(); ++i) {
        int fileId = (*i).first;
        QString item = (*i).second;
        if (allFiles || includedFiles.contains(fileId))
            itemMap[fileId].append(item);
    }

    // Count images that doesn't contain an item
    if ( allFiles )
        result[DB::ImageDB::NONE()] = totalCount(typemask) - itemMap.count();
    else
        result[DB::ImageDB::NONE()] = includedFiles.count() - itemMap.count();

    for( QMap<int,QStringList>::Iterator mapIt = itemMap.begin(); mapIt != itemMap.end(); ++mapIt ) {
        QStringList list = mapIt.data();
        for( QStringList::Iterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
            if ( !alreadyMatched[ *listIt ] ) { // We do not want to match "Jesper & Jesper"
                result[ *listIt ]++;
            }
        }
        counter.count( list );
    }

    QMap<QString,int> groups = counter.result();
    for( QMapIterator<QString,int> it= groups.begin(); it != groups.end(); ++it ) {
        result[it.key()] = it.data();
    }
    return result;
}

QStringList SQLDB::Database::imageList( bool withRelativePath )
{
    QStringList relativePaths = QueryHelper::instance()->relativeFilenames();
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
        QueryHelper::instance()->insertMediaItemsLast(images);
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
    QueryHelper::instance()->addBlockItems(relativePaths);
    deleteList(list);
}

bool SQLDB::Database::isBlocking(const QString& fileName)
{
    return QueryHelper::instance()->isBlocked(fileName);
}

void SQLDB::Database::deleteList( const QStringList& list )
{
    if (!list.isEmpty()) {
        for (QStringList::const_iterator i = list.begin();
             i != list.end(); ++i) {
            QueryHelper::instance()->
                removeMediaItem(Utilities::stripImageDirectory(*i));
        }
        emit totalChanged(totalCount());
    }
}

DB::ImageInfoPtr SQLDB::Database::info( const QString& fileName ) const
{
    return _infoCollection.
        getImageInfoOf(Utilities::stripImageDirectory(fileName));
}

const DB::MemberMap& SQLDB::Database::memberMap()
{
    return _members;
}

void SQLDB::Database::setMemberMap( const DB::MemberMap& map )
{
    _members = map;
}

void SQLDB::Database::save( const QString& /*fileName*/, bool /*isAutoSave*/ )
{
    qDebug("NYI: void SQLDB::Database::save( const QString& fileName )" );
    _infoCollection.clearCache();
}

DB::MD5Map* SQLDB::Database::md5Map()
{
    return &_md5map;
}

void SQLDB::Database::lockDB( bool /*lock*/, bool /*exclude*/ )
{
    qDebug("NYI: void SQLDB::Database::lockDB( bool lock, bool exclude )" );
}

void SQLDB::Database::loadMemberGroups()
{
    QValueList<QString[3]> l =
        QueryHelper::instance()->memberGroupConfiguration();
    for (QValueList<QString[3]>::const_iterator i = l.begin();
         i != l.end(); ++i)
        _members.addMemberToGroup((*i)[0], (*i)[1], (*i)[2]);
}


DB::CategoryCollection* SQLDB::Database::categoryCollection()
{
    return &_categoryCollection;
}

KSharedPtr<DB::ImageDateCollection> SQLDB::Database::rangeCollection()
{
    return new SQLImageDateCollection( /*search( Browser::instance()->currentContext(), false ) */ );
}

void SQLDB::Database::reorder(const QString& item,
                              const QStringList& selection, bool after)
{
    QueryHelper::instance()->
        moveMediaItems(stripImageDirectoryFromList(selection),
                       Utilities::stripImageDirectory(item), after);
}

void SQLDB::Database::sortAndMergeBackIn(const QStringList& fileList)
{
    QueryHelper::instance()->
        sortMediaItems(stripImageDirectoryFromList(fileList));
}

QString
SQLDB::Database::findFirstItemInRange(const DB::ImageDate& range,
                                      bool includeRanges,
                                      const QValueVector<QString>& images) const
{
    if (images.count() == static_cast<uint>(totalCount())) {
        return Settings::SettingsData::instance()->imageDirectory() +
            QueryHelper::instance()->
            findFirstFileInTimeRange(range, includeRanges);
    }
    else {
        QValueList<int> idList;
        for (QValueVector<QString>::const_iterator i = images.begin();
             i != images.end(); ++i) {
            idList << QueryHelper::instance()->
                idForFilename(Utilities::stripImageDirectory(*i));
        }
        return Settings::SettingsData::instance()->imageDirectory() +
            QueryHelper::instance()->
            findFirstFileInTimeRange(range, includeRanges, idList);
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
