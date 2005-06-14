#include "sqlimageinfo.h"
#include "query.h"
#include <qsqlquery.h>
#include <util.h>

const QString imageInfoAttributes = "label, description, dayFrom, monthFrom, yearFrom, dayTo, monthTo, "
                                    "yearTo, hour, minute, second, angle, md5sum, width, height";

SQLDB::SQLImageInfo::SQLImageInfo( const QString& fileName )
    :ImageInfo()
{
    QString relativeFileName = Util::stripImageDirectory( fileName );
    _fileId = idForFileName( relativeFileName );

    QSqlQuery query;
    query.prepare( QString::fromLatin1( "SELECT %1 FROM imageinfo where fileId=:fileId" ).arg( imageInfoAttributes ) );
    query.bindValue( QString::fromLatin1( ":fileId" ), _fileId );
    if ( !query.exec() )
        showError( query );

    Q_ASSERT( query.numRowsAffected() == 1 );
    if ( query.numRowsAffected() < 1 ) {
        qWarning( "Internal Error: Didn't find %s (%s) fileId = %d in Database", fileName.latin1(), relativeFileName.latin1(), _fileId );
        return;
    }

    query.next();
    QString label = query.value(0).toString();
    QString description = query.value( 1 ).toString();
    int dayFrom = query.value( 2 ).toInt();
    int monthFrom = query.value( 3 ).toInt();
    int yearFrom = query.value( 4 ).toInt();
    int dayTo = query.value( 5 ).toInt();
    int monthTo = query.value( 6 ).toInt();
    int yearTo = query.value( 7 ).toInt();
    int hour = query.value( 8 ).toInt();
    int minute = query.value( 9 ).toInt();
    int second = query.value( 10 ).toInt();
    int angle = query.value( 11 ).toInt();
    QString     md5sum = query.value( 12 ).toString();
    int width = query.value( 13 ).toInt();
    int height = query.value( 14 ).toInt();


    ImageInfo::setFileName( relativeFileName );
    ImageInfo::setLabel( label );
    ImageInfo::setDescription( description );
    ImageInfo::setStartDate( ImageDate( dayFrom, monthFrom, yearFrom, hour, minute, second ) );
    ImageInfo::setEndDate( ImageDate( dayTo, monthTo, yearTo ) );
    ImageInfo::setAngle( angle );
    ImageInfo::setMD5Sum( md5sum );
    ImageInfo::setSize( QSize( width, height ) );

    query.prepare( QString::fromLatin1( "SELECT category, value FROM imagecategoryinfo WHERE fileId=:fileId" ) );
    query.bindValue( QString::fromLatin1( ":fileId" ), _fileId );
    if ( !query.exec() )
        showError( query );

    while ( query.next() ) {
        addOption( query.value(0).toString(), query.value(1).toString() );
    }
}

ImageInfo& SQLDB::SQLImageInfo::operator=( const ImageInfo& other )
{
    QStringList queryList;
    QMap<QString, QVariant> map;

    if ( label() != other.label() ) {
        queryList << QString::fromLatin1( "label = :label" );
        map.insert( QString::fromLatin1( ":label" ), other.label() );
    }

    if ( description() != other.description() ) {
        queryList << QString::fromLatin1( "description = :description" );
        map.insert( QString::fromLatin1( ":description" ), other.description() );
    }

    if ( angle() != other.angle() ) {
        queryList << QString::fromLatin1( "angle = :angle" );
        map.insert( QString::fromLatin1( ":angle" ), other.angle() );
    }

    if ( MD5Sum() != other.MD5Sum() ) {
        queryList << QString::fromLatin1( "md5sum = :md5sum" );
        map.insert( QString::fromLatin1( ":md5sum" ), other.MD5Sum() );
    }

    if ( size() != other.size() ) {
        queryList << QString::fromLatin1( "width = :width" ) << QString::fromLatin1( "height = :height" );
        map.insert( QString::fromLatin1( ":width" ), other.size().width() );
        map.insert( QString::fromLatin1( ":height" ), other.size().height() );
    }

    if ( queryList.count() ) {
        QString query = QString::fromLatin1( "UPDATE imageinfo set %1 WHERE fileId = :fileId" )
                        .arg( queryList.join( QString::fromLatin1( ", " ) ) );
        map.insert( QString::fromLatin1( ":fileId" ), _fileId );
        runQuery( query, map );
    }


    // Update categories
    QStringList categories = other.availableCategories();
    QSqlQuery insertQuery;
    insertQuery.prepare( QString::fromLatin1( "INSERT INTO imagecategoryinfo set fileId = :fileId, "
                                              "category = :category, value = :value" ) );
    insertQuery.bindValue( QString::fromLatin1( ":fileId" ), _fileId );

    QSqlQuery removeQuery;
    removeQuery.prepare( QString::fromLatin1( "DELETE FROM imagecategoryinfo WHERE fileId = :fileId AND "
                                              "category = :category AND value = :value" ) );
    removeQuery.bindValue( QString::fromLatin1( ":fileId" ), _fileId );

    for( QStringList::ConstIterator categoryIt = categories.constBegin(); categoryIt != categories.constEnd(); ++categoryIt ) {
        QStringList myItems = itemsOfCategory( *categoryIt );
        QStringList otherItems = other.itemsOfCategory( *categoryIt );
        QStringList newItems = Util::diff( otherItems, myItems );
        QStringList deadItems = Util::diff( myItems, otherItems );

        insertQuery.bindValue( QString::fromLatin1( ":category" ), *categoryIt );
        removeQuery.bindValue( QString::fromLatin1( ":category" ), *categoryIt );

        for( QStringList::Iterator itemIt = newItems.begin(); itemIt != newItems.end(); ++itemIt ) {
            insertQuery.bindValue( QString::fromLatin1( ":value" ), *itemIt );
            if ( !insertQuery.exec() )
                showError( insertQuery );
            // PENDING(blackie) also insert into categorysortorder
        }

        for( QStringList::Iterator itemIt = deadItems.begin(); itemIt != deadItems.end(); ++itemIt ) {
            removeQuery.bindValue( QString::fromLatin1( ":value" ), *itemIt );
            if ( !removeQuery.exec() )
                showError( removeQuery );
        }

    }

    // PENDING(blackie) save draw list.

    return ImageInfo::operator=( other );
}
