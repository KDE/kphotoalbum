#undef QT_NO_CAST_ASCII
#undef QT_CAST_NO_ASCII
#include "Exif/Database.h"
#include <qsqldatabase.h>
#include "options.h"
#include <qsqlquery.h>
#include <exiv2/exif.hpp>
#include "Exif/DatabaseElement.h"

using namespace Exif;

static QValueList<DatabaseElement*> elements()
{
    static QValueList<DatabaseElement*> elms;

    if ( elms.count() == 0 ) {
        elms.append( new RationalExifElement( "Exif.Photo.FocalLength" ) );
        elms.append( new RationalExifElement( "Exif.Photo.ExposureTime" ) );

        elms.append( new RationalExifElement( "Exif.Photo.ApertureValue" ) );
        elms.append( new RationalExifElement( "Exif.Photo.FlashEnergy" ) );

        elms.append( new IntExifElement( "Exif.Photo.Flash" ) );
        elms.append( new IntExifElement( "Exif.Photo.Contrast" ) );
        elms.append( new IntExifElement( "Exif.Photo.Sharpness" ) );
        elms.append( new IntExifElement( "Exif.Photo.Saturation" ) );
        elms.append( new IntExifElement( "Exif.Image.Orientation" ) );
        elms.append( new IntExifElement( "Exif.Photo.MeteringMode" ) );
        elms.append( new IntExifElement( "Exif.Photo.ISOSpeedRatings" ) );
        elms.append( new IntExifElement( "Exif.Photo.ExposureProgram" ) );

        elms.append( new StringExifElement( "Exif.Image.Make" ) );
        elms.append( new StringExifElement( "Exif.Image.Model" ) );
    }

    return elms;
}

Exif::Database* Exif::Database::_instance = 0;

static void showError( QSqlQuery& query )
{
    qWarning( "Error running query: %s\nError was: %s", query.executedQuery().latin1(), query.lastError().text().latin1());
}

Exif::Database::Database()
{
    QSqlDatabase* database = QSqlDatabase::addDatabase( QString::fromLatin1( "QSQLITE" ) );
    Q_ASSERT( database );

    database->setDatabaseName( Options::instance()->imageDirectory() + "/kimdaba.db" );

    if ( !database->open() )
        qFatal("Couldn't open db %s", database->lastError().text().latin1());

    setup();
}


void Exif::Database::setup()
{
    qDebug("Setup!");
    QStringList list;
    QValueList<DatabaseElement*> elms = elements();
    for( QValueList<DatabaseElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        list.append( (*tagIt)->createString() );
    }

    QSqlQuery query( QString::fromLatin1( "create table exif (filename string, %1 )").arg( list.join( QString::fromLatin1(", ") ) ) );
    if ( !query.exec())
        showError( query );

}

void Exif::Database::insert( const QString& filename, Exiv2::ExifData data )
{
    QStringList formalList;
    QValueList<DatabaseElement*> elms = elements();
    for( QValueList<DatabaseElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        formalList.append( (*tagIt)->queryString() );
    }

    QSqlQuery query( QString::fromLatin1( "INSERT into exif values (?, %1) " ).arg( formalList.join( QString::fromLatin1( ", " ) ) ) );
    query.bindValue(  0, filename );
    int i = 1;
    for( QValueList<DatabaseElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        (*tagIt)->bindValues( &query, i, data );
    }

    if ( !query.exec() )
        showError( query );
}

Exif::Database* Exif::Database::instance()
{
    if ( !_instance )
        _instance = new Exif::Database;
    return _instance;
}

// PENDING(blackie) This function is outdated!
RationalList Exif::Database::rationalValue( const QString& tag )
{
    RationalList result;
    QSqlQuery query( QString::fromLatin1( "SELECT DISTINCT %1_denom,%2_nom FROM exif" ).arg(tag).arg(tag));

    if ( !query.exec() ) {
        showError( query );
        return result;
    }

    QMap<int,Rational> map;
    while ( query.next() ) {
        int denominator = query.value(0).toInt();
        int nominator = query.value(1).toInt();
        float f = (1.0 * denominator) / nominator;
        qDebug("%f", f );
        map.insert( (int) (10000*f), Rational( denominator, nominator ), true );
    }

    QValueList<int> keys = map.keys();
    qHeapSort(keys);

    for( QValueList<int>::ConstIterator it = keys.begin(); it != keys.end(); ++it ) {
        result.append( map[*it] );
        qDebug("%d,%d", map[*it].first, map[*it].second );
    }

    return result;
}
