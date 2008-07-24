/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
// PENDING(blackie) these should go away, and the code be clean
#undef QT_NO_CAST_FROM_ASCII
#undef QT_NO_CAST_TO_ASCII

#include "Exif/Database.h"
#include <qsqldatabase.h>
#include <qsqldriver.h>
#include <Q3ValueList>
#include "Settings/SettingsData.h"
#include <qsqlquery.h>
#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>
#include "Exif/DatabaseElement.h"
#include "Database.h"
#include <QProgressDialog>
#include <QDir>
#include <QDebug>
#include <DB/ImageDB.h>
#include <qfile.h>
#include "MainWindow/Window.h"
#include "Utilities/Util.h"
#include <kmessagebox.h>
#include <klocale.h>
#include "DB/ImageDB.h"
#include <QSqlError>

using namespace Exif;

static Q3ValueList<DatabaseElement*> elements()
{
    static Q3ValueList<DatabaseElement*> elms;

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
    const QString txt =
        i18n("<p>There was an error while executing the SQL backend command. "
             "The error is likely due to a broken database file.</p>"
             "<p>To fix this problem run Maintainance->Rebuild EXIF database.</p>"
             "<hr/>"
             "<p>For debugging, this is the command I tried to execute:<br/>%1</p>"
             "<p>This was the error message I got:<br/>%2</p>",
             query.lastQuery(), query.lastError().text() );

    KMessageBox::information( MainWindow::Window::theMainWindow(), txt, i18n("Error Executing Exif Command") );

    qWarning( "Error running query: %s\nError was: %s", qPrintable(query.lastQuery()), qPrintable(query.lastError().text()));
}

Exif::Database::Database()
    : _isOpen(false)
{
}


void Exif::Database::openDatabase()
{
    _db = QSqlDatabase::addDatabase( QString::fromLatin1( "QSQLITE" ), QString::fromLatin1( "exif" ) );

    _db.setDatabaseName( exifDBFile() );

    if ( !_db.open() )
        qWarning("Couldn't open db %s", qPrintable(_db.lastError().text()) );
    else
        _isOpen = true;

    // If SQLite in Qt has Unicode feature, it will convert querys to
    // UTF-8 automatically. Otherwise we should do the conversion to
    // be able to store any Unicode character.
    _doUTF8Conversion = !_db.driver()->hasFeature(QSqlDriver::Unicode);
}

Exif::Database::~Database()
{
    // We have to close the database before destroying the QSqlDatabase object,
    // otherwise Qt screams and kittens might die (see QSqlDatabase's
    // documentation)
    if ( _db.isOpen() )
        _db.close();
}

bool Exif::Database::isOpen() const
{
    return _isOpen;
}

void Exif::Database::populateDatabase()
{
    QStringList attributes;
    Q3ValueList<DatabaseElement*> elms = elements();
    for( Q3ValueList<DatabaseElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        attributes.append( (*tagIt)->createString() );
    }

    QSqlQuery query( QString::fromLatin1( "create table if not exists exif (filename string PRIMARY KEY, %1 )")
                     .arg( attributes.join( QString::fromLatin1(", ") ) ), _db );
    if ( !query.exec())
        showError( query );
}

bool Exif::Database::add( const QString& fileName )
{
    if ( !isUsable() )
        return false;

    try {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fileName.toLocal8Bit().data());
        Q_ASSERT(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData &exifData = image->exifData();
        insert( fileName, exifData );
        return true;
    }
    catch (...)
    {
    }
    return false;
}

void Exif::Database::remove( const QString& fileName )
{
    if ( !isUsable() )
        return;

    QSqlQuery query( QString::fromLatin1( "DELETE FROM exif WHERE fileName=?" ), _db );
    query.bindValue( 0, _doUTF8Conversion ? fileName.toUtf8() : fileName );
    if ( !query.exec() )
        showError( query );
}

void Exif::Database::insert( const QString& filename, Exiv2::ExifData data )
{
    if ( !isUsable() )
        return;

    QStringList formalList;
    Q3ValueList<DatabaseElement*> elms = elements();
    for( Q3ValueList<DatabaseElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        formalList.append( (*tagIt)->queryString() );
    }

    QSqlQuery query( QString::fromLatin1( "INSERT into exif values (?, %1) " ).arg( formalList.join( QString::fromLatin1( ", " ) ) ), _db );
    query.bindValue(  0, _doUTF8Conversion ? filename.toUtf8() : filename );
    int i = 1;
    for( Q3ValueList<DatabaseElement*>::Iterator tagIt = elms.begin(); tagIt != elms.end(); ++tagIt ) {
        (*tagIt)->bindValues( &query, i, data );
    }

    if ( !query.exec() )
        showError( query );

}

Exif::Database* Exif::Database::instance()
{
    if ( !_instance ) {
        _instance = new Exif::Database();
        _instance->init();
    }

    return _instance;
}

void Exif::Database::deleteInstance()
{
    delete _instance;
    _instance = 0;
}

bool Exif::Database::isAvailable()
{
#ifdef QT_NO_SQL
    return false;
#endif

    return QSqlDatabase::isDriverAvailable( QString::fromLatin1( "QSQLITE" ) );
}

bool Exif::Database::isUsable() const
{
    return (isAvailable() && isOpen());
}

QString Exif::Database::exifDBFile()
{
    return ::Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("/exif-info.db");
}

StringSet Exif::Database::filesMatchingQuery( const QString& queryStr )
{
    if ( !isUsable() )
        return StringSet();

    StringSet result;
    QSqlQuery query( queryStr, _db );

    if ( !query.exec() )
        showError( query );

    else {
        if ( _doUTF8Conversion )
            while ( query.next() )
                result.insert( QString::fromUtf8( query.value(0).toByteArray() ) );
        else
            while ( query.next() )
                result.insert( query.value(0).toString() );
    }

    return result;
}

QList< QPair<QString,QString> > Exif::Database::cameras() const
{
    QList< QPair<QString,QString> > result;

    if ( !isUsable() )
        return result;

    QSqlQuery query( QString::fromLatin1("SELECT DISTINCT Exif_Image_Make, Exif_Image_Model FROM exif"), _db );
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

void Exif::Database::init()
{
    if ( !isAvailable() )
        return;

    bool dbExists = QFile::exists( exifDBFile() );

    openDatabase();

    if ( !isOpen() )
        return;

    if ( !dbExists )
        populateDatabase();
}

void Exif::Database::recreate()
{
    _db.close();
    QDir().remove(exifDBFile());
    init();

    QStringList allImages = DB::ImageDB::instance()->images();
    QProgressDialog dialog;
    dialog.setLabelText(i18n("Rereading EXIF information from all images"));
    dialog.setMaximum( allImages.count() );
    int i = 0;
    bool OK = true;
    Q_FOREACH( const QString& fileName, allImages ) {
        dialog.setValue(i);
        OK = add(fileName);
        if ( !OK )
            break;
    }
}

