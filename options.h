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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef OPTIONS_H
#define OPTIONS_H
#include <qstringlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qobject.h>
#include <qdom.h>
#include "membermap.h"
#include "imagesearchinfo.h"
#include "category.h"
class ImageConfig;

class Options :public QObject {
    Q_OBJECT

public:
    static Options* instance();
    static bool ready();
    static void setup( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QDomElement& memberGroups, const QString& imageDirectory );

    void setThumbSize( int );
    int thumbSize() const;

    void setPreviewSize( int );
    int previewSize() const;

    void setMaxImages( int );
    int maxImages() const;

    void setViewerSize( const QSize& );
    QSize viewerSize() const;
    void setLaunchViewerFullScreen( bool b );
    bool launchViewerFullScreen() const;

    void setSlideShowSize( const QSize& );
    QSize slideShowSize() const;
    void setLaunchSlideShowFullScreen( bool b );
    bool launchSlideShowFullScreen() const;

    enum ViewSortType { SortLastUse, SortAlpha };
    void setViewSortType( ViewSortType );
    ViewSortType viewSortType() const;

    void setFromDate( const QDate& );
    QDate fromDate() const;
    void setToDate( const QDate& );
    QDate toDate() const;

    // -------------------------------------------------- Options
    void setOption( const QString& category,  const QStringList& value );
    void addOption( const QString& category,  const QString& value );
    void removeOption( const QString& category, const QString& value );
    QStringList optionValue( const QString& category ) const;
    QStringList optionValueInclGroups( const QString& category ) const;
    void renameOption( const QString& category, const QString& oldValue, const QString& newValue );

    QString fileForCategoryImage(  const QString& category, QString member ) const;
    void setOptionImage( const QString& category, QString, const QImage& image );
    QImage optionImage( const QString& category,  QString, int size ) const;

    // -------------------------------------------------- Categories
    QString albumCategory() const;
    void setAlbumCategory(  const QString& category );

    // -------------------------------------------------- Member Groups
    const MemberMap& memberMap();
    void setMemberMap( const MemberMap& );

    // -------------------------------------------------- Options for the Viewer
    enum Position { Bottom = 0, Top, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    bool showInfoBox() const;
    bool showDrawings() const;
    bool showDescription() const;
    bool showDate() const;
    bool showTime() const;
    bool showOption( const QString& ) const;

    void setShowInfoBox(bool b);
    void setShowDrawings(bool b);
    void setShowDescription(bool b);
    void setShowDate(bool b);
    void setShowTime(bool b);
    void setShowOption( const QString& category, bool b );

    Position infoBoxPosition() const;
    void setInfoBoxPosition( Position pos );

    // -------------------------------------------------- misc
    enum TimeStampTrust {
        Always = 0,
        Ask = 1,
        Never = 2
    };

    bool trustTimeStamps();
    void setTTimeStamps( TimeStampTrust );
    TimeStampTrust tTimeStamps() const;

    void setUseEXIFRotate( bool b );
    bool useEXIFRotate() const;

    void setUseEXIFComments( bool b );
    bool useEXIFComments() const;

    void setAutoSave( int min );
    int autoSave() const;

    void save( QDomElement top );

    QString imageDirectory() const;

    QString HTMLBaseDir() const;
    void setHTMLBaseDir( const QString& dir );

    QString HTMLBaseURL() const;
    void setHTMLBaseURL( const QString& dir );

    QString HTMLDestURL() const;
    void setHTMLDestURL( const QString& dir );

    void saveConfigWindowLayout( ImageConfig* );
    void loadConfigWindowLayout( ImageConfig* );

    void setCurrentLock( const ImageSearchInfo&, bool exclude );
    ImageSearchInfo currentLock() const;

    void setLocked( bool );
    bool isLocked() const;
    bool lockExcludes() const;

    void setPassword( const QString& passwd );
    QString password() const;

    void setSlideShowInterval( int );
    int slideShowInterval() const;

    void setDisplayLabels( bool );
    bool displayLabels() const;

    void setThumbNailBackgroundColor( const QColor& );
    QColor thumbNailBackgroundColor() const;

    enum WindowType { MainWindow = 0, ConfigWindow = 1, LastWindowSize = 2};
    void setWindowSize( WindowType, const QSize& size );
    QSize windowSize( WindowType ) const;

    int viewerCacheSize() const;
    void setViewerCacheSize( int size );

    bool searchForImagesOnStartup() const;
    void setSearchForImagesOnStartup(bool b);

    int autoShowThumbnailView() const;
    void setAutoShowThumbnailView( int val );

    QSize histogramSize() const;
    void setHistogramSize( const QSize& size );

    bool alignColumns() const;
    void setAlignColumns( bool );

    int rowSpacing() const;
    void setRowSpacing( int );

protected:
    void createSpecialCategories();

signals:
    void changed();
    void renamedOption( const QString& category, const QString& oldName, const QString& newName );
    void deletedOption( const QString& category, const QString& name );
    void locked( bool lock, bool exclude );
    void viewSortTypeChanged( Options::ViewSortType );
    void histogramSizeChanged( const QSize& );

private:
    Options( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QDomElement& memberGroups, const QString& imageDirectory  );
    static Options* _instance;

    int _thumbSize,  _previewSize;
    TimeStampTrust _tTimeStamps;
    bool _useEXIFRotate;
    bool _useEXIFComments;
    int _autoSave, _maxImages;
    bool _trustTimeStamps, _markNew, _hasAskedAboutTimeStamps, _ensureImageWindowsOnScreen;
    QSize _histogramSize;

    friend class CategoryCollection;
    QMap<QString, QStringList> _options;
    QString _imageDirectory, _htmlBaseDir, _htmlBaseURL, _htmlDestURL;
    QDate _fromDate, _toDate;

    Position _infoBoxPosition;
    bool _showInfoBox, _showDrawings, _showDescription, _showDate, _showTime;
    QDomElement _configDock;

    QSize _viewerSize;
    QSize _slideShowSize;
    bool _launchViewerFullScreen, _launchSlideShowFullScreen;
    int _slideShowInterval;

    MemberMap _members;
    ImageSearchInfo _currentLock;
    bool _locked, _exclude;
    QString _passwd;
    ViewSortType _viewSortType;
    QString _albumCategory;
    bool _displayLabels;
    QColor _thumbNailBackgroundColor;
    QMap<WindowType, QSize> _windowSizes;
    int _viewerCacheSize;
    bool _searchForImagesOnStartup;
    int _autoShowThumbnailView;
    bool _alignColumns;
    int _rowSpacing;
};

#endif /* OPTIONS_H */

