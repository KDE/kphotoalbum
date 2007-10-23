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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SETTINGS_SETTINGS_H
#define SETTINGS_SETTINGS_H
#include <qpixmap.h>
#include "DB/ImageSearchInfo.h"
#include "DB/Category.h"
#ifdef HASEXIV2
#  include "Exif/Info.h"
#endif
#include "Exif/Syncable.h"
#include "Utilities/Set.h"
#include "Utilities/Util.h"
#ifdef SQLDB_SUPPORT
namespace SQLDB { class DatabaseAddress; }
#endif

#define property__( type, group, prop, setFunction, defaultValue ) \
    void setFunction( type val )                                     \
    {                                                             \
        setValue( QString::fromLatin1(#group), QString::fromLatin1(#prop), val ); \
    }                                             \
    type prop() const                          \
    {                                             \
        return value( QString::fromLatin1(#group), QString::fromLatin1(#prop), defaultValue ); \
    }

#define intProperty( group, prop, setFunction, defaultValue ) property__( int, group, prop, setFunction, defaultValue )
#define boolProperty( group, prop, setFunction, defaultValue ) property__( bool, group, prop, setFunction, defaultValue )
#define colorProperty( group, prop, setFunction, defaultValue ) property__( QColor, group, prop, setFunction, defaultValue )
#define sizeProperty(  group, prop, setFunction, defaultValue ) property__( QSize, group, prop, setFunction, defaultValue )
#define stringProperty(  group, prop, setFunction, defaultValue ) property__( QString, group, prop, setFunction, defaultValue )
#define stringSetProperty( group, prop, setFunction, defaultValue ) property__( StringSet, group, prop, setFunction, defaultValue )
// Adding a new type? Don't forget to #undef these macros at the end.

namespace DB
{
    class CategoryCollection;
}

namespace Settings
{
    using Utilities::StringSet;

    enum Position { Bottom = 0, Top, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    enum ViewSortType { SortLastUse, SortAlpha };
    enum StandardViewSize {
        FullSize = 0,
        NaturalSize = 1,
        NaturalSizeIfFits = 2
    };
    enum ThumbnailAspectRatio {
        Aspect_1_1 = 0,
        Aspect_4_3 = 1,
        Aspect_3_2 = 2,
        Aspect_16_9 = 3,
        Aspect_3_4 = 4,
        Aspect_2_3 = 5,
        Aspect_9_16 = 6
    };
    enum WindowType { MainWindow = 0, ConfigWindow = 1 };

class SettingsData :public QObject {
    Q_OBJECT

public:
    static SettingsData* instance();
    static bool ready();
    static void setup( const QString& imageDirectory );
    // -------------------------------------------------- General
    boolProperty( General, searchForImagesOnStartup, setSearchForImagesOnStartup, true );
    boolProperty( General, dontReadRawFilesWithOtherMatchingFile, setDontReadRawFilesWithOtherMatchingFile, false );
    boolProperty( General, useCompressedIndexXML, setUseCompressedIndexXML, false );
    intProperty( General, autoSave, setAutoSave, 5 );
    intProperty( General, backupCount, setBackupCount, 5 );
    boolProperty( General, compressBackup, setCompressBackup, true );
    boolProperty( General, showSplashScreen, setShowSplashScreen, true );

    QSize histogramSize() const;
    void setHistogramSize( const QSize& size );

    // -------------------------------------------------- Thumbnails
    intProperty( Thumbnails, previewSize, setPreviewSize, 256 );
    boolProperty( Thumbnails, displayLabels, setDisplayLabels, true );
    boolProperty( Thumbnails, displayCategories, setDisplayCategories, false );
    intProperty( Thumbnails, autoShowThumbnailView, setAutoShowThumbnailView, 0 );
    boolProperty( Thumbnails, showNewestThumbnailFirst, setShowNewestFirst, false );
    boolProperty( Thumbnails, thumbnailDarkBackground, setThumbnailDarkBackground, true );
    boolProperty( Thumbnails, thumbnailDisplayGrid, setThumbnailDisplayGrid, false );
    intProperty( Thumbnails, thumbnailSpace, setThumbnailSpace, 1);

    void setThumbnailCache( int value );
    int thumbnailCache() const;

    void setThumbSize( int value );
    int thumbSize() const;

    // -------------------------------------------------- Viewer
    sizeProperty( Viewer, viewerSize, setViewerSize, QSize( 800,600 ) );
    sizeProperty( Viewer, slideShowSize, setSlideShowSize, QSize( 800, 600 ) );
    boolProperty( Viewer, launchViewerFullScreen, setLaunchViewerFullScreen, false );
    boolProperty( Viewer, launchSlideShowFullScreen, setLaunchSlideShowFullScreen, false );
    intProperty( Viewer, slideShowInterval, setSlideShowInterval, 5 );
    intProperty( Viewer, viewerCacheSize, setViewerCacheSize, 25 );

    boolProperty( Viewer, showInfoBox, setShowInfoBox, true );
    boolProperty( Viewer, showDrawings, setShowDrawings, true );
    boolProperty( Viewer, showLabel, setShowLabel, true );
    boolProperty( Viewer, showDescription, setShowDescription, true );
    boolProperty( Viewer, showDate, setShowDate, true );
    boolProperty( Viewer, showImageSize, setShowImageSize, true );
    boolProperty( Viewer, showTime, setShowTime, true );
    boolProperty( Viewer, showFilename, setShowFilename, false );
    boolProperty( Viewer, showEXIF, setShowEXIF, true );
    void setViewerStandardSize(StandardViewSize);
    StandardViewSize viewerStandardSize() const;

    static bool smoothScale();
    void setSmoothScale( bool );

    // -------------------------------------------------- Miscellaneous
    boolProperty( Plug-ins, delayLoadingPlugins, setDelayLoadingPlugins, true );

    void setFromDate( const QDate& );
    QDate fromDate() const;
    void setToDate( const QDate& );
    QDate toDate() const;

    void setViewSortType( ViewSortType );
    ViewSortType viewSortType() const;

    Position infoBoxPosition() const;
    void setInfoBoxPosition( Position pos );


    // -------------------------------------------------- Categories
    QString fileForCategoryImage(  const QString& category, QString member ) const;
    void setCategoryImage( const QString& category, QString, const QImage& image );
    QPixmap categoryImage( const QString& category,  QString, int size ) const;
    QString albumCategory() const;
    void setAlbumCategory(  const QString& category );

    // -------------------------------------------------- EXIF
#ifdef HASEXIV2
    stringSetProperty( EXIF, exifForViewer, setExifForViewer, StringSet() );
    stringSetProperty( EXIF, exifForDialog, setExifForDialog, Exif::Info::instance()->standardKeys() );
#endif

    // -------------------------------------------------- SQLDB
#ifdef SQLDB_SUPPORT
    void setSQLParameters(const SQLDB::DatabaseAddress& other);
    SQLDB::DatabaseAddress getSQLParameters() const;
#endif

    // -------------------------------------------------- misc

    stringProperty( General, backend, setBackend, QString::fromLatin1("xml") );

    void setThumbnailAspectRatio( ThumbnailAspectRatio );
    ThumbnailAspectRatio thumbnailAspectRatio() const;

    intProperty( EXIF, iptcCharset, setIptcCharset, 0 );

    void setCategorySyncingFields( const bool writing, const QString& category, const QValueList<Exif::Syncable::Kind>& fields );
    QValueList<Exif::Syncable::Kind> categorySyncingFields( const bool writing, const QString& category ) const;
    void setCategorySyncingSuperGroups( const QString& category, const Exif::Syncable::SuperGroupHandling how );
    Exif::Syncable::SuperGroupHandling categorySyncingSuperGroups( const QString& category ) const;
    void setCategorySyncingMultiValue( const QString& category, const Exif::Syncable::MultiValueHandling how );
    Exif::Syncable::MultiValueHandling categorySyncingMultiValue( const QString& category ) const;
    void setCategorySyncingAddName( const QString& category, bool include );
    bool categorySyncingAddName( const QString& category );

    void setLabelSyncing( const bool writing, const QValueList<Exif::Syncable::Kind>& fields );
    QValueList<Exif::Syncable::Kind> labelSyncing( const bool writing ) const;
    void setDescriptionSyncing( const bool writing, const QValueList<Exif::Syncable::Kind>& fields );
    QValueList<Exif::Syncable::Kind> descriptionSyncing( const bool writing ) const;
    void setOrientationSyncing( const bool writing, const QValueList<Exif::Syncable::Kind>& fields );
    QValueList<Exif::Syncable::Kind> orientationSyncing( const bool writing ) const;
    void setDateSyncing( const bool writing, const QValueList<Exif::Syncable::Kind>& fields );
    QValueList<Exif::Syncable::Kind> dateSyncing( const bool writing ) const;
    QValueList<Exif::Syncable::Kind> defaultFields( bool writing, const QString& identifier ) const;

    QString imageDirectory() const;

    QString HTMLBaseDir() const;
    void setHTMLBaseDir( const QString& dir );

    QString HTMLBaseURL() const;
    void setHTMLBaseURL( const QString& dir );

    QString HTMLDestURL() const;
    void setHTMLDestURL( const QString& dir );

    void setCurrentLock( const DB::ImageSearchInfo&, bool exclude );
    DB::ImageSearchInfo currentLock() const;

    void setLocked( bool locked, bool force );
    bool isLocked() const;
    bool lockExcludes() const;

    void setPassword( const QString& passwd );
    QString password() const;

    void setWindowGeometry( WindowType, const QRect& geometry );
    QRect windowGeometry( WindowType ) const;
    QString windowTypeToString( WindowType tp ) const;

    QString groupForDatabase( const QString& setting ) const;

protected:
    int value( const QString& group, const QString& option, int defaultValue ) const;
    QString value( const QString& group, const QString& option, const QString& defaultValue ) const;
    bool value( const QString& group, const QString& option, bool defaultValue ) const;
    QColor value( const QString& group, const QString& option, const QColor& defaultValue ) const;
    QSize value( const QString& group, const QString& option, const QSize& defaultValue ) const;
    StringSet value(const QString& group, const QString& option, const StringSet& defaultValue ) const;

    void setValue( const QString& group, const QString& option, int value );
    void setValue( const QString& group, const QString& option, const QString& value );
    void setValue( const QString& group, const QString& option, bool value );
    void setValue( const QString& group, const QString& option, const QColor& value );
    void setValue( const QString& group, const QString& option, const QSize& value );
    void setValue( const QString& group, const QString& option, const StringSet& value );

signals:
    void locked( bool lock, bool exclude );
    void viewSortTypeChanged( Settings::ViewSortType );
    void histogramSizeChanged( const QSize& );

private:
    SettingsData( const QString& imageDirectory  );
    static SettingsData* _instance;
    friend class DB::CategoryCollection;
    QString _imageDirectory;

    // helpers for metadata synchronization
    void _setSyncing( bool writing, const QString& identifier, const QValueList<Exif::Syncable::Kind>& fields );
    QValueList<Exif::Syncable::Kind> _syncing( bool writing, const QString& identifier ) const;
};
} // end of namespace


#undef intProperty
#undef boolProperty
#undef colorProperty
#undef sizeProperty
#undef stringProperty
#undef stringSetProperty
#undef property__


#endif /* SETTINGS_SETTINGS_H */

