#include "SQLDB.h"
#include <membermap.h>
#include <qsqldatabase.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <stdlib.h>
#include <categorycollection.h>
#include <qsqlerror.h>
#include "Query.h"
#include "QueryUtil.h"
#include <imageinfo.h>
#include <util.h>
#include "groupCounter.h"
#include <kdebug.h>
#include "SQLImageInfo.h"
#include "Browser/Browser.h"
#include "SQLImageDateCollection.h"

SQLDB::SQLDB::SQLDB() :_members( this )
{
    if ( !QSqlDatabase::isDriverAvailable( QString::fromLatin1("QMYSQL3") ) ) {
        // PENDING(blackie) better message
        KMessageBox::sorry( 0, i18n("The MySQL driver did not seem to be compiled into your Qt" ) );
        exit(-1);
    }

    openDatabase();
    loadMemberGroups();
}

int SQLDB::SQLDB::totalCount() const
{
    QString query = QString::fromLatin1( "SELECT COUNT(fileId) FROM sortorder" );
    return fetchItem( query ).toInt();
}

QStringList SQLDB::SQLDB::search( const ImageSearchInfo& info, bool /*requireOnDisk*/ ) const
{
    // PENDING(blackie) Handle on disk.
    QValueList<int> matches = filesMatchingQuery( info );
    QStringList result;
    for( QValueList<int>::Iterator it = matches.begin(); it != matches.end(); ++it ) {
        result.append( fileNameForId( *it, true ) );
    }
    return result;
}

void SQLDB::SQLDB::renameCategory( const QString& oldName, const QString newName )
{
    QSqlQuery query;
    query.prepare( "UPDATE imagecategoryinfo SET category=:newName WHERE category=:oldName" );
    query.bindValue( QString::fromLatin1( ":newName" ), newName );
    query.bindValue( QString::fromLatin1( ":oldName" ), oldName );
    if ( !query.exec() )
        showError( query );
}

QMap<QString,int> SQLDB::SQLDB::classify( const ImageSearchInfo& info, const QString& category )
{
    bool allFiles = true;
    QValueList<int> includedFiles;
    if ( !info.isNull() ) {
        includedFiles = filesMatchingQuery( info );
        allFiles = false;
    }

    QMap<QString,int> result;
    GroupCounter counter( category );
    QDict<void> alreadyMatched = info.findAlreadyMatched( category );


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

    // Count images that doesn't contain an item
    if ( allFiles )
        result[ImageDB::NONE()] = totalCount() - itemMap.count();
    else
        result[ImageDB::NONE()] = includedFiles.count() - itemMap.count();


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

ImageInfoList& SQLDB::SQLDB::imageInfoList()
{
    qDebug("NYI: ImageInfoList& SQLDB::SQLDB::imageInfoList()" );
    static ImageInfoList list;
    return list;
}

QStringList SQLDB::SQLDB::imageList( bool withRelativePath )
{
    QSqlQuery query;
    if ( !query.exec( QString::fromLatin1( "SELECT fileId FROM sortorder" ) ) )
        showError( query );
    QStringList result;
    while ( query.next() )
        result << fileNameForId( query.value(0).toInt(), withRelativePath );
    return result;
}


QStringList SQLDB::SQLDB::images()
{
    return imageList( false );
}

void SQLDB::SQLDB::addImages( const ImageInfoList& images )
{
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
    for( ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        ImageInfoPtr info = *it;
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

    emit totalChanged( totalCount() );
}

void SQLDB::SQLDB::addToBlockList( const QStringList& /*list*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::addToBlockList( const QStringList& list )" );
}

bool SQLDB::SQLDB::isBlocking( const QString& /*fileName*/ )
{
    qDebug("NYI: bool SQLDB::SQLDB::isBlocking( const QString& fileName )" );
    return false;
}

void SQLDB::SQLDB::deleteList( const QStringList& list )
{
    QStringList sortOrder = imageList( true );
    QSqlQuery imageInfoQuery, imageCategoryQuery;
    imageInfoQuery.prepare( "DELETE FROM imageinfo where fileId=:fileId" );
    imageCategoryQuery.prepare( "DELETE FROM imagecategoryinfo where fileId=:fileId" );

    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        QString fileName = Util::stripImageDirectory( *it );
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
    emit totalChanged( totalCount() );
}

ImageInfoPtr SQLDB::SQLDB::info( const QString& fileName ) const
{
    return new SQLImageInfo( fileName );
}

const MemberMap& SQLDB::SQLDB::memberMap()
{
    return _members;
}

void SQLDB::SQLDB::setMemberMap( const MemberMap& map )
{
    _members = map;
}

void SQLDB::SQLDB::save( const QString& /*fileName*/, bool /*isAutoSave*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::save( const QString& fileName )" );
}

MD5Map* SQLDB::SQLDB::md5Map()
{
    return &_md5map;
}

void SQLDB::SQLDB::sortAndMergeBackIn( const QStringList& /*fileList*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::sortAndMergeBackIn( const QStringList& fileList )" );
}

void SQLDB::SQLDB::renameItem( Category* category, const QString& oldName, const QString& newName )
{
    QSqlQuery query;
    query.prepare( QString::fromLatin1( "UPDATE imagecategoryinfo SET value = :newName "
                                        "WHERE category = :category and value = :oldName" ));
    query.bindValue( QString::fromLatin1( ":newName" ), newName );
    query.bindValue( QString::fromLatin1( ":category" ), category->name() );
    query.bindValue( QString::fromLatin1( ":oldName" ), oldName );

    if ( !query.exec() )
        showError( query );

}

void SQLDB::SQLDB::deleteItem( Category* /*category*/, const QString& /*option*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::deleteItem( Category* category, const QString& option )" );
}

void SQLDB::SQLDB::lockDB( bool /*lock*/, bool /*exclude*/ )
{
    qDebug("NYI: void SQLDB::SQLDB::lockDB( bool lock, bool exclude )" );
}


void SQLDB::SQLDB::openDatabase()
{
    QSqlDatabase* database = QSqlDatabase::addDatabase( "QMYSQL3" );
    if ( database == 0 ) {
        qFatal("What?!");
    }

    database->setDatabaseName( "kphotoalbum" );
    database->setUserName("root"); // PENDING(blackie) change
    if ( !database->open() )
        qFatal("Couldn't open db");
}

void SQLDB::SQLDB::loadMemberGroups()
{
    QSqlQuery membersQuery;
    if (!membersQuery.exec( "SELECT groupname, category, member FROM membergroup" ) )
        qFatal("Couldn't exec query");

    while ( membersQuery.next() ) {
        QString group = membersQuery.value(0).toString();
        QString category = membersQuery.value(1).toString();
        QString member = membersQuery.value(2).toString();
        _members.addMemberToGroup( category, group, member );
    }
}


CategoryCollection* SQLDB::SQLDB::categoryCollection()
{
    // PENDING(blackie) Implement something similar to XMLDB::createSpecialCategories()
    return &_categoryCollection;
}

KSharedPtr<ImageDateCollection> SQLDB::SQLDB::rangeCollection()
{
    return new SQLImageDateCollection( /*search( Browser::instance()->currentContext(), false ) */ );
}

void SQLDB::SQLDB::reorder( const QString& /*item*/, const QStringList& /*cutList*/, bool /*after*/)
{
    qDebug("Not Yet implemented SQLDB::SQLDB::reorder");
}

void SQLDB::SQLDB::cutToClipboard( const QStringList& /*list*/ )
{
    qDebug("NYI: SQLDB::SQLDB::cutToClipboard");
}

QStringList SQLDB::SQLDB::pasteFromCliboard( const QString& /*afterFile*/ )
{
    qDebug("NYI: SQLDB::SQLDB::pasteFromCliboard");
    return QStringList();
}

bool SQLDB::SQLDB::isClipboardEmpty()
{
    qDebug("NYI: SQLDB::SQLDB::isClipboardEmpty");
    return true;
}

#include "SQLDB.moc"
