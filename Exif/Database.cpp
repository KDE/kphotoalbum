/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include "Database.h"
#include "Logging.h"

#include "DB/ImageDB.h"
#include "Exif/DatabaseElement.h"
#include "MainWindow/Window.h"
#include "Settings/SettingsData.h"

#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>

#include <KLocalizedString>
#include <kmessagebox.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QProgressDialog>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

using namespace Exif;

namespace {
// schema version; bump it up whenever the database schema changes
constexpr int DB_VERSION = 3;
const Database::ElementList elements(int since=0)
{
    static Database::ElementList elms;
    static int sinceDBVersion[DB_VERSION]{};

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
        // gps info has been added in database schema version 2:
        sinceDBVersion[1] = elms.size();
        elms.append( new IntExifElement( "Exif.GPSInfo.GPSVersionID" ) ); // actually a byte value
        elms.append( new RationalExifElement( "Exif.GPSInfo.GPSAltitude" ) );
        elms.append( new IntExifElement( "Exif.GPSInfo.GPSAltitudeRef" ) ); // actually a byte value
        elms.append( new StringExifElement( "Exif.GPSInfo.GPSMeasureMode" ) );
        elms.append( new RationalExifElement( "Exif.GPSInfo.GPSDOP" ) );
        elms.append( new RationalExifElement( "Exif.GPSInfo.GPSImgDirection" ) );
        elms.append( new RationalExifElement( "Exif.GPSInfo.GPSLatitude" ) );
        elms.append( new StringExifElement( "Exif.GPSInfo.GPSLatitudeRef" ) );
        elms.append( new RationalExifElement( "Exif.GPSInfo.GPSLongitude" ) );
        elms.append( new StringExifElement( "Exif.GPSInfo.GPSLongitudeRef" ) );
        elms.append( new RationalExifElement( "Exif.GPSInfo.GPSTimeStamp" ) );
        // lens info has been added in database schema version 3:
        sinceDBVersion[2] = elms.size();
        elms.append( new LensExifElement(  ) );
    }

    // query only for the newly added stuff:
    if (since > 0)
        return elms.mid(sinceDBVersion[since]);

    return elms;
}
}

Exif::Database* Exif::Database::s_instance = nullptr;

/**
 * @brief show and error message for the failed \p query and disable the Exif database.
 * The database is closed because at this point we can not trust the data inside.
 * @param query
 */
void Database::showErrorAndFail(QSqlQuery &query) const
{
    const QString txt =
        i18n("<p>There was an error while accessing the Exif search database. "
             "The error is likely due to a broken database file.</p>"
             "<p>To fix this problem run Maintenance->Recreate Exif Search database.</p>"
             "<hr/>"
             "<p>For debugging: the command that was attempted to be executed was:<br/>%1</p>"
             "<p>The error message obtained was:<br/>%2</p>",
             query.lastQuery(), query.lastError().text() );

    const QString technicalInfo =
            QString::fromUtf8("Error running query: %s\n Error was: %s")
            .arg(query.lastQuery(), query.lastError().text());
    showErrorAndFail(txt, technicalInfo);
}

void Database::showErrorAndFail(const QString &errorMessage, const QString &technicalInfo) const
{
    KMessageBox::information( MainWindow::Window::theMainWindow(), errorMessage, i18n("Error in Exif database"), QString::fromLatin1( "sql_error_in_exif_DB" ) );

    qCWarning(ExifLog) << technicalInfo;

    // disable exif db for now:
    m_isFailed = true;
}

Exif::Database::Database()
    : m_isOpen(false)
    , m_isFailed(false)
{
    m_db = QSqlDatabase::addDatabase( QString::fromLatin1( "QSQLITE" ), QString::fromLatin1( "exif" ) );
}


void Exif::Database::openDatabase()
{
    m_db.setDatabaseName( exifDBFile() );

    m_isOpen = m_db.open();
    if ( !m_isOpen )
    {
        const QString txt =
                i18n("<p>There was an error while opening the Exif search database.</p> "
                     "<p>To fix this problem run Maintenance->Recreate Exif Search database.</p>"
                     "<hr/>"
                     "<p>The error message obtained was:<br/>%1</p>",
                      m_db.lastError().text() );
        const QString logMsg = QString::fromUtf8("Could not open Exif search database! "
                                    "Error was: %s").arg(m_db.lastError().text());
        showErrorAndFail(txt, logMsg);
        return;
    }
    // If SQLite in Qt has Unicode feature, it will convert queries to
    // UTF-8 automatically. Otherwise we should do the conversion to
    // be able to store any Unicode character.
    m_doUTF8Conversion = !m_db.driver()->hasFeature(QSqlDriver::Unicode);
}

Exif::Database::~Database()
{
    // We have to close the database before destroying the QSqlDatabase object,
    // otherwise Qt screams and kittens might die (see QSqlDatabase's
    // documentation)
    if ( m_db.isOpen() )
        m_db.close();
}

bool Exif::Database::isOpen() const
{
    return m_isOpen && ! m_isFailed;
}

void Exif::Database::populateDatabase()
{
    createMetadataTable(SchemaAndDataChanged);
    QStringList attributes;
    Q_FOREACH( DatabaseElement *element, elements() ) {
        attributes.append( element->createString() );
    }

    QSqlQuery query( QString::fromLatin1( "create table if not exists exif (filename string PRIMARY KEY, %1 )")
                     .arg( attributes.join( QString::fromLatin1(", ") ) ), m_db );
    if ( !query.exec())
        showErrorAndFail( query );
}

void Exif::Database::updateDatabase()
{
    if ( m_db.tables().isEmpty())
    {
        const QString txt =
                i18n("<p>The Exif search database is corrupted and has no data.</p> "
                     "<p>To fix this problem run Maintenance->Recreate Exif Search database.</p>"
                      );
        const QString logMsg = QString::fromUtf8("Database open but empty!");
        showErrorAndFail(txt, logMsg);
        return;
    }


    const int version = DBFileVersion();
    if (m_isFailed)
        return;
    if (version < DBVersion())
    {
        // on the next update, we can just query the DB Version
        createMetadataTable(SchemaChanged);
    }
    // update schema
    if ( version < DBVersion() )
    {
        QSqlQuery query( m_db );
        for( const DatabaseElement *e : elements(version))
        {
            query.prepare( QString::fromLatin1( "alter table exif add column %1")
                           .arg( e->createString()) );
            if ( !query.exec())
                showErrorAndFail( query );
        }
    }
}

void Exif::Database::createMetadataTable(DBSchemaChangeType change)
{
    QSqlQuery query(m_db);
    query.prepare( QString::fromLatin1( "create table if not exists settings (keyword TEXT PRIMARY KEY, value TEXT) without rowid") );
    if ( !query.exec())
    {
        showErrorAndFail( query );
        return;
    }

    query.prepare( QString::fromLatin1( "insert or replace into settings (keyword, value) values('DBVersion','%1')").arg( Database::DBVersion()));
    if ( !query.exec())
    {
        showErrorAndFail( query );
        return;
    }

    if (change == SchemaAndDataChanged)
    {
        query.prepare( QString::fromLatin1( "insert or replace into settings (keyword, value) values('GuaranteedDataVersion','%1')").arg( Database::DBVersion()));
        if ( !query.exec())
            showErrorAndFail( query );
    }
}

bool Exif::Database::add( const DB::FileName& fileName )
{
    if ( !isUsable() )
        return false;

    try {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fileName.absolute().toLocal8Bit().data());
        Q_ASSERT(image.get() != nullptr);
        image->readMetadata();
        Exiv2::ExifData &exifData = image->exifData();
        return insert( fileName, exifData );
    }
    catch (...)
    {
        qCWarning(ExifLog, "Error while reading exif information from %s", qPrintable(fileName.absolute()) );
        return false;
    }
}

bool Exif::Database::add( DB::FileInfo& fileInfo )
{
    if ( !isUsable() )
        return false;

    return insert( fileInfo.getFileName(), fileInfo.getExifData() );
}

bool Exif::Database::add( const DB::FileNameList& list )
{
    if ( !isUsable() )
        return false;

    QList<DBExifInfo> map;

    Q_FOREACH(const DB::FileName& fileName, list) {
        try {
            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fileName.absolute().toLocal8Bit().data());
            Q_ASSERT(image.get() != nullptr);
            image->readMetadata();
            map << DBExifInfo(fileName, image->exifData());
        }
        catch (...)
        {
            qWarning("Error while reading exif information from %s", qPrintable(fileName.absolute()) );
        }
    }
    insert(map);
    return true;
}

void Exif::Database::remove( const DB::FileName& fileName )
{
    if ( !isUsable() )
        return;

    QSqlQuery query( m_db);
    query.prepare( QString::fromLatin1( "DELETE FROM exif WHERE fileName=?" ));
    query.bindValue( 0, fileName.absolute() );
    if ( !query.exec() )
        showErrorAndFail( query );
}

void Exif::Database::remove( const DB::FileNameList& list )
{
    if ( !isUsable() )
        return;

    m_db.transaction();
    QSqlQuery query( m_db);
    query.prepare( QString::fromLatin1( "DELETE FROM exif WHERE fileName=?" ));
    Q_FOREACH(const DB::FileName& fileName, list) {
        query.bindValue( 0, fileName.absolute() );
        if ( !query.exec() )
        {
            m_db.rollback();
            showErrorAndFail( query );
            return;
        }
    }
    m_db.commit();
}

QSqlQuery *Exif::Database::getInsertQuery()
{
    if ( !isUsable() )
        return nullptr;
    if ( m_insertTransaction )
        return m_insertTransaction;
    if (m_queryString.isEmpty())
    {
        QStringList formalList;
        Database::ElementList elms = elements();
        for( const DatabaseElement *e : elms )
        {
            formalList.append( e->queryString() );
        }
        m_queryString = QString::fromLatin1( "INSERT OR REPLACE into exif values (?, %1) " ).arg( formalList.join( QString::fromLatin1( ", " ) ) );
    }
    QSqlQuery *query = new QSqlQuery(m_db);
    if (query)
        query->prepare( m_queryString );
    return query;
}

void Exif::Database::concludeInsertQuery( QSqlQuery *query )
{
    if ( m_insertTransaction )
        return;
    m_db.commit();
    delete query;
}

bool Exif::Database::startInsertTransaction()
{
    Q_ASSERT(m_insertTransaction == nullptr);
    m_insertTransaction = getInsertQuery();
    m_db.transaction();
    return ( m_insertTransaction != nullptr );
}

bool Exif::Database::commitInsertTransaction()
{
    if (m_insertTransaction) {
        m_db.commit();
        delete m_insertTransaction;
        m_insertTransaction = nullptr;
    } else
        qCWarning(ExifLog, "Trying to commit transaction, but no transaction is active!");
    return true;
}

bool Exif::Database::abortInsertTransaction()
{
    if (m_insertTransaction) {
        m_db.rollback();
        delete m_insertTransaction;
        m_insertTransaction = nullptr;
    } else
        qCWarning(ExifLog, "Trying to abort transaction, but no transaction is active!");
    return true;
}

bool Exif::Database::insert(const DB::FileName& filename, Exiv2::ExifData data )
{
    if ( !isUsable() )
        return false;

    QSqlQuery *query = getInsertQuery();
    query->bindValue(  0, filename.absolute() );
    int i = 1;
    for( const DatabaseElement *e : elements() )
    {
        query->bindValue( i++, e->valueFromExif(data));
    }

    bool status = query->exec();
    if ( !status )
        showErrorAndFail( *query );
    concludeInsertQuery( query );
    return status;
}

bool Exif::Database::insert(QList<DBExifInfo> map )
{
    if ( !isUsable() )
        return false;

    QSqlQuery *query = getInsertQuery();
    // not a const reference because DatabaseElement::valueFromExif uses operator[] on the exif datum
    Q_FOREACH ( DBExifInfo elt, map )
    {
        query->bindValue(  0, elt.first.absolute() );
        int i = 1;
        for( const DatabaseElement *e : elements() )
        {
            query->bindValue( i++, e->valueFromExif(elt.second));
        }

        if ( !query->exec() )
        {
            showErrorAndFail( *query );
        }
    }
    concludeInsertQuery( query );
    return true;
}

Exif::Database* Exif::Database::instance()
{
    if ( !s_instance ) {
        qCInfo(ExifLog) << "initializing Exif database...";
        s_instance = new Exif::Database();
        s_instance->init();
    }

    return s_instance;
}

void Exif::Database::deleteInstance()
{
    delete s_instance;
    s_instance = nullptr;
}

bool Exif::Database::isAvailable()
{
#ifdef QT_NO_SQL
    return false;
#else
    return QSqlDatabase::isDriverAvailable( QString::fromLatin1( "QSQLITE" ) );
#endif
}

int Exif::Database::DBFileVersion() const
{
    // previous to KPA 4.6, there was no metadata table:
    if ( !m_db.tables().contains( QString::fromLatin1("settings")) )
        return 1;

    QSqlQuery query( QString::fromLatin1("SELECT value FROM settings WHERE keyword = 'DBVersion'"), m_db );
    if ( !query.exec() )
        showErrorAndFail( query );

    if (query.first())
    {
        return query.value(0).toInt();
    }
    return 0;
}

int Exif::Database::DBFileVersionGuaranteed() const
{
    // previous to KPA 4.6, there was no metadata table:
    if ( !m_db.tables().contains( QString::fromLatin1("settings")) )
        return 0;

    QSqlQuery query( QString::fromLatin1("SELECT value FROM settings WHERE keyword = 'GuaranteedDataVersion'"), m_db );
    if ( !query.exec() )
        showErrorAndFail( query );

    if (query.first())
    {
        return query.value(0).toInt();
    }
    return 0;
}

constexpr int Exif::Database::DBVersion()
{
    return DB_VERSION;
}

bool Exif::Database::isUsable() const
{
    return (isAvailable() && isOpen());
}

QString Exif::Database::exifDBFile()
{
    return ::Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("/exif-info.db");
}

bool Exif::Database::readFields( const DB::FileName& fileName, ElementList &fields) const
{
    if ( !isUsable() )
        return false;

    bool foundIt = false;
    QStringList fieldList;
    for( const DatabaseElement *e : fields )
    {
        fieldList.append( e->columnName() );
    }

    QSqlQuery query( m_db );
    // the query returns a single value, so we don't need the overhead for random access:
    query.setForwardOnly( true );

    query.prepare( QString::fromLatin1( "select %1 from exif where filename=?")
                   .arg( fieldList.join( QString::fromLatin1(", "))) );
    query.bindValue( 0, fileName.absolute() );

    if ( !query.exec() ) {
        showErrorAndFail( query );
    }
    if ( query.next() )
    {
        // file in exif db -> write back results
        int i=0;
        for( DatabaseElement *e : fields )
        {
            e->setValue( query.value(i++) );
        }
        foundIt = true;
    }
    return foundIt;
}

DB::FileNameSet Exif::Database::filesMatchingQuery( const QString& queryStr ) const
{
    if ( !isUsable() )
        return DB::FileNameSet();

    DB::FileNameSet result;
    QSqlQuery query( queryStr, m_db );

    if ( !query.exec() )
        showErrorAndFail( query );

    else {
        if ( m_doUTF8Conversion )
            while ( query.next() )
                result.insert( DB::FileName::fromAbsolutePath( QString::fromUtf8( query.value(0).toByteArray() ) ) );
        else
            while ( query.next() )
                result.insert( DB::FileName::fromAbsolutePath( query.value(0).toString() ) );
    }

    return result;
}

QList< QPair<QString,QString> > Exif::Database::cameras() const
{
    QList< QPair<QString,QString> > result;

    if ( !isUsable() )
        return result;

    QSqlQuery query( QString::fromLatin1("SELECT DISTINCT Exif_Image_Make, Exif_Image_Model FROM exif"), m_db );
    if ( !query.exec() )
    {
        showErrorAndFail( query );
    } else {
        while ( query.next() ) {
            QString make = query.value(0).toString();
            QString model = query.value(1).toString();
            if ( !make.isEmpty() && !model.isEmpty() )
                result.append( qMakePair( make, model ) );
        }
    }

    return result;
}

QList< QString > Exif::Database::lenses() const
{
    QList< QString > result;

    if ( !isUsable() )
        return result;

    QSqlQuery query( QString::fromLatin1("SELECT DISTINCT Exif_Photo_LensModel FROM exif"), m_db );
    if ( !query.exec() )
    {
        showErrorAndFail( query );
    } else {
        while ( query.next() ) {
            QString lens = query.value(0).toString();
            if ( !lens.isEmpty() )
                result.append( lens );
        }
    }

    return result;
}

void Exif::Database::init()
{
    if ( !isAvailable() )
        return;

    m_isFailed = false;
    m_insertTransaction = nullptr;
    bool dbExists = QFile::exists( exifDBFile() );

    openDatabase();

    if ( !isOpen() )
        return;

    if ( !dbExists )
        populateDatabase();
    else
        updateDatabase();
}

void Exif::Database::recreate()
{
    // We create a backup of the current database in case
    // the user presse 'cancel' or there is any error. In that case
    // we want to go back to the original DB.

    const QString origBackup = exifDBFile() + QLatin1String(".bak");
    m_db.close();

    QDir().remove(origBackup);
    QDir().rename(exifDBFile(), origBackup);
    init();

    const DB::FileNameList allImages = DB::ImageDB::instance()->images();
    QProgressDialog dialog;
    dialog.setModal(true);
    dialog.setLabelText(i18n("Rereading Exif information from all images"));
    dialog.setMaximum(allImages.size());
    // using a transaction here removes a *huge* overhead on the insert statements
    startInsertTransaction();
    int i = 0;
    for (const DB::FileName& fileName : allImages) {
        const DB::ImageInfoPtr info = fileName.info();
        dialog.setValue(i++);
        if (info->mediaType() == DB::Image) {
            add(fileName);
        }
        if ( i % 10 )
            qApp->processEvents();
        if (dialog.wasCanceled())
            break;
    }

    // PENDING(blackie) We should count the amount of files that did not succeeded and warn the user.
    if (dialog.wasCanceled()) {
        abortInsertTransaction();
        m_db.close();
        QDir().remove(exifDBFile());
        QDir().rename(origBackup, exifDBFile());
        init();
    }
    else {
        commitInsertTransaction();
        QDir().remove(origBackup);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
