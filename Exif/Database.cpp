#undef QT_NO_CAST_ASCII
#undef QT_CAST_NO_ASCII
#include "Exif/Database.h"
#include <qsqldatabase.h>
#include "options.h"
#include <qsqlquery.h>
#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>
#include "Exif/DatabaseElement.h"
#include "Database.h"
#include <qfile.h>
#include "util.h"
#include <kstandarddirs.h>
#include "mainview.h"
#include <kmessagebox.h>
#include <klocale.h>
#include "imagedb.h"

using namespace Exif;

static QValueList<DatabaseElement*> elements()
{
    static QValueList<DatabaseElement*> elms;

    if ( elms.count() == 0 ) {
        elms.append( new RationalExifElement( "Exif.Photo.FocalLength" ) );
        elms.append( new RationalExifElement( "Exif.Photo.ExposureTime" ) );

        elms.append( new RationalExifElement( "Exif.Photo.ApertureValue" ) );
        elms.append( new RationalExifElement( "Exif.Photo.FNumber" ) );
        //elms.append( new RationalExifElement( "Exif.Photo.FlashEnergy" ) );

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
    if ( !isAvailable() )
        return;

    bool dbExists = QFile::exists( exifDBFile() );
    if ( !dbExists )
        Util::copy( locate( "data", QString::fromLatin1( "kimdaba/exif-sqlite.db" ) ), exifDBFile() );

    openDatabase();

    if ( !dbExists ) {
        populateDatabase();
        offerInitialize();
    }
}


void Exif::Database::openDatabase()
{
    _db = QSqlDatabase::addDatabase( QString::fromLatin1( "QSQLITE" ), QString::fromLatin1( "exif" ) );
    Q_ASSERT( _db );

    _db->setDatabaseName( exifDBFile() );

    if ( !_db->open() )
        qFatal("Couldn't open db %s", _db->lastError().text().latin1());

}

void Exif::Database::populateDatabase()
{
    QStringList attributes;
    QValueList<DatabaseElement*> elms = elements();
    for( QValueList<DatabaseElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        attributes.append( (*tagIt)->createString() );
    }

    QSqlQuery query( QString::fromLatin1( "create table exif (filename string PRIMARY KEY, %1 )")
                     .arg( attributes.join( QString::fromLatin1(", ") ) ), _db );
    if ( !query.exec())
        showError( query );
}

void Exif::Database::add( const QString& fileName )
{
    if ( !isAvailable() )
        return;

    try {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fileName.local8Bit().data());
        Q_ASSERT(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData &exifData = image->exifData();
        insert( fileName, exifData );
    }
    catch (...)
    {
    }
}

void Exif::Database::remove( const QString& fileName )
{
    if ( !isAvailable() )
        return;

    QSqlQuery query( QString::fromLatin1( "DELETE FROM exif WHERE fileName=?" ), _db );
    query.bindValue( 0, fileName );
    if ( !query.exec() )
        showError( query );
}

void Exif::Database::insert( const QString& filename, Exiv2::ExifData data )
{
    QStringList formalList;
    QValueList<DatabaseElement*> elms = elements();
    for( QValueList<DatabaseElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        formalList.append( (*tagIt)->queryString() );
    }

    QSqlQuery query( QString::fromLatin1( "INSERT into exif values (?, %1) " ).arg( formalList.join( QString::fromLatin1( ", " ) ) ), _db );
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
        _instance = new Exif::Database();
    return _instance;
}

bool Exif::Database::isAvailable()
{
#ifdef QT_NO_SQL
    return false;
#endif

    return QSqlDatabase::isDriverAvailable( QString::fromLatin1( "QSQLITE" ) );
}

QString Exif::Database::exifDBFile()
{
    return Options::instance()->imageDirectory() + QString::fromLatin1("/exif-info.db");
}

Set<QString> Exif::Database::filesMatchingQuery( const QString& queryStr )
{
    if ( !isAvailable() )
        return Set<QString>();

    Set<QString> result;
    QSqlQuery query( queryStr, _db );

    if ( !query.exec() )
        showError( query );

    else {
        while ( query.next() )
            result.insert( query.value(0).toString() );
    }

    return result;
}

void Exif::Database::offerInitialize()
{
    int ret = KMessageBox::questionYesNo( MainView::theMainView(),
                                          i18n("<qt><p>Congratulation, your KimDaBa version now supports searching "
                                               "for EXIF information.</p>"
                                               "<p>For this to work, KimDaBa needs to rescan your images. "
                                               "Do you want this to happen now?</p>"),
                                          i18n("Rescan for EXIF information") );
    if ( ret == KMessageBox::Yes )
        ImageDB::instance()->slotReread( ImageDB::instance()->images(), EXIFMODE_DATABASE_UPDATE );

}

QValueList< QPair<QString,QString> > Exif::Database::cameras() const
{
    QValueList< QPair<QString,QString> > result;

    QSqlQuery query( "SELECT DISTINCT Exif_Image_Make, Exif_Image_Model FROM exif", _db );
    if ( !query.exec() )
        showError( query );

    else {
        while ( query.next() ) {
            QString make = query.value(0).toString();
            QString model = query.value(1).toString();
            if ( !make.isEmpty() && !model.isEmpty() )
                result.append( qMakePair( make, model ) );
        }
    }

    return result;
}

