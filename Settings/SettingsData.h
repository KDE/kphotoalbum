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

#ifndef SETTINGS_SETTINGS_H
#   define SETTINGS_SETTINGS_H

#include <QPixmap>
#include "DB/ImageSearchInfo.h"
#include "DB/Category.h"
#include <config-kpa-exiv2.h>

#ifdef HAVE_EXIV2
#   include "Exif/Info.h"
#endif

#include "Utilities/Set.h"
#include <config-kpa-sqldb.h>

#ifdef SQLDB_SUPPORT
    namespace SQLDB { class DatabaseAddress; }
#endif

#define property_decl( getType,getFunction, setFunction,setType ) \
    getType getFunction() const;                                    \
    void set##setFunction( const setType )

#define property_decl_copy( type, getFunction, setFunction ) property_decl( type,getFunction, setFunction,type  )
#define property_decl_ref( type, getFunction, setFunction )  property_decl( type,getFunction, setFunction,type& )

#define property( group, prop, getFunction, setFunction, defaultValue, type ) \
    type getFunction() const                                                  \
    {                                                                         \
        return (type)value( #group, prop, defaultValue );                     \
    }                                                                         \
    void set##setFunction( type val )                                         \
    {                                                                         \
        setValue( #group, prop, val );                                        \
    }

#define property_1( group, prop, setFunction, defaultValue, type ) property(#group,#prop,prop,setFunction,defaultValue,type)
#define property_2( group, prop, setFunction, defaultValue, type ) property( group,#prop,prop,setFunction,defaultValue,type)

#define       intProperty( group, prop, setFunction, defaultValue ) property_1( group, prop, setFunction, defaultValue, int       )
#define      boolProperty( group, prop, setFunction, defaultValue ) property_1( group, prop, setFunction, defaultValue, bool      )
#define     colorProperty( group, prop, setFunction, defaultValue ) property_1( group, prop, setFunction, defaultValue, QColor    )
#define      sizeProperty( group, prop, setFunction, defaultValue ) property_1( group, prop, setFunction, defaultValue, QSize     )
#define    stringProperty( group, prop, setFunction, defaultValue ) property_1( group, prop, setFunction, defaultValue, QString   )
#define stringSetProperty( group, prop, setFunction, defaultValue ) property_1( group, prop, setFunction, defaultValue, StringSet )
// Adding a new type? Don't forget to #undef these macros at the end.

namespace DB
{
    class CategoryCollection;
}

namespace Settings
{
    using Utilities::StringSet;

    enum Position             { Bottom, Top, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    enum ViewSortType         { SortLastUse, SortAlpha };
    enum TimeStampTrust       { Always, Ask, Never};
    enum StandardViewSize     { FullSize, NaturalSize, NaturalSizeIfFits };
    enum ThumbnailAspectRatio { Aspect_1_1, Aspect_4_3, Aspect_3_2, Aspect_16_9, Aspect_3_4, Aspect_2_3, Aspect_9_16 };

    typedef const char* WindowType;
    extern const WindowType MainWindow, ConfigWindow;

class SettingsData : public QObject
{
    Q_OBJECT

public:
    static SettingsData* instance();
    static bool          ready();
    static void          setup( const QString& imageDirectory );

    /////////////////
    //// General ////
    /////////////////

    stringProperty( General , backend                               , Backend                               , QString::fromLatin1("xml") );
      boolProperty( General , useEXIFRotate                         , UseEXIFRotate                         , true                       );
      boolProperty( General , useEXIFComments                       , UseEXIFComments                       , true                       );
      boolProperty( General , searchForImagesOnStartup              , SearchForImagesOnStartup              , true                       );
      boolProperty( General , dontReadRawFilesWithOtherMatchingFile , DontReadRawFilesWithOtherMatchingFile , false                      );
      boolProperty( General , useCompressedIndexXML                 , UseCompressedIndexXML                 , false                      );
      boolProperty( General , compressBackup                        , CompressBackup                        , true                       );
      boolProperty( General , showSplashScreen                      , ShowSplashScreen                      , true                       );
       intProperty( General , autoSave                              , AutoSave                              , 5                          );
       intProperty( General , backupCount                           , BackupCount                           , 5                          );
        property_1( General , tTimeStamps                           , TTimeStamps                           , 0,       TimeStampTrust    );

    property_decl_ref( QSize, histogramSize, HistogramSize );

    bool trustTimeStamps();

    property_decl_copy( ViewSortType, viewSortType, ViewSortType );

    ////////////////////
    //// Thumbnails ////
    ////////////////////

    boolProperty( Thumbnails , displayLabels            , DisplayLabels           , true                              );
    boolProperty( Thumbnails , displayCategories        , DisplayCategories       , false                             );
    boolProperty( Thumbnails , autoShowThumbnailView    , AutoShowThumbnailView   , 0                                 );
    boolProperty( Thumbnails , showNewestThumbnailFirst , ShowNewestFirst         , false                             );
    boolProperty( Thumbnails , thumbnailDarkBackground  , ThumbnailDarkBackground , true                              );
    boolProperty( Thumbnails , thumbnailDisplayGrid     , ThumbnailDisplayGrid    , false                             );
     intProperty( Thumbnails , previewSize              , PreviewSize             , 256                               );
     intProperty( Thumbnails , thumbnailSpace           , ThumbnailSpace          , 1                                 ); // Border space around thumbnails.
      property_1( Thumbnails , thumbnailAspectRatio     , ThumbnailAspectRatio     , Aspect_4_3 , ThumbnailAspectRatio );

    property_decl_copy( int, thumbnailCacheScreens, ThumbnailCacheScreens );
    property_decl_copy( int, thumbSize, ThumbSize );

    size_t thumbnailCacheBytes() const;   // convenience method

    /**
     * Return an approximate figure of megabytes to cache to be able to
     * cache the amount of "screens" of caches.
     */
    static size_t thumbnailBytesForScreens(int screen);

    ////////////////
    //// Viewer ////
    ////////////////

    sizeProperty( Viewer , viewerSize                , ViewerSize                , QSize(800,600)              );
    sizeProperty( Viewer , slideShowSize             , SlideShowSize             , QSize(800,600)              );
    boolProperty( Viewer , launchViewerFullScreen    , LaunchViewerFullScreen    , false                       );
    boolProperty( Viewer , launchSlideShowFullScreen , LaunchSlideShowFullScreen , false                       );
    boolProperty( Viewer , showInfoBox               , ShowInfoBox               , true                        );
    boolProperty( Viewer , showLabel                 , ShowLabel                 , true                        );
    boolProperty( Viewer , showDescription           , ShowDescription           , true                        );
    boolProperty( Viewer , showDate                  , ShowDate                  , true                        );
    boolProperty( Viewer , showImageSize             , ShowImageSize             , true                        );
    boolProperty( Viewer , showTime                  , ShowTime                  , true                        );
    boolProperty( Viewer , showFilename              , ShowFilename              , false                       );
    boolProperty( Viewer , showEXIF                  , ShowEXIF                  , true                        );
     intProperty( Viewer , slideShowInterval         , SlideShowInterval         , 5                           );
     intProperty( Viewer , viewerCacheSize           , ViewerCacheSize           , 25                          );
     intProperty( Viewer , infoBoxWidth              , InfoBoxWidth              , 400                         );
     intProperty( Viewer , infoBoxHeight             , InfoBoxHeight             , 300                         );
      property_1( Viewer , viewerStandardSize        , ViewerStandardSize        , FullSize , StandardViewSize );
      property_1( Viewer , infoBoxPosition           , InfoBoxPosition           , 0        , Position         );

    property_decl_copy( bool, smoothScale, SmoothScale);

    ////////////////////
    //// Categories ////
    ////////////////////

    QString fileForCategoryImage ( const QString& category, QString member ) const;
    void    setCategoryImage     ( const QString& category, QString, const QImage& image );
    QPixmap categoryImage        ( const QString& category,  QString, int size ) const;

    property_decl_ref( QString, albumCategory, AlbumCategory );

    //////////////
    //// EXIF ////
    //////////////

#ifdef HAVE_EXIV2
    stringSetProperty ( EXIF , exifForViewer , ExifForViewer , StringSet()                            );
    stringSetProperty ( EXIF , exifForDialog , ExifForDialog , Exif::Info::instance()->standardKeys() );
    stringProperty    ( EXIF , iptcCharset   , IptcCharset   , QString::null                          );
#endif

    ///////////////
    //// SQLDB ////
    ///////////////

#ifdef SQLDB_SUPPORT
    property_decl_ref( SQLDB::DatabaseAddress, SQLParameters, SQLParameters );
#endif

    ///////////////////////
    //// Miscellaneous ////
    ///////////////////////

    boolProperty( Plug-ins, delayLoadingPlugins, DelayLoadingPlugins, true );

    property_decl_ref( QDate , fromDate , FromDate );
    property_decl_ref( QDate , toDate   , ToDate   );

    property_decl_ref( QString, HTMLBaseDir, HTMLBaseDir );
    property_decl_ref( QString, HTMLBaseURL, HTMLBaseURL );
    property_decl_ref( QString, HTMLDestURL, HTMLDestURL );

    property_decl_ref( QString, password, Password );

    QString imageDirectory() const;

    QString groupForDatabase( const char* setting ) const;

    void setCurrentLock( const DB::ImageSearchInfo&, bool exclude );
    DB::ImageSearchInfo currentLock() const;

    void setLocked( bool locked, bool force );
    bool isLocked() const;
    bool lockExcludes() const;

    void  setWindowGeometry( WindowType, const QRect& geometry );
    QRect windowGeometry( WindowType ) const;

private:
    int       value( const char*    group , const char* option , int              defaultValue ) const;
    QString   value( const char*    group , const char* option , const QString&   defaultValue ) const;
    QString   value( const QString& group , const char* option , const QString&   defaultValue ) const;
    bool      value( const char*    group , const char* option , bool             defaultValue ) const;
    bool      value( const QString& group , const char* option , bool             defaultValue ) const;
    QColor    value( const char*    group , const char* option , const QColor&    defaultValue ) const;
    QSize     value( const char*    group , const char* option , const QSize&     defaultValue ) const;
    StringSet value( const char*    group , const char* option , const StringSet& defaultValue ) const;

    void setValue( const char*    group , const char* option , int              value );
    void setValue( const char*    group , const char* option , const QString&   value );
    void setValue( const QString& group , const char* option , const QString&   value );
    void setValue( const char*    group , const char* option , bool             value );
    void setValue( const QString& group , const char* option , bool             value );
    void setValue( const char*    group , const char* option , const QColor&    value );
    void setValue( const char*    group , const char* option , const QSize&     value );
    void setValue( const char*    group , const char* option , const StringSet& value );

signals:
    void locked( bool lock, bool exclude );
    void viewSortTypeChanged( Settings::ViewSortType );
    void histogramSizeChanged( const QSize& );

private:
    SettingsData( const QString& imageDirectory  );

    bool                 _trustTimeStamps;
    bool                 _hasAskedAboutTimeStamps;
    QString              _imageDirectory;
    static SettingsData* _instance;

    friend class DB::CategoryCollection;
};
} // end of namespace


#undef intProperty
#undef boolProperty
#undef colorProperty
#undef sizeProperty
#undef stringProperty
#undef stringSetProperty
#undef property
#undef property_1
#undef property_2
#undef property_decl
#undef property_decl_copy
#undef property_decl_ref


#endif /* SETTINGS_SETTINGS_H */
