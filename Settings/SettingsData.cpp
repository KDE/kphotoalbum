/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "SettingsData.h"
#include <qdir.h>
#include "Utilities/Util.h"
#include <stdlib.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qapplication.h>
#include <qcursor.h>
#include <kiconloader.h>
#include <kglobal.h>
#include "DB/ImageDB.h"
#include <qmessagebox.h>
#include "DB/CategoryCollection.h"
#include <qdatetime.h>
#include "DB/ImageInfo.h"
#include <kapplication.h>
#include <kconfig.h>
#include "SettingsData.moc"
#include "DB/MemberMap.h"
#include <qpixmapcache.h>
#ifdef SQLDB_SUPPORT
#  include "SQLDB/ConfigFileHandler.h"
#  include "SQLDB/DatabaseAddress.h"
#  include "SQLDB/QueryErrors.h"
#endif

#define STR(x) QString::fromLatin1(x)

/**
 * smoothScale() is called from the image loading thread, therefore we need
 * to cache it this way, rather than going to KConfig.
 */
static bool _smoothScale = true;

using namespace Settings;

Settings::SettingsData* Settings::SettingsData::_instance = 0;

Settings::SettingsData* Settings::SettingsData::instance()
{
    if ( ! _instance )
        qFatal("instance called before loading a setup!");
    return _instance;
}

Settings::SettingsData::SettingsData( const QString& imageDirectory )
    : _hasAskedAboutTimeStamps( false ), _imageDirectory( imageDirectory )
{
    QPixmapCache::setCacheLimit( thumbnailCache() * 1024 );
    _smoothScale = value( QString::fromLatin1( "Viewer" ), QString::fromLatin1( "smoothScale" ), true );
}

bool Settings::SettingsData::smoothScale()
{
    return _smoothScale;
}

void Settings::SettingsData::setSmoothScale( bool b )
{
    _smoothScale = b;
    setValue( QString::fromLatin1( "Viewer" ), QString::fromLatin1( "smoothScale" ), b );
}

bool Settings::SettingsData::trustTimeStamps()
{
    if ( tTimeStamps() == Always )
        return true;
    else if ( tTimeStamps() == Never )
        return false;
    else {
        if (!_hasAskedAboutTimeStamps ) {
            QApplication::setOverrideCursor( Qt::ArrowCursor );
            int answer = KMessageBox::questionYesNo( 0, i18n("New images were found. Should I trust their time stamps?"),
                                                     i18n("Trust Time Stamps?") );
            QApplication::restoreOverrideCursor();
            if ( answer == KMessageBox::Yes )
                _trustTimeStamps = true;
            else
                _trustTimeStamps = false;
            _hasAskedAboutTimeStamps = true;
        }
        return _trustTimeStamps;
    }
}
void Settings::SettingsData::setTTimeStamps( TimeStampTrust t )
{
    setValue( STR("General"), STR("trustTimeStamps"), (int) t );
}
Settings::TimeStampTrust Settings::SettingsData::tTimeStamps() const
{
    return (TimeStampTrust) value(  STR("General"), STR("trustTimeStamps"), (int) Always );
}

void Settings::SettingsData::setViewerStandardSize(StandardViewSize v)
{
    setValue( STR("Viewer"), STR("standardViewerSize"), (int) v );
}

Settings::StandardViewSize Settings::SettingsData::viewerStandardSize() const
{
    return (StandardViewSize) value(  STR("Viewer"), STR("standardViewerSize"), (int) FullSize );
}

QString Settings::SettingsData::imageDirectory() const
{
    if ( !_imageDirectory.endsWith( STR( "/" ) ) )
        return _imageDirectory + STR( "/" );
    else
        return _imageDirectory;
}


Settings::Position Settings::SettingsData::infoBoxPosition() const
{
    return (Position) value( STR("Viewer"), STR("infoBoxPosition"), 0 );
}

void Settings::SettingsData::setInfoBoxPosition( Position pos )
{
    setValue( STR("Viewer"), STR("infoBoxPosition"), (int) pos );
}

QString Settings::SettingsData::HTMLBaseDir() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("baseDir"), QString::fromLocal8Bit(getenv("HOME")) + STR( "/public_html") );
}

void Settings::SettingsData::setHTMLBaseDir( const QString& dir )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("baseDir"), dir );
}

QString Settings::SettingsData::HTMLBaseURL() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("baseUrl"),  STR( "file://" ) + HTMLBaseDir() );
}

void Settings::SettingsData::setHTMLBaseURL( const QString& url )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("baseUrl"), url );
}

QString Settings::SettingsData::HTMLDestURL() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("destUrl"),  STR( "file://" ) + HTMLBaseDir() );
}

void Settings::SettingsData::setHTMLDestURL( const QString& url )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("destUrl"), url );
}


void Settings::SettingsData::setup( const QString& imageDirectory )
{
    _instance = new SettingsData( imageDirectory );
}

void Settings::SettingsData::setCurrentLock( const DB::ImageSearchInfo& info, bool exclude )
{
    info.saveLock();
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("exclude"), exclude );
}

DB::ImageSearchInfo Settings::SettingsData::currentLock() const
{
    return DB::ImageSearchInfo::loadLock();
}

void Settings::SettingsData::setLocked( bool lock )
{
    bool changed = ( lock != isLocked() );
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("locked"), lock );
    if (changed)
        emit locked( lock, lockExcludes() );
}

bool Settings::SettingsData::isLocked() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("locked"), false );
}

bool Settings::SettingsData::lockExcludes() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("exclude"), false );
}

void Settings::SettingsData::setPassword( const QString& passwd )
{
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("password"), passwd );
}

QString Settings::SettingsData::password() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("password"), STR("") );
}

// PENDING(blackie) move this function to Category
QString Settings::SettingsData::fileForCategoryImage( const QString& category, QString member ) const
{
    QString dir = imageDirectory() + STR("CategoryImages" );
    member.replace( ' ', '_' );
    QString fileName = dir + STR("/%1-%2.jpg").arg( category ).arg( member );
    return fileName;
}

// PENDING(blackie) move this function to Category
void Settings::SettingsData::setCategoryImage( const QString& category, QString member, const QImage& image )
{
    QString dir = imageDirectory() + STR("CategoryImages" );
    QFileInfo fi( dir );
    bool ok;
    if ( !fi.exists() ) {
        bool ok = QDir().mkdir( dir );
        if ( !ok ) {
            QMessageBox::warning( 0, i18n("Unable to Create Directory"), i18n("Unable to create directory '%1'.").arg( dir ), QMessageBox::Ok, 0 );
            return;
        }
    }
    QString fileName = fileForCategoryImage( category, member );
    ok = image.save( fileName, "JPEG" );
    if ( !ok ) {
        QMessageBox::warning( 0, i18n("Error Saving Image"), i18n("Error when saving image '%1'.").arg(fileName), QMessageBox::Ok, 0 );
        return;
    }

    // PENDING(blackie) HACK ALERT: Remove all images rather than just these resolutions.
    QString key = QString::fromLatin1( "64-%2" ).arg(fileName);
    QPixmapCache::remove( key );

    key = QString::fromLatin1( "128-%2" ).arg(fileName);
    QPixmapCache::remove( key );
}

// PENDING(blackie) moved this function to Category
QPixmap Settings::SettingsData::categoryImage( const QString& category, QString member, int size ) const
{
    QString fileName = fileForCategoryImage( category, member );
    QString key = QString::fromLatin1( "%1-%2" ).arg(size).arg(fileName);
    QPixmap res;
    if ( QPixmapCache::find( key, res ) )
        return res;

    QImage img;
    bool ok = img.load( fileName, "JPEG" );
    if ( ! ok ) {
        if ( DB::ImageDB::instance()->memberMap().isGroup( category, member ) )
            img = KGlobal::iconLoader()->loadIcon( STR( "kuser" ), KIcon::Desktop, size );
        else
            img = DB::ImageDB::instance()->categoryCollection()->categoryForName( category )->icon( size );
    }
    res = Utilities::scaleImage(img, size, size, QImage::ScaleMin);

    QPixmapCache::insert( key, res );
    return res;
}

void Settings::SettingsData::setViewSortType( ViewSortType tp )
{
    bool changed = ( viewSortType() != tp );
    setValue( STR("General"), STR("viewSortType"), (int) tp );
    if ( changed )
        emit viewSortTypeChanged( tp );
}

ViewSortType Settings::SettingsData::viewSortType() const
{
    return (ViewSortType) value( STR("General"), STR("viewSortType"), 0 );
}

void Settings::SettingsData::setFromDate( const QDate& date)
{
    if (date.isValid())
        setValue( STR("Miscellaneous"), STR("fromDate"), date.toString( Qt::ISODate ) );
}

QDate Settings::SettingsData::fromDate() const
{
    QString date = value( STR("Miscellaneous"), STR("fromDate"), STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year(), 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

void  Settings::SettingsData::setToDate( const QDate& date)
{
    if (date.isValid())
        setValue( STR("Miscellaneous"), STR("toDate"), date.toString( Qt::ISODate ) );
}

QDate Settings::SettingsData::toDate() const
{
    QString date = value( STR("Miscellaneous"), STR("toDate"), STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year()+1, 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

QString Settings::SettingsData::albumCategory() const
{
    QString category = value( STR("General"), STR("albumCategory"), STR("") );

    if ( !DB::ImageDB::instance()->categoryCollection()->categoryNames().contains( category ) ) {
        category = DB::ImageDB::instance()->categoryCollection()->categoryNames()[0];
        const_cast<SettingsData*>(this)->setAlbumCategory( category );
    }

    return category;
}

void Settings::SettingsData::setAlbumCategory( const QString& category )
{
    setValue( STR("General"), STR("albumCategory"), category );
}

void Settings::SettingsData::setWindowGeometry( WindowType win, const QRect& geometry )
{
    KConfig* config = kapp->config();
    config->setGroup( "Window Geometry" );
    config->writeEntry( windowTypeToString( win ), geometry );
}

QRect Settings::SettingsData::windowGeometry( WindowType win ) const
{
    KConfig* config = kapp->config();
    config->setGroup( "Window Geometry" );
    QRect rect( 0,0, 800, 600 );
    return config->readRectEntry( windowTypeToString( win ), &rect );
}

bool Settings::SettingsData::ready()
{
    return _instance != 0;
}

int Settings::SettingsData::value( const QString& group, const QString& option, int defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readNumEntry( option, defaultValue );
}

QString Settings::SettingsData::value( const QString& group, const QString& option, const QString& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readEntry( option, defaultValue );
}

bool Settings::SettingsData::value( const QString& group, const QString& option, bool defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readBoolEntry( option, defaultValue );
}

QColor Settings::SettingsData::value( const QString& group, const QString& option, const QColor& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readColorEntry( option, &defaultValue );
}

QSize Settings::SettingsData::value( const QString& group, const QString& option, const QSize& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readSizeEntry( option, &defaultValue );
}

Set<QString> Settings::SettingsData::value(const QString& group, const QString& option, const Set<QString>& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readListEntry( option.latin1(), QStringList( defaultValue.toList() ) );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, int value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, const QString& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, bool value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, const QColor& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, const QSize& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, const Set<QString>& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value.toList() );
}

QSize Settings::SettingsData::histogramSize() const
{
    return value( STR("General"), STR("histogramSize"), QSize( 15, 30 ) );
}

void Settings::SettingsData::setHistogramSize( const QSize& size )
{
    bool changed = (size != histogramSize() );
    setValue( STR("General"), STR("histogramSize"), size );
    if (changed)
        emit histogramSizeChanged( size );
}

QString Settings::SettingsData::windowTypeToString( WindowType tp ) const
{
    switch (tp) {
    case MainWindow: return STR("MainWindow");
    case ConfigWindow: return STR("ConfigWindow");
    }
    return STR("");
}

QString Settings::SettingsData::groupForDatabase( const QString& setting ) const
{
    return STR("%1 - %2").arg( setting ).arg( imageDirectory() );
}



void Settings::SettingsData::setThumbnailCache( int value )
{
    QPixmapCache::setCacheLimit( thumbnailCache() * 1024 );
    QPixmapCache::clear();
    setValue( QString::fromLatin1( "Thumbnails" ), QString::fromLatin1( "thumbnailCache" ), value );
}
int Settings::SettingsData::thumbnailCache() const
{
    return value( QString::fromLatin1( "Thumbnails" ), QString::fromLatin1( "thumbnailCache" ), 5 );
}

void Settings::SettingsData::setThumbSize( int value )
{
    QPixmapCache::clear();
    setValue( QString::fromLatin1( "Thumbnails" ), QString::fromLatin1( "thumbSize" ), value );
}
int Settings::SettingsData::thumbSize() const
{
    return value( QString::fromLatin1( "Thumbnails" ), QString::fromLatin1( "thumbSize" ), 128 );
}

#ifdef SQLDB_SUPPORT
void Settings::SettingsData::setSQLParameters(const SQLDB::DatabaseAddress& address)
{
    KConfig* config = kapp->config();
    config->setGroup(QString::fromLatin1("SQLDB"));
    SQLDB::writeConnectionParameters(address, *config);
}

SQLDB::DatabaseAddress Settings::SettingsData::getSQLParameters() const
{
    KConfig* config = kapp->config();
    config->setGroup(QString::fromLatin1("SQLDB"));
    try {
        return SQLDB::readConnectionParameters(*config);
    }
    catch (SQLDB::DriverNotFoundError&) {}
    return SQLDB::DatabaseAddress();
}
#endif /* SQLDB_SUPPORT */

#undef STR
