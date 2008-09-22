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

#define property( GET_TYPE,GET_FUNC,GET_VALUE,  SET_FUNC,SET_TYPE,SET_VALUE,  GROUP,OPTION,GET_DEFAULT_1,GET_DEFAULT_2,GET_DEFAULT_2_TYPE ) \
    GET_TYPE SettingsData::GET_FUNC() const                                              \
    {                                                                                    \
        KConfigGroup g = KGlobal::config()->group(GROUP);                                \
                                                                                         \
        if ( !g.hasKey(OPTION) )                                                         \
            return GET_DEFAULT_1;                                                        \
                                                                                         \
        GET_DEFAULT_2_TYPE v = g.readEntry<GET_DEFAULT_2_TYPE>( OPTION, GET_DEFAULT_2 ); \
        return GET_VALUE;                                                                \
    }                                                                                    \
    void SettingsData::SET_FUNC( const SET_TYPE v )                                      \
    {                                                                                    \
        KConfigGroup group = KGlobal::config()->group(GROUP);                            \
        group.writeEntry( OPTION, SET_VALUE );                                           \
        group.sync();                                                                    \
    }

#define property_copy( GET_FUNC,SET_FUNC, TYPE,GROUP,GET_DEFAULT ) \
    property( TYPE,GET_FUNC,v,  SET_FUNC,TYPE,v,  #GROUP,#GET_FUNC,GET_DEFAULT,GET_DEFAULT,TYPE )

#define property_ref( GET_FUNC,SET_FUNC, TYPE,GROUP,GET_DEFAULT ) \
    property( TYPE,GET_FUNC,v,  SET_FUNC,TYPE&,v,  #GROUP,#GET_FUNC,GET_DEFAULT,GET_DEFAULT,TYPE )

#define property_enum( GET_FUNC,SET_FUNC, TYPE,GROUP,GET_DEFAULT ) \
    property( TYPE,GET_FUNC,(TYPE)v,  SET_FUNC,TYPE,(int)v,  #GROUP,#GET_FUNC,GET_DEFAULT,GET_DEFAULT,int )

#define property_sset( GET_FUNC,SET_FUNC, GROUP,GET_DEFAULT ) \
    property( StringSet,GET_FUNC,v.toSet(),  SET_FUNC,StringSet&,v.toList(),  #GROUP,#GET_FUNC,GET_DEFAULT,QStringList(),QStringList )

/**
 * smoothScale() is called from the image loading thread, therefore we need
 * to cache it this way, rather than going to KConfig.
 */
static bool _smoothScale = true;

using namespace Settings;

const WindowType Settings::MainWindow   = "MainWindow";
const WindowType Settings::ConfigWindow = "ConfigWindow";

SettingsData* SettingsData::_instance = 0;

SettingsData* SettingsData::instance()
{
    if ( ! _instance )
        qFatal("instance called before loading a setup!");
    return _instance;
}

SettingsData::SettingsData( const QString& imageDirectory )
    : _hasAskedAboutTimeStamps( false ),
      _imageDirectory( imageDirectory )
{
    QPixmapCache::setCacheLimit( thumbnailCacheBytes() / 1024);
    _smoothScale = value( "Viewer", "smoothScale", true );
}

/////////////////
//// General ////
/////////////////

property_ref ( backend                               , setBackend                               ,  QString        , General , QString::fromLatin1("xml") );
property_copy( useEXIFRotate                         , setUseEXIFRotate                         ,  bool           , General , true                       );
property_copy( useEXIFComments                       , setUseEXIFComments                       ,  bool           , General , true                       );
property_copy( searchForImagesOnStartup              , setSearchForImagesOnStartup              ,  bool           , General , true                       );
property_copy( dontReadRawFilesWithOtherMatchingFile , setDontReadRawFilesWithOtherMatchingFile ,  bool           , General , false                      );
property_copy( useCompressedIndexXML                 , setUseCompressedIndexXML                 ,  bool           , General , false                      );
property_copy( compressBackup                        , setCompressBackup                        ,  bool           , General , true                       );
property_copy( showSplashScreen                      , setShowSplashScreen                      ,  bool           , General , true                       );
property_copy( autoSave                              , setAutoSave                              ,  int            , General , 5                          );
property_copy( backupCount                           , setBackupCount                           ,  int            , General , 5                          );
property_enum( tTimeStamps                           , setTTimeStamps                           ,  TimeStampTrust , General , Always                     );

////////////////////
//// Thumbnails ////
////////////////////

property_copy( displayLabels            , setDisplayLabels           ,  bool                  , Thumbnails , true       );
property_copy( displayCategories        , setDisplayCategories       ,  bool                  , Thumbnails , false      );
property_copy( autoShowThumbnailView    , setAutoShowThumbnailView   ,  bool                  , Thumbnails , 0          );
property_copy( showNewestThumbnailFirst , setShowNewestFirst         ,  bool                  , Thumbnails , false      );
property_copy( thumbnailDarkBackground  , setThumbnailDarkBackground ,  bool                  , Thumbnails , true       );
property_copy( thumbnailDisplayGrid     , setThumbnailDisplayGrid    ,  bool                  , Thumbnails , false      );
property_copy( previewSize              , setPreviewSize             ,  int                   , Thumbnails , 256        );
property_copy( thumbnailSpace           , setThumbnailSpace          ,  int                   , Thumbnails , 1          );
property_enum( thumbnailAspectRatio     , setThumbnailAspectRatio    ,  ThumbnailAspectRatio  , Thumbnails , Aspect_4_3 );

////////////////
//// Viewer ////
////////////////

property_ref ( viewerSize                , setViewerSize                ,  QSize            , Viewer , QSize(800,600) );
property_ref ( slideShowSize             , setSlideShowSize             ,  QSize            , Viewer , QSize(800,600) );
property_copy( launchViewerFullScreen    , setLaunchViewerFullScreen    ,  bool             , Viewer , false          );
property_copy( launchSlideShowFullScreen , setLaunchSlideShowFullScreen ,  bool             , Viewer , false          );
property_copy( showInfoBox               , setShowInfoBox               ,  bool             , Viewer , true           );
property_copy( showLabel                 , setShowLabel                 ,  bool             , Viewer , true           );
property_copy( showDescription           , setShowDescription           ,  bool             , Viewer , true           );
property_copy( showDate                  , setShowDate                  ,  bool             , Viewer , true           );
property_copy( showImageSize             , setShowImageSize             ,  bool             , Viewer , true           );
property_copy( showTime                  , setShowTime                  ,  bool             , Viewer , true           );
property_copy( showFilename              , setShowFilename              ,  bool             , Viewer , false          );
property_copy( showEXIF                  , setShowEXIF                  ,  bool             , Viewer , true           );
property_copy( slideShowInterval         , setSlideShowInterval         ,  int              , Viewer , 5              );
property_copy( viewerCacheSize           , setViewerCacheSize           ,  int              , Viewer , 25             );
property_copy( infoBoxWidth              , setInfoBoxWidth              ,  int              , Viewer , 400            );
property_copy( infoBoxHeight             , setInfoBoxHeight             ,  int              , Viewer , 300            );
property_enum( infoBoxPosition           , setInfoBoxPosition           ,  Position         , Viewer , Bottom         );
property_enum( viewerStandardSize        , setViewerStandardSize        ,  StandardViewSize , Viewer , FullSize       );

///////////////////////
//// Miscellaneous ////
///////////////////////

property_copy( delayLoadingPlugins, setDelayLoadingPlugins,  bool, Plug-ins, true  );

//////////////
//// EXIF ////
//////////////

#ifdef HAVE_EXIV2
    property_sset( exifForViewer , setExifForViewer ,           EXIF , StringSet()                            );
    property_sset( exifForDialog , setExifForDialog ,           EXIF , Exif::Info::instance()->standardKeys() );
    property_ref ( iptcCharset   , setIptcCharset   , QString , EXIF , QString::null                          );
#endif

bool SettingsData::smoothScale() const
{
    return _smoothScale;
}

void SettingsData::setSmoothScale( bool b )
{
    _smoothScale = b;
    setValue( "Viewer", "smoothScale", b );
}

bool SettingsData::trustTimeStamps()
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

QString SettingsData::imageDirectory() const
{
    if ( !_imageDirectory.endsWith( STR( "/" ) ) )
        return _imageDirectory + STR( "/" );
    else
        return _imageDirectory;
}

QString SettingsData::HTMLBaseDir() const
{
    return value( groupForDatabase( "HTML Settings" ), "baseDir",
                  QString::fromLocal8Bit(getenv("HOME")) + STR( "/public_html") );
}

void SettingsData::setHTMLBaseDir( const QString& dir )
{
    setValue( groupForDatabase( "HTML Settings" ), "baseDir", dir );
}

QString SettingsData::HTMLBaseURL() const
{
    return value( groupForDatabase( "HTML Settings" ), "baseUrl",  STR( "file://" ) + HTMLBaseDir() );
}

void SettingsData::setHTMLBaseURL( const QString& url )
{
    setValue( groupForDatabase( "HTML Settings" ), "baseUrl", url );
}

QString SettingsData::HTMLDestURL() const
{
    return value( groupForDatabase( "HTML Settings" ), "destUrl",  STR( "file://" ) + HTMLBaseDir() );
}

void SettingsData::setHTMLDestURL( const QString& url )
{
    setValue( groupForDatabase( "HTML Settings" ), "destUrl", url );
}


void SettingsData::setup( const QString& imageDirectory )
{
    _instance = new SettingsData( imageDirectory );
}

void SettingsData::setCurrentLock( const DB::ImageSearchInfo& info, bool exclude )
{
    info.saveLock();
    setValue( groupForDatabase( "Privacy Settings" ), "exclude", exclude );
}

DB::ImageSearchInfo SettingsData::currentLock() const
{
    return DB::ImageSearchInfo::loadLock();
}

void SettingsData::setLocked( bool lock, bool force )
{
    bool changed = ( lock != isLocked() );
    setValue( groupForDatabase( "Privacy Settings" ), "locked", lock );
    if (changed || force )
        emit locked( lock, lockExcludes() );
}

bool SettingsData::isLocked() const
{
    return value( groupForDatabase( "Privacy Settings" ), "locked", false );
}

bool SettingsData::lockExcludes() const
{
    return value( groupForDatabase( "Privacy Settings" ), "exclude", false );
}

void SettingsData::setPassword( const QString& passwd )
{
    setValue( groupForDatabase( "Privacy Settings" ), "password", passwd );
}

QString SettingsData::password() const
{
    return value( groupForDatabase( "Privacy Settings" ), "password", STR("") );
}

// PENDING(blackie) move this function to Category
QString SettingsData::fileForCategoryImage( const QString& category, QString member ) const
{
    QString dir = imageDirectory() + STR("CategoryImages" );
    member.replace( QChar::fromLatin1(' '), QChar::fromLatin1('_') );
    member.replace( QChar::fromLatin1('/'), QChar::fromLatin1('_') );
    QString fileName = dir + STR("/%1-%2.jpg").arg( category ).arg( member );
    return fileName;
}

// PENDING(blackie) move this function to Category
void SettingsData::setCategoryImage( const QString& category, QString member, const QImage& image )
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
QPixmap SettingsData::categoryImage( const QString& category, QString member, int size ) const
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

void SettingsData::setViewSortType( const ViewSortType tp )
{
    bool changed = ( viewSortType() != tp );
    setValue( "General", "viewSortType", (int) tp );
    if ( changed )
        emit viewSortTypeChanged( tp );
}

ViewSortType SettingsData::viewSortType() const
{
    return (ViewSortType) value( "General", "viewSortType", 0 );
}

void SettingsData::setFromDate( const QDate& date)
{
    if (date.isValid())
        setValue( "Miscellaneous", "fromDate", date.toString( Qt::ISODate ) );
}

QDate SettingsData::fromDate() const
{
    QString date = value( "Miscellaneous", "fromDate", STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year(), 1, 1 );
    else
        return QDate::fromString( date, Qt::ISODate );
}

void  SettingsData::setToDate( const QDate& date)
{
    if (date.isValid())
        setValue( "Miscellaneous", "toDate", date.toString( Qt::ISODate ) );
}

QDate SettingsData::toDate() const
{
    QString date = value( "Miscellaneous", "toDate", STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year()+1, 1, 1 );
    else
        return QDate::fromString( date, Qt::ISODate );
}

QString SettingsData::albumCategory() const
{
    QString category = value( "General", "albumCategory", STR("") );

    if ( !DB::ImageDB::instance()->categoryCollection()->categoryNames().contains( category ) )
    {
        category = DB::ImageDB::instance()->categoryCollection()->categoryNames()[0];
        const_cast<SettingsData*>(this)->setAlbumCategory( category );
    }

    return category;
}

void SettingsData::setAlbumCategory( const QString& category )
{
    setValue( "General", "albumCategory", category );
}

void SettingsData::setWindowGeometry( WindowType win, const QRect& geometry )
{
    KConfigGroup group = KGlobal::config()->group("Window Geometry");
    group.writeEntry( win, geometry );
    group.sync();
}

QRect SettingsData::windowGeometry( WindowType win ) const
{
    KSharedConfigPtr config = KGlobal::config();
    QRect rect( 0,0, 800, 600 );
    return config->group("Window Geometry").readEntry<QRect>( win, rect );
}

bool SettingsData::ready()
{
    return _instance != 0;
}

int SettingsData::value( const char* grp, const char* option, int defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    return config->group( grp ).readEntry<int>( option, defaultValue );
}

QString SettingsData::value( const char* grp, const char* option, const QString& defaultValue ) const
{
    return value( STR(grp), option, defaultValue);
}

QString SettingsData::value( const QString& grp, const char* option, const QString& defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    return config->group(grp).readEntry<QString>( option, defaultValue );
}

bool SettingsData::value( const char* grp, const char* option, bool defaultValue ) const
{
    return value( STR(grp), option, defaultValue);
}

bool SettingsData::value( const QString& grp, const char* option, bool defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    return config->group(grp).readEntry<bool>( option, defaultValue );
}

QSize SettingsData::value( const char* grp, const char* option, const QSize& defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    return config->group(grp).readEntry<QSize>( option, defaultValue );
}

StringSet SettingsData::value(const char* grp, const char* option, const StringSet& defaultValue ) const
{
    KSharedConfigPtr config = KGlobal::config();
    if ( !config->group(grp).hasKey( option ) )
        return defaultValue;
    return config->group(grp).readEntry<QStringList>( option, QStringList() ).toSet();
}

void SettingsData::setValue( const char* grp, const char* option, int value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value );
    group.sync();
}

void SettingsData::setValue( const char* grp, const char* option, const QString& value )
{
    setValue( STR(grp), option, value);
}

void SettingsData::setValue( const QString& grp, const char* option, const QString& value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value );
    group.sync();
}

void SettingsData::setValue( const QString&grp, const char* option, bool value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value );
    group.sync();
}

void SettingsData::setValue( const char* grp, const char* option, bool value )
{
    setValue( STR(grp), option, value);
}

void SettingsData::setValue( const char* grp, const char* option, const QSize& value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value );
    group.sync();
}

void SettingsData::setValue( const char* grp, const char* option, const StringSet& value )
{
    KConfigGroup group = KGlobal::config()->group(grp);
    group.writeEntry( option, value.toList() );
    group.sync();
}

QSize SettingsData::histogramSize() const
{
    return value( "General", "histogramSize", QSize( 15, 30 ) );
}

void SettingsData::setHistogramSize( const QSize& size )
{
    bool changed = (size != histogramSize() );
    setValue( "General", "histogramSize", size );
    if (changed)
        emit histogramSizeChanged( size );
}

QString SettingsData::groupForDatabase( const char* setting ) const
{
    return STR("%1 - %2").arg( setting ).arg( imageDirectory() );
}


size_t SettingsData::thumbnailBytesForScreens(int screens) {
    const QRect screen = QApplication::desktop()->screenGeometry();
    const size_t kBytesPerPixel = 4;
    return kBytesPerPixel * screen.width() * screen.height() * screens;
}

void SettingsData::setThumbnailCacheScreens( int screens )
{
    setValue( "Thumbnails", "thumbnailCacheScreens", screens );
    QPixmapCache::setCacheLimit( thumbnailCacheBytes() / 1024);
    QPixmapCache::clear();
}

int SettingsData::thumbnailCacheScreens() const
{
    // Three pages sounds good; one before, one after the current screen
    return value( "Thumbnails", "thumbnailCacheScreens", 3);
}

size_t SettingsData::thumbnailCacheBytes() const {
    return thumbnailBytesForScreens(thumbnailCacheScreens());
}

void SettingsData::setThumbSize( int value )
{
    QPixmapCache::clear();
    setValue( "Thumbnails", "thumbSize", value );
}
int SettingsData::thumbSize() const
{
    return value( "Thumbnails", "thumbSize", 128 );
}

#ifdef SQLDB_SUPPORT
void SettingsData::setSQLParameters(const SQLDB::DatabaseAddress& address)
{
    KConfigGroup config = KGlobal::config()->group(QString::fromLatin1("SQLDB"));
    SQLDB::writeConnectionParameters(address, config);
    config.sync();
}

SQLDB::DatabaseAddress SettingsData::SQLParameters() const
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
