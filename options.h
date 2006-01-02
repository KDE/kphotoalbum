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

#ifndef OPTIONS_H
#define OPTIONS_H
#include <qstringlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qobject.h>
#include <qdom.h>
#include "imagesearchinfo.h"
#include "category.h"
#ifdef HASEXIV2
#  include "Exif/Info.h"
#endif
#include "set.h"
#include <config.h>
class ImageConfig;


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
#define stringSetProperty( group, prop, setFunction, defaultValue ) property__( Set<QString>, group, prop, setFunction, defaultValue )

class Options :public QObject {
    Q_OBJECT

public:
    static Options* instance();
    static bool ready();
    static void setup( const QString& imageDirectory );
    // -------------------------------------------------- General
    boolProperty( General, useEXIFRotate, setUseEXIFRotate, true );
    boolProperty( General, useEXIFComments, setUseEXIFComments, true );
    boolProperty( General, searchForImagesOnStartup, setSearchForImagesOnStartup, true );
    boolProperty( General, useCompressedIndexXML, setUseCompressedIndexXML, false );
    intProperty( General, autoSave, setAutoSave, 5 );
    intProperty( General, backupCount, setBackupCount, 0 );
    boolProperty( General, compressBackup, setCompressBackup, true );

    QSize histogramSize() const;
    void setHistogramSize( const QSize& size );

    // -------------------------------------------------- Thumbnails
    intProperty( Thumbnails, thumbSize, setThumbSize, 128 );
    intProperty( Thumbnails, previewSize, setPreviewSize, 256 );
    boolProperty( Thumbnails, displayLabels, setDisplayLabels, true );
    intProperty( Thumbnails, autoShowThumbnailView, setAutoShowThumbnailView, 0 );

    // -------------------------------------------------- Viewer
    sizeProperty( Viewer, viewerSize, setViewerSize, QSize( 800,600 ) );
    sizeProperty( Viewer, slideShowSize, setSlideShowSize, QSize( 800, 600 ) );
    boolProperty( Viewer, launchViewerFullScreen, setLaunchViewerFullScreen, false );
    boolProperty( Viewer, launchSlideShowFullScreen, setLaunchSlideShowFullScreen, false );
    intProperty( Viewer, slideShowInterval, setSlideShowInterval, 5 );
    intProperty( Viewer, viewerCacheSize, setViewerCacheSize, 25 );

    boolProperty( Viewer, showInfoBox, setShowInfoBox, true );
    boolProperty( Viewer, showDrawings, setShowDrawings, true );
    boolProperty( Viewer, showDescription, setShowDescription, true );
    boolProperty( Viewer, showDate, setShowDate, true );
    boolProperty( Viewer, showTime, setShowTime, true );
    boolProperty( Viewer, showEXIF, setShowEXIF, true );


    // -------------------------------------------------- Miscellaneous
    boolProperty( Plug-ins, delayLoadingPlugins, setDelayLoadingPlugins, true );

    void setFromDate( const QDate& );
    QDate fromDate() const;
    void setToDate( const QDate& );
    QDate toDate() const;

    enum ViewSortType { SortLastUse, SortAlpha };
    void setViewSortType( ViewSortType );
    ViewSortType viewSortType() const;

    enum Position { Bottom = 0, Top, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    Position infoBoxPosition() const;
    void setInfoBoxPosition( Position pos );


    // -------------------------------------------------- Categories
    QString fileForCategoryImage(  const QString& category, QString member ) const;
    void setCategoryImage( const QString& category, QString, const QImage& image );
    QImage categoryImage( const QString& category,  QString, int size ) const;
    QString albumCategory() const;
    void setAlbumCategory(  const QString& category );

    // -------------------------------------------------- EXIF
#ifdef HASEXIV2
    stringSetProperty( EXIF, exifForViewer, setExifForViewer, Set<QString>() );
    stringSetProperty( EXIF, exifForDialog, setExifForDialog, Exif::Info::instance()->standardKeys() );
#endif

    // -------------------------------------------------- misc
    enum TimeStampTrust {
        Always = 0,
        Ask = 1,
        Never = 2
    };

    bool trustTimeStamps();
    void setTTimeStamps( TimeStampTrust );
    TimeStampTrust tTimeStamps() const;

    QString imageDirectory() const;

    QString HTMLBaseDir() const;
    void setHTMLBaseDir( const QString& dir );

    QString HTMLBaseURL() const;
    void setHTMLBaseURL( const QString& dir );

    QString HTMLDestURL() const;
    void setHTMLDestURL( const QString& dir );

    void setCurrentLock( const ImageSearchInfo&, bool exclude );
    ImageSearchInfo currentLock() const;

    void setLocked( bool );
    bool isLocked() const;
    bool lockExcludes() const;

    void setPassword( const QString& passwd );
    QString password() const;

    enum WindowType { MainWindow = 0, ConfigWindow = 1 };
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
    Set<QString> value(const QString& group, const QString& option, const Set<QString>& defaultValue ) const;

    void setValue( const QString& group, const QString& option, int value );
    void setValue( const QString& group, const QString& option, const QString& value );
    void setValue( const QString& group, const QString& option, bool value );
    void setValue( const QString& group, const QString& option, const QColor& value );
    void setValue( const QString& group, const QString& option, const QSize& value );
    void setValue( const QString& group, const QString& option, const Set<QString>& value );

signals:
    void locked( bool lock, bool exclude );
    void viewSortTypeChanged( Options::ViewSortType );
    void histogramSizeChanged( const QSize& );

private:
    Options( const QString& imageDirectory  );
    static Options* _instance;
    bool _trustTimeStamps, _hasAskedAboutTimeStamps;
    friend class CategoryCollection;
    QString _imageDirectory;
};

#undef intProperty
#undef boolProperty
#undef property__

#endif /* OPTIONS_H */

