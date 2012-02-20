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

#include "SettingsData.h"
#include "SettingsData.moc"

#include <stdlib.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
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

#define value( GROUP, OPTION, DEFAULT )                            \
    KGlobal::config()->group( GROUP ).readEntry( OPTION, DEFAULT ) \

#define setValue( GROUP, OPTION, VALUE )                    \
{                                                           \
    KConfigGroup group = KGlobal::config()->group( GROUP ); \
    group.writeEntry( OPTION, VALUE );                      \
    group.sync();                                           \
}

#define getValueFunc_( TYPE,FUNC,  GROUP,OPTION,DEFAULT ) \
    TYPE SettingsData::FUNC() const                       \
    { return (TYPE) value( GROUP, OPTION, DEFAULT ); }    \

#define setValueFunc_( FUNC,TYPE,  GROUP,OPTION,VALUE ) \
    void SettingsData::FUNC( const TYPE v )             \
    { setValue( GROUP, OPTION, VALUE ); }               \

#define getValueFunc( TYPE,FUNC,  GROUP,DEFAULT ) getValueFunc_( TYPE,FUNC,  #GROUP,#FUNC,DEFAULT )
#define setValueFunc( FUNC,TYPE,  GROUP,OPTION  ) setValueFunc_( FUNC,TYPE,  #GROUP,#OPTION,v       )

// TODO(mfwitten): document parameters.
#define property_( GET_TYPE,GET_FUNC,GET_VALUE,  SET_FUNC,SET_TYPE,SET_VALUE,  GROUP,OPTION,GET_DEFAULT_1,GET_DEFAULT_2,GET_DEFAULT_2_TYPE ) \
    GET_TYPE SettingsData::GET_FUNC() const                                              \
    {                                                                                    \
        KConfigGroup g = KGlobal::config()->group(GROUP);                                \
                                                                                         \
        if ( !g.hasKey(OPTION) )                                                         \
            return GET_DEFAULT_1;                                                        \
                                                                                         \
        GET_DEFAULT_2_TYPE v = g.readEntry( OPTION, (GET_DEFAULT_2_TYPE)GET_DEFAULT_2 ); \
        return (GET_TYPE) GET_VALUE;                                                     \
    }                                                                                    \
    setValueFunc_( SET_FUNC,SET_TYPE, GROUP,OPTION,SET_VALUE )                           \

#define property( GET_TYPE,GET_FUNC,  SET_FUNC,SET_TYPE,SET_VALUE,  GROUP,OPTION,GET_DEFAULT ) \
    getValueFunc_( GET_TYPE,GET_FUNC, GROUP,OPTION,GET_DEFAULT)                                \
    setValueFunc_( SET_FUNC,SET_TYPE, GROUP,OPTION,SET_VALUE  )

#define property_copy( GET_FUNC,SET_FUNC, TYPE,GROUP,GET_DEFAULT ) \
    property( TYPE,GET_FUNC,  SET_FUNC,TYPE,v,  #GROUP,#GET_FUNC,GET_DEFAULT )

#define property_ref_( GET_FUNC,SET_FUNC, TYPE,GROUP,GET_DEFAULT ) \
    property( TYPE,GET_FUNC,  SET_FUNC,TYPE&,v,  GROUP,#GET_FUNC,GET_DEFAULT )

#define property_ref( GET_FUNC,SET_FUNC, TYPE,GROUP,GET_DEFAULT ) \
    property( TYPE,GET_FUNC,  SET_FUNC,TYPE&,v,  #GROUP,#GET_FUNC,GET_DEFAULT )

#define property_enum( GET_FUNC,SET_FUNC, TYPE,GROUP,GET_DEFAULT ) \
    property( TYPE,GET_FUNC,  SET_FUNC,TYPE,(int)v,  #GROUP,#GET_FUNC,(int)GET_DEFAULT )

#define property_sset( GET_FUNC,SET_FUNC, GROUP,GET_DEFAULT ) \
    property_( StringSet,GET_FUNC,v.toSet(),  SET_FUNC,StringSet&,v.toList(),  #GROUP,#GET_FUNC,GET_DEFAULT,QStringList(),QStringList )

/**
 * smoothScale() is called from the image loading thread, therefore we need
 * to cache it this way, rather than going to KConfig.
 */
static bool _smoothScale = true;

using namespace Settings;

const WindowType Settings::MainWindow       = "MainWindow";
const WindowType Settings::AnnotationDialog = "AnnotationDialog";

SettingsData* SettingsData::_instance = 0;

SettingsData* SettingsData::instance()
{
    if ( ! _instance )
        qFatal("instance called before loading a setup!");

    return _instance;
}

bool SettingsData::ready()
{
    return _instance;
}

void SettingsData::setup( const QString& imageDirectory )
{
    if ( !_instance )
        _instance = new SettingsData( imageDirectory );
}

SettingsData::SettingsData( const QString& imageDirectory )
{
    _hasAskedAboutTimeStamps = false;

    QString s = STR( "/" );
    _imageDirectory = imageDirectory.endsWith(s) ? imageDirectory : imageDirectory + s;

    _smoothScale = value( "Viewer", "smoothScale", true );
}

/////////////////
//// General ////
/////////////////

property_ref ( backend               , setBackend               , QString       , General, QString::fromLatin1("xml") )
property_copy( useEXIFRotate         , setUseEXIFRotate         , bool          , General, true                       )
property_copy( useEXIFComments       , setUseEXIFComments       , bool          , General, true                       )
property_copy( searchForImagesOnStart, setSearchForImagesOnStart, bool          , General, true                       )
property_copy( ignoreFileExtension   , setIgnoreFileExtension   , bool          , General, false                      )
property_copy( skipSymlinks,           setSkipSymlinks          , bool          , General, false                      )
property_copy( skipRawIfOtherMatches , setSkipRawIfOtherMatches , bool          , General, false                      )
property_copy( useRawThumbnail       , setUseRawThumbnail       , bool          , General, false                      )
property_copy( useRawThumbnailSize   , setUseRawThumbnailSize   , QSize         , General, QSize(1024,768)            )
property_copy( useCompressedIndexXML , setUseCompressedIndexXML , bool          , General, false                      )
property_copy( compressBackup        , setCompressBackup        , bool          , General, true                       )
property_copy( showSplashScreen      , setShowSplashScreen      , bool          , General, true                       )
property_copy( showHistogram         , setShowHistogram         , bool          , General, true                       )
property_copy( autoSave              , setAutoSave              , int           , General, 5                          )
property_copy( backupCount           , setBackupCount           , int           , General, 5                          )
property_enum( tTimeStamps           , setTTimeStamps           , TimeStampTrust, General, Always                     )
property_copy( excludeDirectories    , setExcludeDirectories    , QString       , General, QString::fromLatin1("xml,ThumbNails,.thumbs") )

getValueFunc( QSize,histogramSize,  General,QSize(15,30) )
getValueFunc( ViewSortType,viewSortType,  General,(int)SortLastUse )

void SettingsData::setHistogramSize( const QSize& size )
{
    if ( size == histogramSize() )
        return;

    setValue( "General", "histogramSize", size );
    emit histogramSizeChanged( size );
}

void SettingsData::setViewSortType( const ViewSortType tp )
{
    if ( tp == viewSortType() )
        return;

    setValue( "General", "viewSortType", (int)tp );
    emit viewSortTypeChanged( tp );
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

////////////////////////////////
//// File Version Detection ////
////////////////////////////////

property_copy( detectModifiedFiles   , setDetectModifiedFiles   , bool          , FileVersionDetection, false               )
property_copy( modifiedFileComponent , setModifiedFileComponent , QString       , FileVersionDetection, QString()           )
property_copy( originalFileComponent , setOriginalFileComponent , QString       , FileVersionDetection, QString()           )
property_copy( moveOriginalContents  , setMoveOriginalContents  , bool          , FileVersionDetection, false               )
property_copy( autoStackNewFiles     , setAutoStackNewFiles     , bool          , FileVersionDetection, true                )
property_copy( copyFileComponent     , setCopyFileComponent     , QString       , FileVersionDetection, QString()           )
property_copy( copyFileReplacementComponent , setCopyFileReplacementComponent , QString  , FileVersionDetection, QString()  )

////////////////////
//// Thumbnails ////
////////////////////

property_copy( displayLabels           , setDisplayLabels          , bool                , Thumbnails, true       )
property_copy( displayCategories       , setDisplayCategories      , bool                , Thumbnails, false      )
property_copy( autoShowThumbnailView   , setAutoShowThumbnailView  , unsigned int        , Thumbnails, 0          )
property_copy( showNewestThumbnailFirst, setShowNewestFirst        , bool                , Thumbnails, false      )
property_copy( thumbnailDisplayGrid    , setThumbnailDisplayGrid   , bool                , Thumbnails, false      )
property_copy( previewSize             , setPreviewSize            , int                 , Thumbnails, 256        )
property_copy( thumbnailSpace          , setThumbnailSpace         , int                 , Thumbnails, 4          )
property_enum( thumbnailAspectRatio    , setThumbnailAspectRatio   , ThumbnailAspectRatio, Thumbnails, Aspect_4_3 )
property_ref(  backgroundColor         , setBackgroundColor        , QString             , Thumbnails, QColor(Qt::darkGray).name() )

getValueFunc_( int, thumbSize, groupForDatabase("Thumbnails"), "thumbSize", 150)

void SettingsData::setThumbSize( int value )
{
    QPixmapCache::clear();
    setValue( groupForDatabase("Thumbnails"), "thumbSize", value );
}


////////////////
//// Viewer ////
////////////////

property_ref ( viewerSize               , setViewerSize               , QSize           , Viewer, QSize(800,600) )
property_ref ( slideShowSize            , setSlideShowSize            , QSize           , Viewer, QSize(800,600) )
property_copy( launchViewerFullScreen   , setLaunchViewerFullScreen   , bool            , Viewer, false          )
property_copy( launchSlideShowFullScreen, setLaunchSlideShowFullScreen, bool            , Viewer, false          )
property_copy( showInfoBox              , setShowInfoBox              , bool            , Viewer, true           )
property_copy( showLabel                , setShowLabel                , bool            , Viewer, true           )
property_copy( showDescription          , setShowDescription          , bool            , Viewer, true           )
property_copy( showDate                 , setShowDate                 , bool            , Viewer, true           )
property_copy( showImageSize            , setShowImageSize            , bool            , Viewer, true           )
property_copy( showTime                 , setShowTime                 , bool            , Viewer, true           )
property_copy( showFilename             , setShowFilename             , bool            , Viewer, false          )
property_copy( showEXIF                 , setShowEXIF                 , bool            , Viewer, true           )
property_copy( slideShowInterval        , setSlideShowInterval        , int             , Viewer, 5              )
property_copy( viewerCacheSize          , setViewerCacheSize          , int             , Viewer, 120            )
property_copy( infoBoxWidth             , setInfoBoxWidth             , int             , Viewer, 400            )
property_copy( infoBoxHeight            , setInfoBoxHeight            , int             , Viewer, 300            )
property_enum( infoBoxPosition          , setInfoBoxPosition          , Position        , Viewer, Bottom         )
property_enum( viewerStandardSize       , setViewerStandardSize       , StandardViewSize, Viewer, FullSize       )

bool SettingsData::smoothScale() const
{
    return _smoothScale;
}

void SettingsData::setSmoothScale( bool b )
{
    _smoothScale = b;
    setValue( "Viewer", "smoothScale", b );
}

////////////////////
//// Categories ////
////////////////////

setValueFunc( setAlbumCategory,QString&,  General,albumCategory )

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

property_ref( untaggedCategory, setUntaggedCategory, QString, General, "Events")
property_ref( untaggedTag,      setUntaggedTag,      QString, General, "untagged")

//////////////
//// Exif ////
//////////////

#ifdef HAVE_EXIV2
    property_sset( exifForViewer, setExifForViewer,          Exif, StringSet()                            )
    property_sset( exifForDialog, setExifForDialog,          Exif, Exif::Info::instance()->standardKeys() )
    property_ref ( iptcCharset  , setIptcCharset  , QString, Exif, QString()                 )
#endif

/////////////////////
//// Exif Import ////
/////////////////////

property_copy( updateExifData           , setUpdateExifData           , bool , ExifImport, true );
property_copy( updateImageDate          , setUpdateImageDate          , bool , ExifImport, false );
property_copy( useModDateIfNoExif       , setUseModDateIfNoExif       , bool , ExifImport, true );
property_copy( updateOrientation        , setUpdateOrientation        , bool , ExifImport, false );
property_copy( updateDescription        , setUpdateDescription        , bool , ExifImport, false );

///////////////
//// SQLDB ////
///////////////

#ifdef SQLDB_SUPPORT
SQLDB::DatabaseAddress SettingsData::SQLParameters() const
{
    KConfigGroup config = KGlobal::config()->group(QString::fromLatin1("SQLDB"));
    try {
        return SQLDB::readConnectionParameters(config);
    }
    catch (SQLDB::DriverNotFoundError&) {}
    return SQLDB::DatabaseAddress();
}

void SettingsData::setSQLParameters(const SQLDB::DatabaseAddress& address)
{
    KConfigGroup config = KGlobal::config()->group(QString::fromLatin1("SQLDB"));
    SQLDB::writeConnectionParameters(address, config);
    config.sync();
}
#endif /* SQLDB_SUPPORT */

///////////////////////
//// Miscellaneous ////
///////////////////////

property_copy( delayLoadingPlugins, setDelayLoadingPlugins,  bool, Plug-ins, true  )

property_ref_(
        HTMLBaseDir, setHTMLBaseDir, QString,
        groupForDatabase( "HTML Settings" ),
        QString::fromLocal8Bit(qgetenv( "HOME" )) + STR( "/public_html" ) )
property_ref_(
        HTMLBaseURL, setHTMLBaseURL, QString,
        groupForDatabase( "HTML Settings" ),
        STR( "file://" ) + HTMLBaseDir() )
property_ref_(
        HTMLDestURL, setHTMLDestURL, QString,
        groupForDatabase( "HTML Settings" ),
        STR( "file://" ) + HTMLBaseDir() )
property_ref_(
        HTMLCopyright, setHTMLCopyright, QString,
        groupForDatabase( "HTML Settings" ),
        STR( "" ) )
property_ref_(
        HTMLDate, setHTMLDate, int,
        groupForDatabase( "HTML Settings" ),
        true )
property_ref_(
        HTMLTheme, setHTMLTheme, int,
        groupForDatabase( "HTML Settings" ),
        -1 )
property_ref_(
        HTMLKimFile, setHTMLKimFile, int,
        groupForDatabase( "HTML Settings" ),
        true )
property_ref_(
        HTMLInlineMovies, setHTMLInlineMovies, int,
        groupForDatabase( "HTML Settings" ),
        true )
property_ref_(
        HTMLThumbSize, setHTMLThumbSize, int,
        groupForDatabase( "HTML Settings" ),
        128 )
property_ref_(
        HTMLNumOfCols, setHTMLNumOfCols, int,
        groupForDatabase( "HTML Settings" ),
        5 )
property_ref_(
        HTMLSizes, setHTMLSizes, QString,
        groupForDatabase( "HTML Settings" ),
        STR("") )
property_ref_(
        HTMLIncludeSelections, setHTMLIncludeSelections, QString,
        groupForDatabase( "HTML Settings" ),
        STR("") )

property_ref_( password, setPassword, QString, groupForDatabase( "Privacy Settings" ), STR("") )

QDate SettingsData::fromDate() const
{
    QString date = value( "Miscellaneous", "fromDate", STR("") );
    return date.isEmpty() ? QDate( QDate::currentDate().year(), 1, 1 ) : QDate::fromString( date, Qt::ISODate );
}

void SettingsData::setFromDate( const QDate& date)
{
    if (date.isValid())
        setValue( "Miscellaneous", "fromDate", date.toString( Qt::ISODate ) );
}

QDate SettingsData::toDate() const
{
    QString date = value( "Miscellaneous", "toDate", STR("") );
    return date.isEmpty() ? QDate( QDate::currentDate().year()+1, 1, 1 ) : QDate::fromString( date, Qt::ISODate );
}

void  SettingsData::setToDate( const QDate& date)
{
    if (date.isValid())
        setValue( "Miscellaneous", "toDate", date.toString( Qt::ISODate ) );
}

QString SettingsData::imageDirectory() const
{
    return _imageDirectory;
}

QString SettingsData::groupForDatabase( const char* setting ) const
{
    return STR("%1 - %2").arg( STR(setting) ).arg( imageDirectory() );
}

DB::ImageSearchInfo SettingsData::currentLock() const
{
    return DB::ImageSearchInfo::loadLock();
}

void SettingsData::setCurrentLock( const DB::ImageSearchInfo& info, bool exclude )
{
    info.saveLock();
    setValue( groupForDatabase( "Privacy Settings" ), "exclude", exclude );
}

bool SettingsData::lockExcludes() const
{
    return value( groupForDatabase( "Privacy Settings" ), "exclude", false );
}

getValueFunc_( bool,locked,  groupForDatabase("Privacy Settings"),"locked",false )

void SettingsData::setLocked( bool lock, bool force )
{
    if ( lock == locked() && !force )
        return;

    setValue( groupForDatabase( "Privacy Settings" ), "locked", lock );
    emit locked( lock, lockExcludes() );
}

void SettingsData::setWindowGeometry( WindowType win, const QRect& geometry )
{
    setValue( "Window Geometry", win, geometry );
}

QRect SettingsData::windowGeometry( WindowType win ) const
{
    return value( "Window Geometry", win, QRect(0,0,800,600) );
}

bool Settings::SettingsData::hasUntaggedCategoryFeatureConfigured() const
{
    return DB::ImageDB::instance()->categoryCollection()->categoryNames().contains( untaggedCategory() );
}

double Settings::SettingsData::getThumbnailAspectRatio() const
{
    double ratio = 1.0;
    switch (Settings::SettingsData::instance()->thumbnailAspectRatio()) {
        case Settings::Aspect_16_9:
            ratio = 9.0 / 16;
            break;
        case Settings::Aspect_4_3:
            ratio = 3.0 / 4;
            break;
        case Settings::Aspect_3_2:
            ratio = 2.0 / 3;
            break;
        case Settings::Aspect_9_16:
            ratio = 16 / 9.0;
            break;
        case Settings::Aspect_3_4:
            ratio = 4 / 3.0;
            break;
        case Settings::Aspect_2_3:
            ratio = 3 / 2.0;
            break;
        case Settings::Aspect_1_1:
            ratio = 1.0;
            break;
    }
    return ratio;
}
