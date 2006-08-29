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
    _handler(address.connectionData())
{
    _handler.openDatabase(address.databaseName());
    _connection = _handler.connection();
    _qh = new QueryHelper(*_connection);
    _categoryCollection = new SQLCategoryCollection(*_connection);
    _members = new DB::MemberMap();
    _infoCollection = new SQLImageInfoCollection(*_connection);
    _md5map = new SQLMD5Map(*_connection);

    connect(categoryCollection(),
            SIGNAL(itemRemoved(DB::Category*, const QString&)),
            _infoCollection,
            SLOT(deleteTag(DB::Category*, const QString&)));
    connect(categoryCollection(),
            SIGNAL(itemRenamed(DB::Category*, const QString&, const QString&)),
            _infoCollection,
            SLOT(renameTag(DB::Category*, const QString&, const QString&)));
}

SQLDB::Database::~Database()
{
    delete _md5map;
    delete _infoCollection;
    delete _members;
    delete _categoryCollection;
    delete _qh;
    _connection->disconnect();
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
    return _qh->mediaItemCount();
}

uint SQLDB::Database::totalCount(DB::MediaType typemask) const
{
    return _qh->mediaItemCount(typemask);
}

DB::MediaCount SQLDB::Database::count(const DB::ImageSearchInfo& searchInfo)
{
    QValueList<int> mediaIds;
    QValueList<int>* scope = 0;
    bool all = (searchInfo.query().count() == 0);
    if (!all) {
        mediaIds = _qh->searchMediaItems(searchInfo);
        scope = &mediaIds;
    }

    return DB::MediaCount(_qh->mediaItemCount(DB::Image, scope),
                          _qh->mediaItemCount(DB::Video, scope));
}

QStringList SQLDB::Database::search( const DB::ImageSearchInfo& info, bool requireOnDisk ) const
{
    QValueList<int> matches = _qh->searchMediaItems(info);
    QStringList result;
    QString imageRoot = Settings::SettingsData::instance()->imageDirectory();
    for( QValueList<int>::Iterator it = matches.begin(); it != matches.end(); ++it ) {
        QString fullPath = imageRoot + _infoCollection->filenameForId(*it);
        if (requireOnDisk && !DB::ImageInfo::imageOnDisk(fullPath))
            continue;
        result.append(fullPath);
    }
    return result;
}

void SQLDB::Database::renameCategory( const QString& oldName, const QString newName )
{
    // TODO: this
}

QMap<QString, uint> SQLDB::Database::classify(const DB::ImageSearchInfo& info,
                                              const QString& category,
                                              DB::MediaType typemask)
{
    QValueList<int>* scope;
    QValueList<int> includedFiles;
    if (info.isNull())
        scope = 0;
    else {
        includedFiles = _qh->searchMediaItems(info, typemask);
        scope = &includedFiles;
    }

    return _qh->classify(category, typemask, scope);
}

QStringList SQLDB::Database::imageList( bool withRelativePath )
{
    QStringList relativePaths = _qh->filenames();
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
        _qh->insertMediaItemsLast(images);
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
    _qh->addBlockItems(relativePaths);
    deleteList(list);
}

bool SQLDB::Database::isBlocking(const QString& fileName)
{
    return _qh->isBlocked(fileName);
}

void SQLDB::Database::deleteList( const QStringList& list )
{
    if (!list.isEmpty()) {
        for (QStringList::const_iterator i = list.begin();
             i != list.end(); ++i) {
            _qh->removeMediaItem(Utilities::stripImageDirectory(*i));
        }
        emit totalChanged(totalCount());
    }
}

DB::ImageInfoPtr SQLDB::Database::info( const QString& fileName ) const
{
    return _infoCollection->
        getImageInfoOf(Utilities::stripImageDirectory(fileName));
}

const DB::MemberMap& SQLDB::Database::memberMap()
{
    return *_members;
}

void SQLDB::Database::setMemberMap( const DB::MemberMap& map )
{
    *_members = map;
}

void SQLDB::Database::save( const QString& /*fileName*/, bool /*isAutoSave*/ )
{
    qDebug("NYI: void SQLDB::Database::save( const QString& fileName )" );
    _infoCollection->clearCache();
}

DB::MD5Map* SQLDB::Database::md5Map()
{
    return _md5map;
}

void SQLDB::Database::lockDB( bool /*lock*/, bool /*exclude*/ )
{
    qDebug("NYI: void SQLDB::Database::lockDB( bool lock, bool exclude )" );
}

DB::CategoryCollection* SQLDB::Database::categoryCollection()
{
    return _categoryCollection;
}

KSharedPtr<DB::ImageDateCollection> SQLDB::Database::rangeCollection()
{
    return new SQLImageDateCollection(*_connection
                                      /*, search(Browser::instance()->currentContext(), false)*/);
}

void SQLDB::Database::reorder(const QString& item,
                              const QStringList& selection, bool after)
{
    _qh->moveMediaItems(stripImageDirectoryFromList(selection),
                       Utilities::stripImageDirectory(item), after);
}

void SQLDB::Database::sortAndMergeBackIn(const QStringList& fileList)
{
    _qh->sortMediaItems(stripImageDirectoryFromList(fileList));
}

QString
SQLDB::Database::findFirstItemInRange(const DB::ImageDate& range,
                                      bool includeRanges,
                                      const QValueVector<QString>& images) const
{
    if (images.count() == totalCount()) {
        return Settings::SettingsData::instance()->imageDirectory() +
            _qh->findFirstFileInTimeRange(range, includeRanges);
    }
    else {
        QValueList<int> idList;
        for (QValueVector<QString>::const_iterator i = images.begin();
             i != images.end(); ++i) {
            idList << _qh->mediaItemId(Utilities::stripImageDirectory(*i));
        }
        return Settings::SettingsData::instance()->imageDirectory() +
            _qh->findFirstFileInTimeRange(range, includeRanges, idList);
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
