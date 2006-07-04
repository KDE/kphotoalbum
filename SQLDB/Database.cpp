#include "Database.h"
#include "DB/MemberMap.h"
#include <qsqldatabase.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <stdlib.h>
#include "DB/CategoryCollection.h"
#include <qsqlerror.h>
#include "Query.h"
#include "QueryUtil.h"
#include "DB/ImageInfo.h"
#include "Utilities/Util.h"
#include "DB/GroupCounter.h"
#include <kdebug.h>
#include "SQLImageInfo.h"
#include "Browser/BrowserWidget.h"
#include "SQLImageDateCollection.h"

#ifdef HASKEXIDB
#include "DB/MediaCount.h"
#include "QueryHelper.h"

#include <kexidb/driver.h>
#include <kexidb/connection.h>
#include <kexidb/drivermanager.h> // KexiDB::DriverManager
#include <kexidb/connectiondata.h> // KexiDB::ConnectionData
#include <kexidb/tableschema.h> // KexiDB::TableSchema
#include <kexidb/indexschema.h> // KexiDB::IndexSchema
#include <kexidb/transaction.h> // KexiDB::Transaction
#include <kexidb/field.h> // KexiDB::Field
#include <kexidb/cursor.h>
#include <qvaluevector.h> // QValueVector, used by kexi enum

#include <kexidb/parser.h>
#include <kexidb/queryschema.h>

#include <qfileinfo.h>

using KexiDB::Field;
#endif

//TODO: error handling
#include <stdlib.h>
namespace
{
    void tellError(const QString& msg)
    {
        KMessageBox::sorry(0, msg);
    }
    void exitError(const QString& msg)
    {
        tellError(msg);
        exit(-1);
    }
}


SQLDB::Database::Database( const QString& username, const QString& password ) :_members( this )
{
#ifndef HASKEXIDB
    if ( !QSqlDatabase::isDriverAvailable( QString::fromLatin1("QMYSQL3") ) ) {
        // PENDING(blackie) better message
        KMessageBox::sorry( 0, i18n("The MySQL driver did not seem to be compiled into your Qt" ) );
        exit(-1);
    }
#else
    KexiDB::DriverManager *manager = new KexiDB::DriverManager();
    _driver = manager->driver("MySQL");
    if (!_driver) {
        // TODO: error handling (get from manager)
        exitError(i18n("Kexi driver not found."));
    }
#endif
    if (!openConnection(username, password)) {
        exit(-1);
    }
    openDatabase();
#ifdef HASKEXIDB
    QueryHelper::setup(_connection);
#endif
    loadMemberGroups();
}

void SQLDB::Database::createAndOpen()
{
#ifdef HASKEXIDB
    qDebug("Creating db kphotoalbum");
    if (!_connection->createDatabase("kphotoalbum")) {
        qDebug("create failed: %s", _connection->errorMsg().latin1());
        exit(-1);
    }
    else qDebug("create succeed");
    if (!_connection->useDatabase("kphotoalbum")) {
        exitError("Cannot use db kphotoalbum");
    }
    // TODO: use transaction
//     bool useTransactions = driver->transactionsSupported();
//     KexiDB::Transaction t;
//     if (useTransactions) {
//         _connection->setAutoCommit(false);
//         t = _connection->beginTransaction();
//         if (_connection->error()) {
//             qDebug("transaction failed: %s",
//                    _connection->errorMsg().latin1());
//         }
//     }

    KexiDB::Field* f;
    KexiDB::TableSchema* schema;

    //TODO: Set NotNull flags where should
    //TODO: error handling

    // ==== dir table ====
    schema = new KexiDB::TableSchema("dir");
    schema->setCaption("directories");

    f = new KexiDB::Field("id", Field::Integer,
                          Field::PrimaryKey | Field::AutoInc,
                          Field::Unsigned);
    f->setCaption("id");
    schema->addField(f);

    f = new KexiDB::Field("path", Field::Text,
                          Field::NotNull /* | Field::Unique */,
                          Field::NoOptions, 511);
    f->setCaption("path");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        qDebug("creating dir table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


    // ==== media table ====
    schema = new KexiDB::TableSchema("media");
    schema->setCaption("media items");

    f = new KexiDB::Field("id", Field::BigInteger,
                          Field::PrimaryKey | Field::AutoInc,
                          Field::Unsigned);
    f->setCaption("id");
    schema->addField(f);

    f = new KexiDB::Field("place", Field::BigInteger,
                          Field::Indexed, Field::Unsigned);
    f->setCaption("place");
    schema->addField(f);

    f = new KexiDB::Field("dirId", Field::Integer,
                          Field::ForeignKey | Field::NotNull,
                          Field::Unsigned);
    f->setCaption("directory id");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (dirId) REFERENCES dir(id)
    // ON DELETE RESTRICT ON UPDATE RESTRICT

    f = new KexiDB::Field("filename", KexiDB::Field::Text,
                          Field::NotNull, Field::NoOptions, 255);
    f->setCaption("filename");
    schema->addField(f);

    // TODO: UNIQUE(dirId, filename)

    f = new KexiDB::Field("md5sum", KexiDB::Field::Text,
                          Field::NoConstraints, Field::NoOptions, 32);
    f->setCaption("md5sum");
    schema->addField(f);

    f = new KexiDB::Field("type", KexiDB::Field::ShortInteger,
                          Field::NotNull, Field::Unsigned);
    f->setCaption("type");
    schema->addField(f);

    f = new KexiDB::Field("label", KexiDB::Field::Text,
                          Field::NoConstraints, Field::NoOptions, 255);
    f->setCaption("label");
    schema->addField(f);

    f = new KexiDB::Field("description", KexiDB::Field::LongText);
    f->setCaption("description");
    schema->addField(f);

    f = new KexiDB::Field("startTime", KexiDB::Field::DateTime);
    f->setCaption("start time");
    schema->addField(f);

    f = new KexiDB::Field("endTime", KexiDB::Field::DateTime);
    f->setCaption("end time");
    schema->addField(f);

    f = new KexiDB::Field("width", KexiDB::Field::Integer,
                          Field::NoConstraints,
                          Field::Unsigned);
    f->setCaption("type");
    schema->addField(f);

    f = new KexiDB::Field("height", KexiDB::Field::Integer,
                          Field::NoConstraints,
                          Field::Unsigned);
    f->setCaption("type");
    schema->addField(f);

    f = new KexiDB::Field("angle", KexiDB::Field::ShortInteger);
    f->setCaption("type");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        qDebug("creating media table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


    // ==== blockitem table ====
    schema = new KexiDB::TableSchema("blockitem");
    schema->setCaption("block items");

    f = new KexiDB::Field("dirId", Field::Integer,
                          Field::ForeignKey | Field::NotNull,
                          Field::Unsigned);
    f->setCaption("directory id");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (dirId) REFERENCES dir(id)
    // ON DELETE RESTRICT ON UPDATE RESTRICT

    f = new KexiDB::Field("filename", KexiDB::Field::Text,
                          Field::NotNull, Field::NoOptions, 255);
    f->setCaption("filename");
    schema->addField(f);

    // TODO: UNIQUE(dirId, filename)

    if (!_connection->createTable(schema)) {
        qDebug("creating blockitem table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


    // ==== category table ====
    schema = new KexiDB::TableSchema("category");
    schema->setCaption("categories");

    f = new KexiDB::Field("id", Field::Integer,
                          Field::PrimaryKey | Field::AutoInc,
                          Field::Unsigned);
    f->setCaption("id");
    schema->addField(f);

    f = new KexiDB::Field("name", Field::Text,
                          Field::NotNull | Field::Unique,
                          Field::NoOptions, 255);
    f->setCaption("name");
    schema->addField(f);

    f = new KexiDB::Field("icon", Field::Text,
                          Field::NoConstraints, Field::NoOptions, 1023);
    f->setCaption("name");
    schema->addField(f);

    f = new KexiDB::Field("visible", KexiDB::Field::Boolean);
    f->setCaption("is visible");
    schema->addField(f);

    f = new KexiDB::Field("viewtype", KexiDB::Field::ShortInteger);
    f->setCaption("view type");
    schema->addField(f);

    f = new KexiDB::Field("viewsize", KexiDB::Field::ShortInteger);
    f->setCaption("view size");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        qDebug("creating category table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


    // ==== tag table ====
    schema = new KexiDB::TableSchema("tag");
    schema->setCaption("tags");

    f = new KexiDB::Field("id", Field::BigInteger,
                          Field::PrimaryKey | Field::AutoInc,
                          Field::Unsigned);
    f->setCaption("id");
    schema->addField(f);

    f = new KexiDB::Field("place", Field::BigInteger,
                          Field::Indexed, Field::Unsigned);
    f->setCaption("place");
    schema->addField(f);

    f = new KexiDB::Field("categoryId", Field::Integer,
                          Field::ForeignKey | Field::NotNull,
                          Field::Unsigned);
    f->setCaption("category id");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (categoryId) REFERENCES category(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    f = new KexiDB::Field("name", Field::Text,
                          Field::NoConstraints, Field::NoOptions, 255);
    f->setCaption("name");
    schema->addField(f);

    // TODO: UNIQUE(categoryId, name)

    f = new KexiDB::Field("isGroup", KexiDB::Field::Boolean,
                          Field::NotNull);
    f->setCaption("is member group");
    f->setDefaultValue(QVariant(0));
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        qDebug("creating tag table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


    // ==== media_tag table ====
    schema = new KexiDB::TableSchema("media_tag");
    schema->setCaption("media item tags");
    // TODO: create index
    //KexiDB::IndexSchema *indexSchema = new KexiDB::IndexSchema(schema);

    f = new Field("mediaId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("media item id");
    schema->addField(f);
    //indexSchema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (mediaId) REFERENCES media(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT
    // FOREIGN KEY (tagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    f = new Field("tagId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("tag id");
    schema->addField(f);
    //indexSchema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (tagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    // TODO: UNIQUE(mediaId, tagId)
    //schema->addIndex(indexSchema);
    //schema->setPrimaryKey(indexSchema);

    if (!_connection->createTable(schema)) {
        qDebug("creating media_tag table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


    // ==== tag_relation table ====
    schema = new KexiDB::TableSchema("tag_relation");
    schema->setCaption("tag relations");
    // TODO: create index
    //indexSchema = new KexiDB::IndexSchema(schema);

    f = new Field("toTagId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("media item id");
    schema->addField(f);
    //indexSchema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (toTagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    f = new Field("fromTagId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("tag id");
    schema->addField(f);
    //indexSchema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (fromTagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    // TODO: UNIQUE(toTagId, fromTagId)
    //schema->addIndex(indexSchema);
    //schema->setPrimaryKey(indexSchema);

    if (!_connection->createTable(schema)) {
        qDebug("creating tag_relation table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }

//     if (useTransactions) {
//         _connection->setAutoCommit(false);
//         if (!_connection->commitTransaction(t)) {
//             qDebug("transaction commit failed: %s",
//                    _connection->errorMsg().latin1());
//         }
//     }
#endif
}


int SQLDB::Database::totalCount() const
{
#ifndef HASKEXIDB
    QString query = QString::fromLatin1( "SELECT COUNT(fileId) FROM sortorder" );
    return fetchItem( query ).toInt();
#else
    return QueryHelper::instance()->
        executeQuery("SELECT COUNT(*) FROM media").firstItem().toInt();
#endif
}

int SQLDB::Database::totalCount(int type) const
{
    return QueryHelper::instance()->
        executeQuery("SELECT COUNT(*) FROM media WHERE type=%s",
                     QueryHelper::Bindings() << type).firstItem().toInt();
}

DB::MediaCount SQLDB::Database::count(const DB::ImageSearchInfo& searchInfo)
{
    QueryHelper::Bindings bindings;
    QString rangeCond = "";
    bool all = (searchInfo.query().count() == 0);
    if (!all) {
        QValueList<int> mediaIds = filesMatchingQuery(searchInfo);
        if (mediaIds.count() > 0) {
            rangeCond = " AND id IN (%s)";
            bindings << toVariantList(mediaIds);
        }
    }

    DB::MediaType types[] = {DB::Image, DB::Movie};
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
    // PENDING(blackie) Handle on disk.
    QValueList<int> matches = filesMatchingQuery( info );
    QStringList result;
    for( QValueList<int>::Iterator it = matches.begin(); it != matches.end(); ++it ) {
#ifndef HASKEXIDB
        result.append( fileNameForId( *it, true ) );
#else
        QString fullPath = QueryHelper::instance()->filenameForId(*it, true);
        if (requireOnDisk && !QFileInfo(fullPath).exists())
            continue;
        result.append(fullPath);
#endif
    }
    return result;
}

void SQLDB::Database::renameCategory( const QString& oldName, const QString newName )
{
#ifndef HASKEXIDB
    QSqlQuery query;
    query.prepare( "UPDATE imagecategoryinfo SET category=:newName WHERE category=:oldName" );
    query.bindValue( QString::fromLatin1( ":newName" ), newName );
    query.bindValue( QString::fromLatin1( ":oldName" ), oldName );
    if ( !query.exec() )
        showError( query );
#else
    QueryHelper::instance()->
        executeStatement("UPDATE category SET name=%s WHERE name=%s",
                         QueryHelper::Bindings() << newName << oldName);
#endif
}

QMap<QString,int> SQLDB::Database::classify(const DB::ImageSearchInfo& info,
                                            const QString& category,
                                            int type)
{
    bool allFiles = true;
    QValueList<int> includedFiles;
    if ( !info.isNull() ) {
        includedFiles = searchFilesOfType(static_cast<DB::MediaType>(type),
                                          info);
        allFiles = false;
    }

    QMap<QString,int> result;
    DB::GroupCounter counter( category );
    QDict<void> alreadyMatched = info.findAlreadyMatched( category );

#ifndef HASKEXIDB
    QSqlQuery query;
    query.prepare( "SELECT fileId, value from imagecategoryinfo WHERE categoryId=:categoryId" );
    query.bindValue( QString::fromLatin1( ":categoryId" ), idForCategory(category) );
    if ( !query.exec() )
        showError( query );

    QMap<int,QStringList> itemMap;
    while ( query.next() ) {
        int fileId = query.value(0).toInt();
        QString item = query.value(1).toString();
        if ( allFiles || includedFiles.contains( fileId ) )
            itemMap[fileId].append( item );
    }
#else
    KexiDB::Cursor* c;
    if (category == "Folder")
        c = QueryHelper::instance()->
            executeQuery("SELECT media.id, dir.path FROM media, dir "
                         "WHERE media.dirId=dir.id AND media.type=%s",
                         QueryHelper::Bindings() << type).cursor();
    else
        c = QueryHelper::instance()->
            executeQuery("SELECT media.id, tag.name "
                         "FROM media, media_tag, tag, category "
                         "WHERE media.id=media_tag.mediaId AND "
                         "media_tag.tagId=tag.id AND "
                         "tag.categoryId=category.id AND "
                         "media.type=%s AND category.name=%s",
                         QueryHelper::Bindings() << type << category).cursor();
    if (!c) {
        // TODO: error handling
        Q_ASSERT(false);
        return result;
    }

    QMap<int,QStringList> itemMap;
    for (c->moveFirst(); !c->eof(); c->moveNext()) {
        int fileId = c->value(0).toInt();
        QString item = c->value(1).toString();
        if (allFiles || includedFiles.contains(fileId))
            itemMap[fileId].append(item);
    }

    _connection->deleteCursor(c);
#endif

    // Count images that doesn't contain an item
    if ( allFiles )
        result[DB::ImageDB::NONE()] = totalCount(type) - itemMap.count();
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

DB::ImageInfoList& SQLDB::Database::imageInfoList()
{
    qDebug("NYI: ImageInfoList& SQLDB::Database::imageInfoList()" );
    static DB::ImageInfoList list;
    return list;
}

QStringList SQLDB::Database::imageList( bool withRelativePath )
{
#ifndef HASKEXIDB
    QSqlQuery query;
    if ( !query.exec( QString::fromLatin1( "SELECT fileId FROM sortorder" ) ) )
        showError( query );
    QStringList result;
    while ( query.next() )
        result << fileNameForId( query.value(0).toInt(), withRelativePath );
    return result;
#else
    QueryHelper* qh = QueryHelper::instance();
    QValueList<int> idList =
        qh->executeQuery("SELECT id FROM media "
                         "ORDER BY place").asIntegerList();
    QStringList r;
    for (QValueList<int>::const_iterator i = idList.begin();
         i != idList.end(); ++i) {
        r.append(qh->filenameForId(*i, !withRelativePath));
    }
    return r;
#endif
}


QStringList SQLDB::Database::images()
{
    return imageList( false );
}

void SQLDB::Database::addImages( const DB::ImageInfoList& images )
{
#ifndef HASKEXIDB
    int idx = totalCount();

    QString imageQueryString = QString::fromLatin1( "INSERT INTO imageinfo set "
                                                    "width = :width, height = :height, md5sum = :md5sum, fileId = :fileId, label = :label, "
                                                    "angle = :angle, description = :description, startDate = :startDate, "
                                                    "endDate = :endDate " );
    QSqlQuery imageQuery;
    imageQuery.prepare( imageQueryString );

    QString categoryQueryString = QString::fromLatin1( "insert INTO imagecategoryinfo set fileId = :fileId, "
                                                       "categoryId = :categoryId, value = :value" );
    QSqlQuery categoryQuery;
    categoryQuery.prepare( categoryQueryString );

    QString sortOrderQueryString = QString::fromLatin1( "INSERT INTO sortorder SET idx=:idx, fileId=:fileId, fileName=:fileName" );
    QSqlQuery sortOrderQuery;
    sortOrderQuery.prepare( sortOrderQueryString );

    int nextId = fetchItem( QString::fromLatin1( "SELECT MAX(fileId) FROM sortorder" ) ).toInt();
    for( DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        DB::ImageInfoPtr info = *it;
        ++nextId;

        imageQuery.bindValue( QString::fromLatin1( ":width" ),  info->size().width() );
        imageQuery.bindValue( QString::fromLatin1( ":height" ),  info->size().height() );
        imageQuery.bindValue( QString::fromLatin1( ":md5sum" ), info->MD5Sum() );
        imageQuery.bindValue( QString::fromLatin1( ":fileId" ),  nextId );
        imageQuery.bindValue( QString::fromLatin1( ":label" ),  info->label() );
        imageQuery.bindValue( QString::fromLatin1( ":angle" ),  info->angle() );
        imageQuery.bindValue( QString::fromLatin1( ":description" ),  info->description() );
        imageQuery.bindValue( QString::fromLatin1( ":startDate" ), info->date().start() );
        imageQuery.bindValue( QString::fromLatin1( ":endDate" ), info->date().end() );

        if ( !imageQuery.exec() )
            showError( imageQuery );

        sortOrderQuery.bindValue( QString::fromLatin1( ":idx" ), idx++ );
        sortOrderQuery.bindValue( QString::fromLatin1( ":fileId" ), nextId );
        sortOrderQuery.bindValue( QString::fromLatin1( ":fileName" ), info->fileName( true ) );
        if ( !sortOrderQuery.exec() )
            showError( sortOrderQuery );


        // Category info

        QStringList categories = info->availableCategories();
        categoryQuery.bindValue( QString::fromLatin1( ":fileId" ), nextId );
        for( QStringList::ConstIterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
            QStringList items = info->itemsOfCategory( *categoryIt );
            categoryQuery.bindValue( QString::fromLatin1( ":categoryId" ), idForCategory(*categoryIt) );
            for( QStringList::ConstIterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
                categoryQuery.bindValue( QString::fromLatin1( ":value" ), *itemIt );
                if ( !categoryQuery.exec() )
                    showError( categoryQuery );
            }
        }
    }
#else
    for(DB::ImageInfoListConstIterator it = images.constBegin();
        it != images.constEnd(); ++it ) {
        DB::ImageInfoPtr info = *it;

        QueryHelper::instance()->insertMediaItem(*info);
    }
#endif

    emit totalChanged( totalCount() );
}

void SQLDB::Database::addToBlockList(const QStringList& list)
{
    QueryHelper::instance()->addBlockItems(list);
    deleteList(list);
}

bool SQLDB::Database::isBlocking(const QString& fileName)
{
    return QueryHelper::instance()->isBlocked(fileName);
}

void SQLDB::Database::deleteList( const QStringList& list )
{
#ifndef HASKEXIDB
    QStringList sortOrder = imageList( true );
    QSqlQuery imageInfoQuery, imageCategoryQuery;
    imageInfoQuery.prepare( "DELETE FROM imageinfo where fileId=:fileId" );
    imageCategoryQuery.prepare( "DELETE FROM imagecategoryinfo where fileId=:fileId" );

    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        QString fileName = Utilities::stripImageDirectory( *it );
        imageInfoQuery.bindValue( QString::fromLatin1( ":fileId" ), idForFileName( fileName ) );
        imageCategoryQuery.bindValue( QString::fromLatin1( ":fileId" ), idForFileName( fileName ) );

        if ( !imageInfoQuery.exec() )
            showError( imageInfoQuery );

        if ( !imageCategoryQuery.exec() )
            showError( imageCategoryQuery );

        sortOrder.remove( fileName );
    }

    QSqlQuery query;
    if ( !query.exec( QString::fromLatin1( "DELETE FROM sortorder" ) ) )
        showError( query );

    // PENDING(blackie) We need to remember the original id's in a map for this.
    int idx = 0;
    query.prepare( QString::fromLatin1( "INSERT INTO sortorder SET idx=:idx, fileName=:fileName" ) );
    for( QStringList::Iterator it = sortOrder.begin(); it != sortOrder.end(); ++it ) {
        query.bindValue( QString::fromLatin1( ":idx" ), idx++ );
        query.bindValue( QString::fromLatin1( ":fileName" ), *it );
        if ( !query.exec() )
            showError( query );
    }
#else
    for (QStringList::const_iterator i = list.begin(); i != list.end(); ++i)
        QueryHelper::instance()->
            removeMediaItem(Utilities::stripImageDirectory(*i));
#endif
    if (list.count() != 0)
        emit totalChanged(totalCount());
}

DB::ImageInfoPtr SQLDB::Database::info( const QString& fileName ) const
{
    return new SQLImageInfo( fileName );
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
#ifndef HASKEXIDB
    QSqlQuery query;
    query.prepare( QString::fromLatin1( "UPDATE imagecategoryinfo SET value = :newName "
                                        "WHERE category = :category and value = :oldName" ));
    query.bindValue( QString::fromLatin1( ":newName" ), newName );
    query.bindValue( QString::fromLatin1( ":category" ), category->name() );
    query.bindValue( QString::fromLatin1( ":oldName" ), oldName );

    if ( !query.exec() )
        showError( query );
#else
    QueryHelper::instance()->
        executeStatement("UPDATE tag SET name=%s "
                         "WHERE name=%s AND "
                         "categoryId=(SELECT id FROM category WHERE name=%s)",
                         QueryHelper::Bindings() <<
                         newName << oldName << category->name());
#endif
}

void SQLDB::Database::deleteItem( DB::Category* /*category*/, const QString& /*option*/ )
{
    qDebug("NYI: void SQLDB::Database::deleteItem( DB::Category* category, const QString& option )" );
}

void SQLDB::Database::lockDB( bool /*lock*/, bool /*exclude*/ )
{
    qDebug("NYI: void SQLDB::Database::lockDB( bool lock, bool exclude )" );
}


bool SQLDB::Database::openConnection(const QString& username,
                                     const QString& password)
{
#ifndef HASKEXIDB
    QSqlDatabase* database = QSqlDatabase::addDatabase( "QMYSQL3" );
    if ( database == 0 ) {
        qFatal("What?!");
        return false;
    }

    database->setDatabaseName( "kphotoalbum" );
    if ( !username.isNull() ) {
        database->setUserName(username);
        if ( !password.isNull() )
            database->setPassword(password);
    }
    if ( !database->open() ) {
        qFatal("Couldn't open db");
        return false;
    }
    return true;
#else
    if (!_driver) {
        qDebug("openConnection: Driver should be initialized first");
        return false;
    }
    KexiDB::ConnectionData connData;
    connData.userName = username;
    connData.password = password;
    _connection = _driver->createConnection(connData);
    if (!_connection || _driver->error()) {
        // TODO: error handling (get from manager)
        tellError(QString::fromLatin1("cannot create connection or driver error"));
        return false;
    }
    if (!_connection->connect()) {
        tellError(QString::fromLatin1("connecting to db failed"));
        return false;
    }
    return true;
#endif
}

void SQLDB::Database::openDatabase()
{
#ifdef HASKEXIDB
    if (!_connection->databaseExists("kphotoalbum")) {
        createAndOpen();
        return;
    }
    qDebug("Opening database with Kexi");
    if (!_connection->useDatabase("kphotoalbum")) {
        qDebug("cannot use db kphotoalbum: %s",
               _connection->errorMsg().latin1());
        exit(-1);
    }
    qDebug("Opened");
#endif
}


void SQLDB::Database::loadMemberGroups()
{
#ifndef HASKEXIDB
    QSqlQuery membersQuery;
    if (!membersQuery.exec( "SELECT groupname, category, member FROM membergroup" ) )
        qFatal("Couldn't exec query");

    while ( membersQuery.next() ) {
        QString group = membersQuery.value(0).toString();
        QString category = membersQuery.value(1).toString();
        QString member = membersQuery.value(2).toString();
        _members.addMemberToGroup( category, group, member );
    }
#else
    QValueList<QString[3]> l = QueryHelper::instance()->
        executeQuery("SELECT c.name, t.name, f.name "
                     "FROM tag_relation tr, tag t, tag f, category c "
                     "WHERE tr.toTagId=t.id AND "
                     "tr.fromTagId=f.id AND "
                     "t.categoryId=c.id").asString3List();
    for (QValueList<QString[3]>::const_iterator i = l.begin();
         i != l.end(); ++i)
        _members.addMemberToGroup((*i)[0], (*i)[1], (*i)[2]);
#endif
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
    qDebug("NYI: SQLDB::Database::cutToClipboard");
}

QStringList SQLDB::Database::pasteFromCliboard( const QString& /*afterFile*/ )
{
    qDebug("NYI: SQLDB::Database::pasteFromCliboard");
    return QStringList();
}

bool SQLDB::Database::isClipboardEmpty()
{
    qDebug("NYI: SQLDB::Database::isClipboardEmpty");
    return true;
}

#include "Database.moc"
