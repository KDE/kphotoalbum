#include "Database.h"
#include "DB/MemberMap.h"
#include "DB/CategoryCollection.h"
#include "Query.h"
#include "DB/ImageInfo.h"
#include "Utilities/Util.h"
#include "DB/GroupCounter.h"
#include "SQLImageInfo.h"
#include "Browser/BrowserWidget.h"
#include "SQLImageDateCollection.h"
#include "DB/MediaCount.h"
#include "DatabaseHandler.h"
#include "QueryHelper.h"
#include "QueryErrors.h"
#include <qfileinfo.h>

SQLDB::Database::Database( const QString& username, const QString& password ) :_members( this )
{
    _dbhandler = DatabaseHandler::getMySQLHandler(username, password);
    _dbhandler->openDatabase("kphotoalbum");
    KexiDB::Connection* connection = _dbhandler->connection();
    if (!connection)
        throw Error(/* TODO: message */);
    QueryHelper::setup(*connection);
    loadMemberGroups();
}

SQLDB::Database::~Database()
{
    delete _dbhandler;
}

int SQLDB::Database::totalCount() const
{
    return QueryHelper::instance()->
        executeQuery("SELECT COUNT(*) FROM media").firstItem().toInt();
}

int SQLDB::Database::totalCount(int typemask) const
{
    if (typemask == DB::anyMediaType)
        return totalCount();

    return QueryHelper::instance()->
        executeQuery("SELECT COUNT(*) FROM media WHERE type&%s!=0",
                     QueryHelper::Bindings() << typemask).firstItem().toInt();
}

DB::MediaCount SQLDB::Database::count(const DB::ImageSearchInfo& searchInfo)
{
    QueryHelper::Bindings bindings;
    QString rangeCond = "";
    bool all = (searchInfo.query().count() == 0);
    if (!all) {
        QValueList<int> mediaIds = searchMediaItems(searchInfo);
        if (mediaIds.count() > 0) {
            rangeCond = " AND id IN (%s)";
            bindings << toVariantList(mediaIds);
        }
    }

    DB::MediaType types[] = {DB::Image, DB::Video};
    int count[2];
    for (size_t i = 0; i < 2; ++i) {
        count[i] = QueryHelper::instance()->
            executeQuery("SELECT COUNT(*) FROM media "
                         "WHERE type=%s" + rangeCond,
                         (QueryHelper::Bindings() << types[i]) + bindings).
            firstItem().asInt();
    }
    return DB::MediaCount(count[0], count[1]);
}

QStringList SQLDB::Database::search( const DB::ImageSearchInfo& info, bool requireOnDisk ) const
{
    QValueList<int> matches = searchMediaItems(info);
    QStringList result;
    for( QValueList<int>::Iterator it = matches.begin(); it != matches.end(); ++it ) {
        QString fullPath = QueryHelper::instance()->filenameForId(*it, true);
        if (requireOnDisk && !QFileInfo(fullPath).exists())
            continue;
        result.append(fullPath);
    }
    return result;
}

void SQLDB::Database::renameCategory( const QString& oldName, const QString newName )
{
    QueryHelper::instance()->
        executeStatement("UPDATE category SET name=%s WHERE name=%s",
                         QueryHelper::Bindings() << newName << oldName);
}

QMap<QString,int> SQLDB::Database::classify(const DB::ImageSearchInfo& info,
                                            const QString& category,
                                            int typemask)
{
    bool allFiles = true;
    QValueList<int> includedFiles;
    if ( !info.isNull() ) {
        includedFiles = searchMediaItems(info, typemask);
        allFiles = false;
    }

    QMap<QString,int> result;
    DB::GroupCounter counter( category );
    QDict<void> alreadyMatched = info.findAlreadyMatched( category );

    QValueList< QPair<int, QString> > mediaIdTagPairs =
        QueryHelper::instance()->getMediaIdTagPairs(category, typemask);

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
    for(DB::ImageInfoListConstIterator it = images.constBegin();
        it != images.constEnd(); ++it ) {
        DB::ImageInfoPtr info = *it;

        QueryHelper::instance()->insertMediaItem(*info);
    }

    emit totalChanged( totalCount() );
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
    for (QStringList::const_iterator i = list.begin(); i != list.end(); ++i)
        QueryHelper::instance()->
            removeMediaItem(Utilities::stripImageDirectory(*i));
    if (list.count() != 0)
        emit totalChanged(totalCount());
}

DB::ImageInfoPtr SQLDB::Database::info( const QString& fileName ) const
{
    return SQLImageInfo::
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
    SQLImageInfo::clearCache();
}

DB::MD5Map* SQLDB::Database::md5Map()
{
    return &_md5map;
}

void SQLDB::Database::sortAndMergeBackIn( const QStringList& /*fileList*/ )
{
    qDebug("NYI: void SQLDB::Database::sortAndMergeBackIn( const QStringList& fileList )" );
}

void SQLDB::Database::renameItem( DB::Category* category, const QString& oldName, const QString& newName )
{
    QueryHelper::instance()->
        executeStatement("UPDATE tag SET name=%s "
                         "WHERE name=%s AND "
                         "categoryId=(SELECT id FROM category WHERE name=%s)",
                         QueryHelper::Bindings() <<
                         newName << oldName << category->name());
}

void SQLDB::Database::deleteItem(DB::Category* category, const QString& option)
{
    if (category)
        category->removeItem(option);
}

void SQLDB::Database::lockDB( bool /*lock*/, bool /*exclude*/ )
{
    qDebug("NYI: void SQLDB::Database::lockDB( bool lock, bool exclude )" );
}

void SQLDB::Database::loadMemberGroups()
{
    QValueList<QString[3]> l = QueryHelper::instance()->
        executeQuery("SELECT c.name, t.name, f.name "
                     "FROM tag_relation tr, tag t, tag f, category c "
                     "WHERE tr.toTagId=t.id AND "
                     "tr.fromTagId=f.id AND "
                     "t.categoryId=c.id").asString3List();
    for (QValueList<QString[3]>::const_iterator i = l.begin();
         i != l.end(); ++i)
        _members.addMemberToGroup((*i)[0], (*i)[1], (*i)[2]);
}


DB::CategoryCollection* SQLDB::Database::categoryCollection()
{
    // PENDING(blackie) Implement something similar to XMLDB::createSpecialCategories()
    return &_categoryCollection;
}

KSharedPtr<DB::ImageDateCollection> SQLDB::Database::rangeCollection()
{
    return new SQLImageDateCollection( /*search( Browser::instance()->currentContext(), false ) */ );
}

void SQLDB::Database::reorder( const QString& /*item*/, const QStringList& /*cutList*/, bool /*after*/)
{
    qDebug("Not Yet implemented SQLDB::Database::reorder");
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
