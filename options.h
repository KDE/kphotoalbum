/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef OPTIONS_H
#define OPTIONS_H
#include <qstringlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qobject.h>
#include <qdom.h>
#include "membermap.h"
#include "imagesearchinfo.h"
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

    // -------------------------------------------------- Options
    void setOption( const QString& optionGroup,  const QStringList& value );
    void addOption( const QString& optionGroup,  const QString& value );
    void removeOption( const QString& optionGroup, const QString& value );
    QStringList optionValue( const QString& optionGroup ) const;
    QStringList optionValueInclGroups( const QString& optionGroup ) const;
    void renameOption( const QString& optionGroup, const QString& oldValue, const QString& newValue );

    QString fileForCategoryImage(  const QString& optionGroup, QString member ) const;
    void setOptionImage( const QString& optionGroup, QString, const QImage& image );
    QImage optionImage( const QString& optionGroup,  QString, int size ) const;

    // -------------------------------------------------- Option Groups
    enum ViewSize { Small, Large };
    enum ViewType { ListView, IconView };

    struct OptionGroupInfo
    {
        OptionGroupInfo() {}
        OptionGroupInfo( const QString& icon, ViewSize size, ViewType type, bool show = true )
            : _icon(icon), _show(show), _size( size ), _type( type ) {}
        QString _icon;
        bool _show;
        ViewSize _size;
        ViewType _type;
    };

    QStringList optionGroups() const;
    void addOptionGroup( const QString& name, const QString& icon, ViewSize size, ViewType type );
    void deleteOptionGroup( const QString& name );
    void renameOptionGroup( const QString& oldName, const QString& newName );

    QString textForOptionGroup( const QString& name ) const;

    QPixmap iconForOptionGroup( const QString& name, int size = 22 ) const;
    QString iconNameForOptionGroup( const QString& name ) const;
    void setIconForOptionGroup( const QString& name, const QString& icon );

    void setViewSize( const QString& optionGroup, ViewSize size );
    void setViewType( const QString& optionGroup, ViewType type );
    ViewSize viewSize( const QString& optionGroup ) const;
    ViewType viewType( const QString& optionGroup ) const;

    QString albumCategory() const;
    void setAlbumCategory(  const QString& optionGroup );

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
    void setShowOption( const QString& optionGroup, bool b );

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

signals:
    void optionGroupsChanged();
    void changed();
    void renamedOption( const QString& optionGroup, const QString& oldName, const QString& newName );
    void deletedOption( const QString& optionGroup, const QString& name );
    void locked( bool lock, bool exclude );
    void viewSortTypeChanged( Options::ViewSortType );

private:
    Options( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QDomElement& memberGroups, const QString& imageDirectory  );
    static Options* _instance;

    int _thumbSize,  _previewSize;
    TimeStampTrust _tTimeStamps;
    bool _useEXIFRotate;
    bool _useEXIFComments;
    int _autoSave, _maxImages;
    bool _trustTimeStamps, _markNew, _hasAskedAboutTimeStamps, _ensureImageWindowsOnScreen;
    QMap<QString, QStringList> _options;
    QMap<QString,OptionGroupInfo> _optionGroups;
    QString _imageDirectory, _htmlBaseDir, _htmlBaseURL, _htmlDestURL;

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
};

#endif /* OPTIONS_H */

