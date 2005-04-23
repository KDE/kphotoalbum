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

#include "options.h"
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include "util.h"
#include <stdlib.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qapplication.h>
#include <qcursor.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kglobal.h>
#include "imagedb.h"
#include "imageconfig.h"
#include <qtextstream.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include "categorycollection.h"
#include <qdatetime.h>
#include <qnamespace.h>
#include "imageinfo.h"

Options* Options::_instance = 0;

Options* Options::instance()
{
    if ( ! _instance )
        qFatal("instance called before loading a setup!");
    return _instance;
}

Options::Options( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QDomElement& memberGroups, const QString& imageDirectory )
    : _hasAskedAboutTimeStamps( false ), _imageDirectory( imageDirectory )
{
    _thumbSize = config.attribute( QString::fromLatin1("thumbSize"), QString::fromLatin1( "64" ) ).toInt();
    _previewSize = config.attribute( QString::fromLatin1( "previewSize" ), QString::fromLatin1( "256" ) ).toInt();
    _tTimeStamps = (TimeStampTrust) config.attribute( QString::fromLatin1("trustTimeStamps"),  QString::fromLatin1("0") ).toInt();
    _useEXIFRotate = (bool) config.attribute( QString::fromLatin1( "useEXIFRotate" ), QString::fromLatin1( "1" ) ).toInt();
    _useEXIFComments = (bool) config.attribute( QString::fromLatin1( "useEXIFComments" ), QString::fromLatin1( "1" ) ).toInt();
    _autoSave = config.attribute( QString::fromLatin1("autoSave"), QString::number( 5 ) ).toInt();
    _maxImages = config.attribute( QString::fromLatin1("maxImages"), QString::number( 100 ) ).toInt();
    _ensureImageWindowsOnScreen = (bool) config.attribute( QString::fromLatin1( "ensureImageWindowsOnScreen" ), QString::fromLatin1( "1" ) ).toInt();
    _htmlBaseDir = config.attribute( QString::fromLatin1("htmlBaseDir"), QString::fromLocal8Bit(getenv("HOME")) + QString::fromLatin1("/public_html") );
    _htmlBaseURL = config.attribute( QString::fromLatin1("htmlBaseURL"), QString::fromLatin1( "file://" ) + _htmlBaseDir );
    _htmlDestURL = config.attribute( QString::fromLatin1("htmlDestURL"), QString::fromLatin1( "file://" ) + _htmlBaseDir );
    _infoBoxPosition = (Position) config.attribute( QString::fromLatin1("infoBoxPosition"), QString::fromLatin1("0") ).toInt();
    _showInfoBox = config.attribute( QString::fromLatin1("showInfoBox"), QString::fromLatin1("1") ).toInt();
    _showDrawings = config.attribute( QString::fromLatin1("showDrawings"), QString::fromLatin1("1") ).toInt();
    _showDescription = config.attribute( QString::fromLatin1("showDescription"), QString::fromLatin1("1") ).toInt();
    _showDate = config.attribute( QString::fromLatin1("showDate"), QString::fromLatin1("1") ).toInt();
    _showTime = config.attribute( QString::fromLatin1("showTime"), QString::fromLatin1("1") ).toInt();
    _locked = config.attribute( QString::fromLatin1( "locked" ), QString::fromLatin1( "0" ) ).toInt();
    _exclude = config.attribute( QString::fromLatin1( "exclude" ), QString::fromLatin1( "1" ) ).toInt();
    _passwd = config.attribute( QString::fromLatin1( "passwd" ) );
    _albumCategory = config.attribute( QString::fromLatin1( "albumCategory" ) );
    _viewSortType = (ViewSortType) config.attribute( QString::fromLatin1( "viewSortType" ) ).toInt();
    if ( config.hasAttribute( QString::fromLatin1( "fromDate" ) ) )
        _fromDate = QDate::fromString( config.attribute( QString::fromLatin1( "fromDate" ) ), ISODate );
    else
        _fromDate = QDate( QDate::currentDate().year(), 1, 1 );
    if ( config.hasAttribute( QString::fromLatin1( "toDate" ) ) )
        _toDate = QDate::fromString( config.attribute( QString::fromLatin1( "toDate" ) ), ISODate );
    else
        _toDate = QDate( QDate::currentDate().year()+1, 1, 1 );
    _launchViewerFullScreen = (bool) config.attribute( QString::fromLatin1( "launchViewerFullScreen" ) ).toInt();
    _launchSlideShowFullScreen = (bool) config.attribute( QString::fromLatin1( "launchSlideShowFullScreen" ) ).toInt();
    _displayLabels = (bool) config.attribute( QString::fromLatin1( "displayLabels" ), QString::fromLatin1( "1" ) ).toInt();
    _thumbNailBackgroundColor = QColor( config.attribute( QString::fromLatin1( "thumbNailBackgroundColor" ),
                                                          QString::fromLatin1( "black" ) ) );
    _viewerCacheSize = config.attribute( QString::fromLatin1( "viewerCacheSize" ), QString::fromLatin1( "25" )  ).toInt();
    _searchForImagesOnStartup = (bool) config.attribute( QString::fromLatin1( "searchForImagesOnStartup" ),
                                                         QString::fromLatin1( "1" ) ).toInt();
    _autoShowThumbnailView =  config.attribute( QString::fromLatin1( "autoShowThumbnailViewCount" ),
                                                QString::fromLatin1( "0" ) ).toInt();
    int width = config.attribute( QString::fromLatin1( "histogramWidth" ), QString::fromLatin1( "15" ) ).toInt();
    int height = config.attribute( QString::fromLatin1( "histogramHeigth" ), QString::fromLatin1( "30" ) ).toInt();
    _histogramSize = QSize( QMAX( 15, width ), QMAX( 15, height ) );

    _alignColumns = config.attribute( QString::fromLatin1( "alignColumns" ), QString::fromLatin1( "1" ) ).toInt();
    _rowSpacing = config.attribute( QString::fromLatin1( "rowSpacing" ), QString::fromLatin1( "10" ) ).toInt();

    // Viewer size
    QDesktopWidget* desktop = qApp->desktop();
    QRect rect = desktop->screenGeometry( desktop->primaryScreen() );
    width = config.attribute( QString::fromLatin1( "viewerWidth_%1" ).arg(rect.width()),
                                  QString::fromLatin1( "600" ) ).toInt();
    height = config.attribute( QString::fromLatin1( "viewerHeight_%1" ).arg( rect.width()),
                                   QString::fromLatin1( "450" ) ).toInt();
    _viewerSize = QSize( width, height );

    // Slideshow size
    width = config.attribute( QString::fromLatin1( "slideShowWidth_%1" ).arg(rect.width()),
                                  QString::fromLatin1( "600" ) ).toInt();
    height = config.attribute( QString::fromLatin1( "slideShowHeight_%1" ).arg( rect.width()),
                                   QString::fromLatin1( "450" ) ).toInt();
    _slideShowSize = QSize( width, height );
    _slideShowInterval = config.attribute( QString::fromLatin1( "slideShowInterval" ), QString::fromLatin1( "5" ) ).toInt();

    // Window sizes
    for ( int i = 0; i < LastWindowSize; ++i ) {
        bool ok;
        int w = config.attribute( QString::fromLatin1( "windowWidth-%1" ).arg(i), QString::fromLatin1( "800" )).toInt(&ok);
        if ( !ok )
            w = 800;
        int h = config.attribute( QString::fromLatin1( "windowHeight-%1" ).arg(i), QString::fromLatin1( "600" )).toInt(&ok);
        if ( !ok )
            h = 600;
        _windowSizes[(WindowType) i] = QSize( w,h );
    }

    Util::readOptions( options, &_options, CategoryCollection::instance() );
    createSpecialCategories();

    _configDock = configWindowSetup;
    _members.load( memberGroups );
    _currentLock.load( config );
}

void Options::setThumbSize( int w )
{
    if ( _thumbSize != w )
        emit changed();
    _thumbSize = w;
}

int Options::thumbSize() const
{
    return _thumbSize;
}


void Options::save( QDomElement top )
{
    QDomDocument doc = top.ownerDocument();
    QDomElement config = doc.createElement( QString::fromLatin1( "config" ) );
    top.appendChild( config );

    config.setAttribute( QString::fromLatin1( "version" ), QString::fromLatin1( "1" ) );
    config.setAttribute( QString::fromLatin1("thumbSize"), _thumbSize );
    config.setAttribute( QString::fromLatin1( "previewSize" ), _previewSize );
    config.setAttribute( QString::fromLatin1("trustTimeStamps"), _tTimeStamps );
    config.setAttribute( QString::fromLatin1("useEXIFRotate"), _useEXIFRotate );
    config.setAttribute( QString::fromLatin1("useEXIFComments"), _useEXIFComments );
    config.setAttribute( QString::fromLatin1("autoSave"), _autoSave );
    config.setAttribute( QString::fromLatin1("maxImages" ), _maxImages );
    config.setAttribute( QString::fromLatin1( "ensureImageWindowsOnScreen" ), _ensureImageWindowsOnScreen );
    config.setAttribute( QString::fromLatin1("imageDirectory"), _imageDirectory );
    config.setAttribute( QString::fromLatin1("htmlBaseDir"), _htmlBaseDir );
    config.setAttribute( QString::fromLatin1("htmlBaseURL"), _htmlBaseURL );
    config.setAttribute( QString::fromLatin1("htmlDestURL"), _htmlDestURL );

    config.setAttribute( QString::fromLatin1("infoBoxPosition"), (int) _infoBoxPosition );
    config.setAttribute( QString::fromLatin1("showInfoBox"), _showInfoBox );
    config.setAttribute( QString::fromLatin1("showDrawings"), _showDrawings );
    config.setAttribute( QString::fromLatin1("showDescription"), _showDescription );
    config.setAttribute( QString::fromLatin1("showDate"), _showDate );
    config.setAttribute( QString::fromLatin1("showTime"), _showTime );
    config.setAttribute( QString::fromLatin1("locked"), _locked );
    config.setAttribute( QString::fromLatin1("exclude"), _exclude );
    config.setAttribute( QString::fromLatin1("passwd"), _passwd );
    config.setAttribute( QString::fromLatin1( "albumCategory" ), _albumCategory );
    config.setAttribute( QString::fromLatin1( "viewSortTye" ), _viewSortType );
    config.setAttribute( QString::fromLatin1( "fromDate" ), _fromDate.toString( Qt::ISODate ) );
    config.setAttribute( QString::fromLatin1( "toDate" ), _toDate.toString( Qt::ISODate ) );
    config.setAttribute( QString::fromLatin1( "slideShowInterval" ), _slideShowInterval );
    config.setAttribute( QString::fromLatin1( "launchViewerFullScreen" ), _launchViewerFullScreen );
    config.setAttribute( QString::fromLatin1( "launchSlideShowFullScreen" ), _launchSlideShowFullScreen );
    config.setAttribute( QString::fromLatin1( "displayLabels" ), _displayLabels );
    config.setAttribute( QString::fromLatin1( "thumbNailBackgroundColor" ), _thumbNailBackgroundColor.name() );
    config.setAttribute( QString::fromLatin1( "viewerCacheSize" ), _viewerCacheSize );
    config.setAttribute( QString::fromLatin1( "searchForImagesOnStartup" ), _searchForImagesOnStartup );
    config.setAttribute( QString::fromLatin1( "autoShowThumbnailViewCount" ), _autoShowThumbnailView );
    config.setAttribute( QString::fromLatin1( "histogramWidth" ), _histogramSize.width() );
    config.setAttribute( QString::fromLatin1( "histogramHeigth" ), _histogramSize.height() );
    config.setAttribute( QString::fromLatin1( "alignColumns" ), _alignColumns );
    config.setAttribute( QString::fromLatin1( "rowSpacing" ), _rowSpacing );

    // Viewer size
    QDesktopWidget* desktop = qApp->desktop();
    QRect rect = desktop->screenGeometry( desktop->primaryScreen() );
    config.setAttribute( QString::fromLatin1( "viewerWidth_%1" ).arg(rect.width()), _viewerSize.width() );
    config.setAttribute( QString::fromLatin1( "viewerHeight_%1" ).arg( rect.width()), _viewerSize.height() );

    // Slide show size
    config.setAttribute( QString::fromLatin1( "slideShowWidth_%1" ).arg(rect.width()), _slideShowSize.width() );
    config.setAttribute( QString::fromLatin1( "slideShowHeight_%1" ).arg( rect.width()), _slideShowSize.height() );

    // Window sizes
    for ( int i = 0; i < LastWindowSize; ++i ) {
        config.setAttribute( QString::fromLatin1( "windowWidth-%1" ).arg(i), _windowSizes[(WindowType)i].width() );
        config.setAttribute( QString::fromLatin1( "windowHeight-%1" ).arg(i), _windowSizes[(WindowType)i].height() );
    }

    QStringList grps = CategoryCollection::instance()->categoryNames();
    QDomElement options = doc.createElement( QString::fromLatin1("options") );
    top.appendChild( options );
    (void) Util::writeOptions( doc, options, _options, CategoryCollection::instance() );

    // Save window layout for config window
    top.appendChild( _configDock );

    // Member Groups
    if ( ! _members.isEmpty() )
        top.appendChild( _members.save( doc ) );

    if ( !_currentLock.isNull() )
        config.appendChild( _currentLock.toXML( doc ) );
}

void Options::setOption( const QString& key, const QStringList& value )
{
    if ( _options[key] != value )
        emit changed();
    _options[key] = value;
}

void Options::removeOption( const QString& key, const QString& value )
{
    emit changed();
    _options[key].remove( value );
    emit deletedOption( key, value );
}

void Options::renameOption( const QString& category, const QString& oldValue, const QString& newValue )
{
    _options[category].remove( oldValue );
    addOption( category, newValue );
    emit renamedOption( category, oldValue, newValue );
}

void Options::addOption( const QString& key, const QString& value )
{
    if ( _options[key].contains( value ) )
        _options[key].remove( value );
    else
        emit changed();
    _options[key].prepend( value );
}

QStringList Options::optionValue( const QString& key ) const
{
    return _options[key];
}

QStringList Options::optionValueInclGroups( const QString& category ) const
{
    // values including member groups

    QStringList items = optionValue( category );
    QStringList itemsAndGroups = QStringList::QStringList();
    for( QStringList::Iterator it = items.begin(); it != items.end(); ++it ) {
        itemsAndGroups << *it ;
    };
    // add the groups to the listbox too, but only if the group is not there already, which will be the case
    // if it has ever been selected once.
    QStringList groups = _members.groups( category );
    for( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it ) {
        if ( ! items.contains(  *it ) )
            itemsAndGroups << *it ;
    };
    if ( viewSortType() == SortAlpha )
        itemsAndGroups.sort();
    return itemsAndGroups;
}


bool Options::trustTimeStamps()
{
    if ( _tTimeStamps == Always )
        return true;
    else if ( _tTimeStamps == Never )
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
void Options::setTTimeStamps( TimeStampTrust t )
{
    if ( _tTimeStamps != t )
        emit changed();
    _tTimeStamps = t;
}

Options::TimeStampTrust Options::tTimeStamps() const
{
    return _tTimeStamps;
}

QString Options::imageDirectory() const
{
    if ( !_imageDirectory.endsWith( QString::fromLatin1( "/" ) ) )
        return _imageDirectory + QString::fromLatin1( "/" );
    else
        return _imageDirectory;
}

bool Options::showInfoBox() const
{
    return _showInfoBox;
}

bool Options::showDrawings() const
{
    return _showDrawings;
}

bool Options::showDescription() const
{
    return _showDescription;
}

bool Options::showDate() const
{
    return _showDate;
}

bool Options::showTime() const
{
    return _showTime;
}

void Options::setShowInfoBox(bool b)
{
    if ( _showInfoBox != b ) emit changed();
    _showInfoBox = b;
}

void Options::setShowDrawings(bool b)
{
    if ( _showDrawings != b ) emit changed();
    _showDrawings = b;
}

void Options::setShowDescription(bool b)
{
    if ( _showDescription != b ) emit changed();
    _showDescription = b;
}

void Options::setShowDate(bool b)
{
    if ( _showDate != b ) emit changed();
    _showDate = b;
}

void Options::setShowTime(bool b)
{
    if ( _showTime != b ) emit changed();
    _showTime = b;
}

Options::Position Options::infoBoxPosition() const
{
    return _infoBoxPosition;
}

void Options::setInfoBoxPosition( Position pos )
{
    if ( _infoBoxPosition != pos ) emit changed();
    _infoBoxPosition = pos;
}

/**
   Returns whether the given category is shown in the viewer.
*/
bool Options::showOption( const QString& category ) const
{
    return CategoryCollection::instance()->categoryForName(category)->doShow();
}

void Options::setShowOption( const QString& category, bool b )
{
    if ( CategoryCollection::instance()->categoryForName(category)->doShow() != b ) emit changed();
    CategoryCollection::instance()->categoryForName(category)->setDoShow( b );
}

QString Options::HTMLBaseDir() const
{
    return _htmlBaseDir;
}

void Options::setHTMLBaseDir( const QString& dir )
{
    if ( _htmlBaseDir != dir ) emit changed();
    _htmlBaseDir = dir;
}

QString Options::HTMLBaseURL() const
{
    return _htmlBaseURL;
}

void Options::setHTMLBaseURL( const QString& url )
{
    if ( _htmlBaseURL != url ) emit changed();
    _htmlBaseURL = url;
}

QString Options::HTMLDestURL() const
{
    return _htmlDestURL;
}

void Options::setHTMLDestURL( const QString& url )
{
    if ( _htmlDestURL != url ) emit changed();
    _htmlDestURL = url;
}

void Options::setAutoSave( int min )
{
    if ( _autoSave != min ) emit changed();
    _autoSave = min;
}

int Options::autoSave() const
{
    return _autoSave;
}

void Options::setMaxImages( int i )
{
    if ( _maxImages != i ) emit changed();
    _maxImages = i;
}

int Options::maxImages() const
{
    return 10000000;
#ifdef TEMPORARILY_REMOVED
    return _maxImages;
#endif
}


void Options::saveConfigWindowLayout( ImageConfig* config )
{
    QDomDocument doc;
    _configDock = doc.createElement( QString::fromLatin1("configWindowSetup" ) );
    config->writeDockConfig( _configDock );
    emit changed();
}


void Options::loadConfigWindowLayout( ImageConfig* config )
{
    config->readDockConfig( _configDock );
}

void Options::setup( const QDomElement& config, const QDomElement& options,
                     const QDomElement& configWindowSetup, const QDomElement& memberGroups,
                     const QString& imageDirectory )
{
    _instance = new Options( config, options, configWindowSetup, memberGroups, imageDirectory );
}

void Options::setViewerSize( const QSize& size )
{
    if ( size != _viewerSize )
        emit changed();

    _viewerSize = size;
}

QSize Options::viewerSize() const
{
    return _viewerSize;
}

void Options::setSlideShowSize( const QSize& size )
{
    if ( size != _slideShowSize )
        emit changed();

    _slideShowSize = size;
}

QSize Options::slideShowSize() const
{
    return _slideShowSize;
}



const MemberMap& Options::memberMap()
{
    return _members;
}

void Options::setMemberMap( const MemberMap& members )
{
    // In a perfect world, I should check if _members != members, and only emit changed in that case.
    emit changed();
    _members = members;
}

void Options::setCurrentLock( const ImageSearchInfo& info, bool exclude )
{
    _currentLock = info;
    _exclude = exclude;
}

ImageSearchInfo Options::currentLock() const
{
    return _currentLock;
}

void Options::setLocked( bool lock )
{
    _locked = lock;
    emit locked( lock, _exclude );
}

bool Options::isLocked() const
{
    return _locked;
}

bool Options::lockExcludes() const
{
    return _exclude;
}

void Options::setPassword( const QString& passwd )
{
    _passwd = passwd;
}

QString Options::password() const
{
    return _passwd;
}

QString Options::fileForCategoryImage( const QString& category, QString member ) const
{
    QString dir = imageDirectory() + QString::fromLatin1("CategoryImages" );
    member.replace( ' ', '_' );
    QString fileName = dir + QString::fromLatin1("/%1-%2.jpg").arg( category ).arg( member );
    return fileName;
}


void Options::setOptionImage( const QString& category, QString member, const QImage& image )
{
    QString dir = imageDirectory() + QString::fromLatin1("CategoryImages" );
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
}

QImage Options::optionImage( const QString& category, QString member, int size ) const
{
    QString fileName = fileForCategoryImage( category, member );
    QImage img;
    bool ok = img.load( fileName, "JPEG" );
    if ( ! ok ) {
        if ( Options::instance()->memberMap().isGroup( category, member ) )
            img = KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kuser" ), KIcon::Desktop, size );
        else
            img = CategoryCollection::instance()->categoryForName( category )->icon( size );
    }
    return img.smoothScale( size, size, QImage::ScaleMin );
}

void Options::setUseEXIFRotate( bool b )
{
    if ( _useEXIFRotate != b )
        emit changed();
    _useEXIFRotate = b;
}

bool Options::useEXIFRotate() const
{
    return _useEXIFRotate;
}

void Options::setUseEXIFComments( bool b )
{
    if ( _useEXIFComments != b )
        emit changed();
    _useEXIFComments = b;
}

bool Options::useEXIFComments() const
{
    return _useEXIFComments;
}

void Options::setPreviewSize( int size )
{
    if ( _previewSize != size )
        emit changed();
    _previewSize = size;
}

int Options::previewSize() const
{
    return _previewSize;
}

void Options::setViewSortType( ViewSortType tp )
{
    if ( _viewSortType != tp ) {
        _viewSortType = tp;
        emit viewSortTypeChanged( tp );
        emit changed();
    }
}

Options::ViewSortType Options::viewSortType() const
{
    return _viewSortType;
}

void Options::setFromDate( const QDate& date)
{
    if (date.isValid())
        _fromDate = date;
}

QDate Options::fromDate() const
{
    return _fromDate;
}

void  Options::setToDate( const QDate& date)
{
    if (date.isValid())
	_toDate = date;
}

QDate Options::toDate() const
{
    return _toDate;
}

void Options::setSlideShowInterval( int interval )
{
    if (_slideShowInterval != interval ) {
        _slideShowInterval = interval;
        emit changed();
    }
}

int Options::slideShowInterval() const
{
    return _slideShowInterval;
}

QString Options::albumCategory() const
{
    if ( !CategoryCollection::instance()->categoryNames().contains( _albumCategory ) )
        const_cast<Options*>(this)->_albumCategory = CategoryCollection::instance()->categoryNames()[0];
    return _albumCategory;
}

void Options::setAlbumCategory( const QString& category )
{
    if (_albumCategory != category ) {
        _albumCategory = category;
        emit changed();
    }
}

void Options::setLaunchViewerFullScreen( bool b )
{
    if (_launchViewerFullScreen != b ) {
        _launchViewerFullScreen = b;
        emit changed();
    }
}

bool Options::launchViewerFullScreen() const
{
    return _launchViewerFullScreen;
}

void Options::setLaunchSlideShowFullScreen( bool b )
{
    if ( _launchSlideShowFullScreen != b ) {
        _launchSlideShowFullScreen = b;
        emit changed();
    }
}

bool Options::launchSlideShowFullScreen() const
{
    return _launchSlideShowFullScreen;
}

void Options::setDisplayLabels( bool b )
{
    if (_displayLabels != b ) {
        _displayLabels = b;
        emit changed();
    }
}

bool Options::displayLabels() const
{
    return _displayLabels;
}

void Options::setThumbNailBackgroundColor( const QColor& col )
{
    if ( _thumbNailBackgroundColor != col ) {
        _thumbNailBackgroundColor = col;
        emit changed();
    }
}

QColor Options::thumbNailBackgroundColor() const
{
    return _thumbNailBackgroundColor;
}

void Options::setWindowSize( WindowType win, const QSize& size )
{
    if ( _windowSizes[win] != size ) {
        _windowSizes[win] = size;
        emit changed();
    }
}

QSize Options::windowSize( WindowType win ) const
{
    return _windowSizes[win];
}

bool Options::ready()
{
    return _instance != 0;
}

int Options::viewerCacheSize() const
{
    return _viewerCacheSize;
}

void Options::setViewerCacheSize( int size )
{
    if ( _viewerCacheSize != size ) {
        _viewerCacheSize = size;
        emit changed();
    }
}

bool Options::searchForImagesOnStartup() const
{
    return _searchForImagesOnStartup;
}

void Options::setSearchForImagesOnStartup(bool b)
{
    if ( b != _searchForImagesOnStartup ) {
        _searchForImagesOnStartup = b;
        emit changed();
    }
}

int Options::autoShowThumbnailView() const
{
    return _autoShowThumbnailView;
}

void Options::setAutoShowThumbnailView( int val )
{
    if ( val != _autoShowThumbnailView ) {
        _autoShowThumbnailView = val;
        emit changed();
    }
}

void Options::createSpecialCategories()
{
    Category* folderCat = CategoryCollection::instance()->categoryForName( QString::fromLatin1( "Folder" ) );
    if( folderCat == 0 ) {
        _options.insert( QString::fromLatin1("Folder"), QStringList() );
        folderCat = new Category( QString::fromLatin1("Folder"), QString::fromLatin1("folder"), Category::Small, Category::ListView, false );
        CategoryCollection::instance()->addCategory( folderCat );
    }
    folderCat->setSpecialCategory( true );


    Category* tokenCat = CategoryCollection::instance()->categoryForName( QString::fromLatin1( "Tokens" ) );
    if ( !tokenCat ) {
        _options.insert( QString::fromLatin1("Tokens"), QStringList() );
        tokenCat = new Category( QString::fromLatin1("Tokens"), QString::fromLatin1("cookie"), Category::Small, Category::ListView, true );
        CategoryCollection::instance()->addCategory( tokenCat );
    }
    tokenCat->setSpecialCategory( true );
}

QSize Options::histogramSize() const
{
    return _histogramSize;
}

void Options::setHistogramSize( const QSize& size )
{
    if ( _histogramSize != size ) {
        emit changed();
        emit histogramSizeChanged( size );
    }

    _histogramSize = size;
}

bool Options::alignColumns() const
{
    return _alignColumns;
}

void Options::setAlignColumns( bool b )
{
    if ( _alignColumns != b ) {
        _alignColumns = b;
        emit changed();
    }
}

int Options::rowSpacing() const
{
    return _rowSpacing;
}

void Options::setRowSpacing( int i )
{
    if ( _rowSpacing != i ) {
        _rowSpacing = i;
        emit changed();
    }
}

#include "options.moc"
