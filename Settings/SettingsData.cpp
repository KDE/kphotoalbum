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

#include "SettingsData.h"
#include "SettingsData.moc"

#include <stdlib.h>

#include <QApplication>
#include <QColor>
#include <QDesktopWidget>
#include <QDir>
#include <QPixmap> //Added by qt3to4
#include <QPixmapCache>
#include <QStringList>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "DB/MemberMap.h"
#include "Utilities/Util.h"

#include "config-kpa-sqldb.h"
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
    : _hasAskedAboutTimeStamps( false ),
      _imageDirectory( imageDirectory )
{
    QPixmapCache::setCacheLimit( thumbnailCacheBytes() / 1024);
    _smoothScale = value( "Viewer", "smoothScale", true );
}

bool Settings::SettingsData::smoothScale()
{
    return _smoothScale;
}

void Settings::SettingsData::setSmoothScale( bool b )
{
    _smoothScale = b;
    setValue( "Viewer", "smoothScale", b );
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
            QString txt = i18n("When reading time information of images, their EXIF info is used. "
                               "Exif info may, however, not be supported by your KPhotoAlbum installation, "
                               "or no valid information may be in the file. "
                               "As a backup, KPhotoAlbum may use the timestamp of the image - this may, "
                               "however, not be valid in case the image is scanned in. "
                               "So the question is, should KPhotoAlbum trust the time stamp on your images?" );
            int answer = KMessageBox::questionYesNo( 0, txt, i18n("Trust Time Stamps?") );
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
    setValue( "General", "trustTimeStamps", (int) t );
}
Settings::TimeStampTrust Settings::SettingsData::tTimeStamps() const
{
    return (TimeStampTrust) value( "General", "trustTimeStamps", (int) Always );
}

void Settings::SettingsData::setThumbnailAspectRatio( Settings::ThumbnailAspectRatio aspect )
{
    setValue( "Thumbnails", "thumbnailAspectRatio", (int) aspect);
}
Settings::ThumbnailAspectRatio Settings::SettingsData::thumbnailAspectRatio() const
{
    return (ThumbnailAspectRatio) value( "Thumbnails", "thumbnailAspectRatio", (int) Aspect_4_3);
}

void Settings::SettingsData::setViewerStandardSize(StandardViewSize v)
{
    setValue( "Viewer", "standardViewerSize", (int) v );
}

Settings::StandardViewSize Settings::SettingsData::viewerStandardSize() const
{
    return (StandardViewSize) value( "Viewer", "standardViewerSize", (int) FullSize );
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
    return (Position) value( "Viewer", "infoBoxPosition", 0 );
}

void Settings::SettingsData::setInfoBoxPosition( Position pos )
{
    setValue( "Viewer", "infoBoxPosition", (int) pos );
}

QString Settings::SettingsData::HTMLBaseDir() const
{
    return value( groupForDatabase( "HTML Settings" ), "baseDir",
                  QString::fromLocal8Bit(getenv("HOME")) + STR( "/public_html") );
}

void Settings::SettingsData::setHTMLBaseDir( const QString& dir )
{
    setValue( groupForDatabase( "HTML Settings" ), "baseDir", dir );
}

QString Settings::SettingsData::HTMLBaseURL() const
{
    return value( groupForDatabase( "HTML Settings" ), "baseUrl",  STR( "file://" ) + HTMLBaseDir() );
}

void Settings::SettingsData::setHTMLBaseURL( const QString& url )
{
    setValue( groupForDatabase( "HTML Settings" ), "baseUrl", url );
}

QString Settings::SettingsData::HTMLDestURL() const
{
    return value( groupForDatabase( "HTML Settings" ), "destUrl",  STR( "file://" ) + HTMLBaseDir() );
}

void Settings::SettingsData::setHTMLDestURL( const QString& url )
{
    setValue( groupForDatabase( "HTML Settings" ), "destUrl", url );
}


void Settings::SettingsData::setup( const QString& imageDirectory )
{
    _instance = new SettingsData( imageDirectory );
}

void Settings::SettingsData::setCurrentLock( const DB::ImageSearchInfo& info, bool exclude )
{
    info.saveLock();
    setValue( groupForDatabase( "Privacy Settings" ), "exclude", exclude );
}

DB::ImageSearchInfo Settings::SettingsData::currentLock() const
{
    return DB::ImageSearchInfo::loadLock();
}

void Settings::SettingsData::setLocked( bool lock, bool force )
{
    bool changed = ( lock != isLocked() );
    setValue( groupForDatabase( "Privacy Settings" ), "locked", lock );
    if (changed || force )
        emit locked( lock, lockExcludes() );
}

bool Settings::SettingsData::isLocked() const
{
    return value( groupForDatabase( "Privacy Settings" ), "locked", false );
}

bool Settings::SettingsData::lockExcludes() const
{
    return value( groupForDatabase( "Privacy Settings" ), "exclude", false );
}

void Settings::SettingsData::setPassword( const QString& passwd )
{
    setValue( groupForDatabase( "Privacy Settings" ), "password", passwd );
}

QString Settings::SettingsData::password() const
{
    return value( groupForDatabase( "Privacy Settings" ), "password", STR("") );
}

// PENDING(blackie) move this function to Category
QString Settings::SettingsData::fileForCategoryImage( const QString& category, QString member ) const
{
    QString dir = imageDirectory() + STR("CategoryImages" );
    member.replace( QChar::fromLatin1(' '), QChar::fromLatin1('_') );
    member.replace( QChar::fromLatin1('/'), QChar::fromLatin1('_') );
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
            KMessageBox::error( 0, i18n("Unable to create directory '%1'.", dir ), i18n("Unable to Create Directory") );
            return;
        }
    }
    QString fileName = fileForCategoryImage( category, member );
    ok = image.save( fileName, "JPEG" );
    if ( !ok ) {
        KMessageBox::error( 0, i18n("Error when saving image '%1'.",fileName), i18n("Error Saving Image") );
        return;
    }

    // PENDING(blackie) HACK ALERT: Remove all images rather than just these resolutions.
    QString key = STR( "64-%2" ).arg(fileName);
    QPixmapCache::remove( key );

    key = STR( "128-%2" ).arg(fileName);
    QPixmapCache::remove( key );
}

// PENDING(blackie) moved this function to Category
QPixmap Settings::SettingsData::categoryImage( const QString& category, QString member, int size ) const
{
    QString fileName = fileForCategoryImage( category, member );
    QString key = STR( "%1-%2" ).arg(size).arg(fileName);
    QPixmap res;
    if ( QPixmapCache::find( key, res ) )
        return res;

    QImage img;
    bool ok = img.load( fileName, "JPEG" );
    if ( ! ok ) {
        if ( DB::ImageDB::instance()->memberMap().isGroup( category, member ) )
            img = KIconLoader::global()->loadIcon( STR( "kuser" ), KIconLoader::Desktop, size ).toImage();
        else
            img = DB::ImageDB::instance()->categoryCollection()->categoryForName( category )->icon().toImage();
    }
    res = QPixmap::fromImage( Utilities::scaleImage(img, size, size, Qt::KeepAspectRatio) );

    QPixmapCache::insert( key, res );
    return res;
}

void Settings::SettingsData::setViewSortType( ViewSortType tp )
{
    bool changed = ( viewSortType() != tp );
    setValue( "General", "viewSortType", (int) tp );
    if ( changed )
        emit viewSortTypeChanged( tp );
}

ViewSortType Settings::SettingsData::viewSortType() const
{
    return (ViewSortType) value( "General", "viewSortType", 0 );
}

void Settings::SettingsData::setFromDate( const QDate& date)
{
    if (date.isValid())
        setValue( "Miscellaneous", "fromDate", date.toString( Qt::ISODate ) );
}

QDate Settings::SettingsData::fromDate() const
{
    QString date = value( "Miscellaneous", "fromDate", STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year(), 1, 1 );
    else
        return QDate::fromString( date, Qt::ISODate );
}

void  Settings::SettingsData::setToDate( const QDate& date)
{
    if (date.isValid())
        setValue( "Miscellaneous", "toDate", date.toString( Qt::ISODate ) );
}

QDate Settings::SettingsData::toDate() const
{
    QString date = value( "Miscellaneous", "toDate", STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year()+1, 1, 1 );
    else
        return QDate::fromString( date, Qt::ISODate );
}

QString Settings::SettingsData::albumCategory() const
{
    QString category = value( "General", "albumCategory", STR("") );

    if ( !DB::ImageDB::instance()->categoryCollection()->categoryNames().contains( category ) ) {
        category = DB::ImageDB::instance()->categoryCollection()->categoryNames()[0];
        const_cast<SettingsData*>(this)->setAlbumCategory( category );
    }

    return category;
}

void Settings::SettingsData::setAlbumCategory( const QString& category )
{
    setValue( "General", "albumCategory", category );
}

void Settings::SettingsData::setWindowGeometry( WindowType win, const QRect& geometry )
{
    KConfigGroup group = KGlobal::config()->group("Window Geometry");
    group.writeEntry( windowTypeToString( win ), geometry );
    group.sync();
}

QRect Settings::SettingsData::windowGeometry( WindowType win ) const
{
    KSharedConfigPtr config = KGlobal::config();
    QRect rect( 0,0, 800, 600 );
    return config->group("Window Geometry").readEntry<QRect>( windowTypeToString( win ), rect );
}

bool Settings::SettingsData::ready()
{
    return _instance != 0;
}

int Settings::SettingsData::value( const char* grp, const char* option, int defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    return config->group( grp ).readEntry<int>( option, defaultValue );
}

QString Settings::SettingsData::value( const char* grp, const char* option, const QString& defaultValue ) const
{
    return value( STR(grp), option, defaultValue);
}

QString Settings::SettingsData::value( const QString& grp, const char* option, const QString& defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    return config->group(grp).readEntry<QString>( option, defaultValue );
}

bool Settings::SettingsData::value( const char* grp, const char* option, bool defaultValue ) const
{
    return value( STR(grp), option, defaultValue);
}

bool Settings::SettingsData::value( const QString& grp, const char* option, bool defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    return config->group(grp).readEntry<bool>( option, defaultValue );
}

QColor Settings::SettingsData::value( const char* grp, const char* option, const QColor& defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    return config->group(grp).readEntry<QColor>( option, defaultValue );
}

QSize Settings::SettingsData::value( const char* grp, const char* option, const QSize& defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    return config->group(grp).readEntry<QSize>( option, defaultValue );
}

StringSet Settings::SettingsData::value(const char* grp, const char* option, const StringSet& defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    if ( !config->group(grp).hasKey( option ) )
        return defaultValue;
    return config->group(grp).readEntry<QStringList>( option, QStringList() ).toSet();
}

void Settings::SettingsData::setValue( const char* grp, const char* option, int value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value );
    group.sync();
}

void Settings::SettingsData::setValue( const char* grp, const char* option, const QString& value )
{
    setValue( STR(grp), option, value);
}

void Settings::SettingsData::setValue( const QString& grp, const char* option, const QString& value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value );
    group.sync();
}

void Settings::SettingsData::setValue( const QString&grp, const char* option, bool value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value );
    group.sync();
}

void Settings::SettingsData::setValue( const char* grp, const char* option, bool value )
{
    setValue( STR(grp), option, value);
}

void Settings::SettingsData::setValue( const char* grp, const char* option, const QColor& value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value );
    group.sync();
}

void Settings::SettingsData::setValue( const char* grp, const char* option, const QSize& value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value );
    group.sync();
}

void Settings::SettingsData::setValue( const char* grp, const char* option, const StringSet& value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value.toList() );
    group.sync();
}

QSize Settings::SettingsData::histogramSize() const
{
    return value( "General", "histogramSize", QSize( 15, 30 ) );
}

void Settings::SettingsData::setHistogramSize( const QSize& size )
{
    bool changed = (size != histogramSize() );
    setValue( "General", "histogramSize", size );
    if (changed)
        emit histogramSizeChanged( size );
}

const char* Settings::SettingsData::windowTypeToString( WindowType tp ) const
{
    switch (tp) {
    case MainWindow:   return "MainWindow";
    case ConfigWindow: return "ConfigWindow";
    }
    return "";
}

QString Settings::SettingsData::groupForDatabase( const char* setting ) const
{
    return STR("%1 - %2").arg( setting ).arg( imageDirectory() );
}


size_t Settings::SettingsData::thumbnailBytesForScreens(int screens) {
    const QRect screen = QApplication::desktop()->screenGeometry();
    const size_t kBytesPerPixel = 4;
    return kBytesPerPixel * screen.width() * screen.height() * screens;
}

void Settings::SettingsData::setThumbnailCacheScreens( int screens )
{
    setValue( "Thumbnails", "thumbnailCacheScreens", screens );
    QPixmapCache::setCacheLimit( thumbnailCacheBytes() / 1024);
    QPixmapCache::clear();
}

int Settings::SettingsData::thumbnailCacheScreens() const
{
    // Three pages sounds good; one before, one after the current screen
    return value( "Thumbnails", "thumbnailCacheScreens", 3);
}

size_t Settings::SettingsData::thumbnailCacheBytes() const {
    return thumbnailBytesForScreens(thumbnailCacheScreens());
}

void Settings::SettingsData::setThumbSize( int value )
{
    QPixmapCache::clear();
    setValue( "Thumbnails", "thumbSize", value );
}
int Settings::SettingsData::thumbSize() const
{
    return value( "Thumbnails", "thumbSize", 128 );
}

#ifdef SQLDB_SUPPORT
void Settings::SettingsData::setSQLParameters(const SQLDB::DatabaseAddress& address)
{
    KConfigGroup config = KGlobal::config()->group(QString::fromLatin1("SQLDB"));
    SQLDB::writeConnectionParameters(address, config);
    config.sync();
}

SQLDB::DatabaseAddress Settings::SettingsData::getSQLParameters() const
{
    KConfigGroup config = KGlobal::config()->group(QString::fromLatin1("SQLDB"));
    try {
        return SQLDB::readConnectionParameters(config);
    }
    catch (SQLDB::DriverNotFoundError&) {}
    return SQLDB::DatabaseAddress();
}
#endif /* SQLDB_SUPPORT */

#undef STR
