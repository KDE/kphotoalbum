#undef QT_NO_CAST_ASCII
#undef QT_CAST_NO_ASCII
#include "exifdatabase.h"
#include <qsqldatabase.h>
#include "options.h"
#include <qsqlquery.h>
#include <exiv2/exif.hpp>

class ExifElement
{
public:
    virtual QString createString() = 0; // Exif_Photo_FNumber_denominator int, Exif_Photo_FNumber_nominator int
    virtual QString queryString() = 0; // ?, ?
    virtual void bindValues( QSqlQuery*, int& counter, Exiv2::ExifData& data ) = 0; // bind values
};

static QString replaceDotWithUnderscore( const char* cstr )
{
    QString str( QString::fromLatin1( cstr ) );
    return str.replace( QString::fromLatin1( "." ), QString::fromLatin1( "_" ) );
}

class StringExifElement :public ExifElement
{
public:
    StringExifElement( const char* tag ) : _tag( tag ) {}

    QString createString()
    {
        return QString::fromLatin1( "%1 string" ).arg( replaceDotWithUnderscore( _tag ) );
    }

    QString queryString()
    {
        return QString::fromLatin1( "?" );
    }

    void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data )
    {
        query->bindValue( counter++, data[_tag].toString().c_str() );
    }

private:
    const char* _tag;
};

class IntExifElement :public ExifElement
{
public:
    IntExifElement( const char* tag ) : _tag( tag ) {}

    QString createString()
    {
        return QString::fromLatin1( "%1 int" ).arg( replaceDotWithUnderscore( _tag ) );
    }

    QString queryString()
    {
        return QString::fromLatin1( "?" );
    }

    void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data )
    {
        query->bindValue( counter++, (int) data[_tag].toLong() );
    }

private:
    const char* _tag;
};


class RationalExifElement :public ExifElement
{
public:
    RationalExifElement( const char* tag ) : _tag( tag ) {}
    virtual QString createString()
    {
        return QString::fromLatin1( "%1_denom int, %2_nom int" ).arg( replaceDotWithUnderscore( _tag ) )
            .arg( replaceDotWithUnderscore( _tag ) );
    }

    virtual QString queryString()
    {
        return QString::fromLatin1( "?, ?" );
    }

    virtual void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data )
    {
        query->bindValue( counter++, data[_tag].toRational().first );
        query->bindValue( counter++, data[_tag].toRational().second);
    }

private:
    const char* _tag;
};



static QValueList<ExifElement*> elements()
{
    static QValueList<ExifElement*> elms;

    if ( elms.count() == 0 ) {
        elms.append( new RationalExifElement( "Exif.Photo.FocalLength" ) );
        elms.append( new RationalExifElement( "Exif.Photo.ExposureTime" ) );

        elms.append( new RationalExifElement( "Exif.Photo.FNumber" ) );
        elms.append( new RationalExifElement( "Exif.Photo.FlashEnergy" ) );

        elms.append( new IntExifElement( "Exif.Photo.Flash" ) );
        elms.append( new IntExifElement( "Exif.Photo.Contrast" ) );
        elms.append( new IntExifElement( "Exif.Photo.Sharpness" ) );
        elms.append( new IntExifElement( "Exif.Photo.Saturation" ) );
        elms.append( new IntExifElement( "Exif.Image.Orientation" ) );
        elms.append( new IntExifElement( "Exif.Photo.MeteringMode" ) );
        elms.append( new IntExifElement( "Exif.Photo.ISOSpeedRatings" ) );

        elms.append( new StringExifElement( "Exif.Image.Make" ) );
        elms.append( new StringExifElement( "Exif.Image.Model" ) );
    }

    return elms;
}



















ExifDatabase* ExifDatabase::_instance = 0;

static void showError( QSqlQuery& query )
{
    qWarning( "Error running query: %s\nError was: %s", query.executedQuery().latin1(), query.lastError().text().latin1());
}

ExifDatabase::ExifDatabase()
{
    QSqlDatabase* database = QSqlDatabase::addDatabase( QString::fromLatin1( "QSQLITE" ) );
    Q_ASSERT( database );

    database->setDatabaseName( Options::instance()->imageDirectory() + "/kimdaba.db" );

    if ( !database->open() )
        qFatal("Couldn't open db %s", database->lastError().text().latin1());

    setup();
}


void ExifDatabase::setup()
{
    qDebug("Setup!");
    QStringList list;
    QValueList<ExifElement*> elms = elements();
    for( QValueList<ExifElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        list.append( (*tagIt)->createString() );
    }

    QSqlQuery query( QString::fromLatin1( "create table exif (filename string, %1 )").arg( list.join( QString::fromLatin1(", ") ) ) );
    if ( !query.exec())
        showError( query );

}

void ExifDatabase::insert( const QString& filename, Exiv2::ExifData data )
{
    QStringList formalList;
    QValueList<ExifElement*> elms = elements();
    for( QValueList<ExifElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        formalList.append( (*tagIt)->queryString() );
    }

    QSqlQuery query( QString::fromLatin1( "INSERT into exif values (?, %1) " ).arg( formalList.join( QString::fromLatin1( ", " ) ) ) );
    query.bindValue(  0, filename );
    int i = 1;
    for( QValueList<ExifElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        (*tagIt)->bindValues( &query, i, data );
    }

    if ( !query.exec() )
        showError( query );
}

ExifDatabase* ExifDatabase::instance()
{
    if ( !_instance )
        _instance = new ExifDatabase;
    return _instance;
}

RationalList ExifDatabase::rationalValue( const QString& tag )
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
