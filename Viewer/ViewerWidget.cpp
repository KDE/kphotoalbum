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

#include <kdeversion.h>
#include "Viewer/ViewerWidget.h"
#include <qlayout.h>
#include <qlabel.h>
#include "DB/ImageInfo.h"
#include "ImageManager/Manager.h"
#include <qsizepolicy.h>
#include <qsimplerichtext.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qrect.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include "DisplayArea.h"
#include <qtoolbar.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kaction.h>
#include <klocale.h>
#include "Utilities/Util.h"
#include <qsignalmapper.h>
#include "ShowOptionAction.h"
#include <qtimer.h>
#include "DrawHandler.h"
#include <kwin.h>
#include <kglobalsettings.h>
#include "SpeedDisplay.h"
#include <qdesktopwidget.h>
#include "MainWindow/Window.h"
#include <qdatetime.h>
#include "CategoryImageConfig.h"
#include <dcopref.h>
#include "MainWindow/ExternalPopup.h"
#include <kaccel.h>
#include <kkeydialog.h>
#include <kapplication.h>
#include <kglobal.h>
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "InfoBox.h"
#include <qwidgetstack.h>
#include "VideoDisplay.h"
#include "ViewerWidget.h"


/**
 * \namespace Viewer
 * \brief Viewer used for displaying images and movies
 *
 * This class implements the viewer used to display images and movies.
 *
 * The class consists of these components:
 * <ul>
 * <li>\ref ViewerWidget - This is the topmost widget used as the viewer.
 * <li>\ref ImageDisplay -
 * // PENDING(blackie) Fill in the rest of the description.
 * </ul>
 */
Viewer::ViewerWidget* Viewer::ViewerWidget::_latest = 0;

Viewer::ViewerWidget* Viewer::ViewerWidget::latest()
{
    return _latest;
}


// Notice the parent is zero to allow other windows to come on top of it.
Viewer::ViewerWidget::ViewerWidget( const char* name )
    :QWidget( 0,  name, WType_TopLevel ), _current(0), _popup(0), _showingFullScreen( false ), _forward( true ), _isRunningSlideShow( false )
{
    setWFlags( WDestructiveClose );
    _latest = this;

    QVBoxLayout* layout = new QVBoxLayout( this );

    _stack = new QWidgetStack( this, "stack" );

    _display = _imageDisplay = new ImageDisplay( _stack ); // Must be created before the toolbar.
    _videoDisplay = new VideoDisplay( _stack );
    connect( _videoDisplay, SIGNAL( stopped() ), this, SLOT( videoStopped() ) );

    connect( _imageDisplay, SIGNAL( possibleChange() ), this, SLOT( updateCategoryConfig() ) );
    createToolBar();
    _toolbar->hide();


    layout->addWidget( _toolbar );
    layout->addWidget( _stack );

    // This must not be added to the layout, as it is standing on top of
    // the ImageDisplay
    _infoBox = new InfoBox( this );
    _infoBox->setShown( Settings::SettingsData::instance()->showInfoBox() );

    setupContextMenu();

    _slideShowTimer = new QTimer( this );
    _slideShowPause = Settings::SettingsData::instance()->slideShowInterval() * 1000;
    connect( _slideShowTimer, SIGNAL( timeout() ), this, SLOT( slotSlideShowNext() ) );
    _speedDisplay = new SpeedDisplay( this );

    setFocusPolicy( StrongFocus );
}


void Viewer::ViewerWidget::setupContextMenu()
{
    _popup = new QPopupMenu( this, "context popup menu" );
    _actions = new KActionCollection( this, "viewer", KGlobal::instance() );
    KAction* action;

    _firstAction = new KAction( i18n("First"), Key_Home, this, SLOT( showFirst() ), _actions, "viewer-home" );
    _firstAction->plug( _popup );

    _lastAction = new KAction( i18n("Last"), Key_End, this, SLOT( showLast() ), _actions, "viewer-end" );
    _lastAction->plug( _popup );

    _nextAction = new KAction( i18n("Show Next"), Key_PageDown, this, SLOT( showNext() ), _actions, "viewer-next" );
    _nextAction->plug( _popup );

    _prevAction = new KAction( i18n("Show Previous"), Key_PageUp, this, SLOT( showPrev() ), _actions, "viewer-prev" );
    _prevAction->plug( _popup );

    _popup->insertSeparator();

    _startStopSlideShow = new KAction( i18n("Run Slideshow"), CTRL+Key_R, this, SLOT( slotStartStopSlideShow() ),
                                       _actions, "viewer-start-stop-slideshow" );
    _startStopSlideShow->plug( _popup );

    _slideShowRunFaster = new KAction( i18n("Run Faster"), CTRL + Key_Plus, this, SLOT( slotSlideShowFaster() ),
                                       _actions, "viewer-run-faster" );
    _slideShowRunFaster->plug( _popup );

    _slideShowRunSlower = new KAction( i18n("Run Slower"), CTRL+Key_Minus, this, SLOT( slotSlideShowSlower() ),
                                       _actions, "viewer-run-slower" );
    _slideShowRunSlower->plug( _popup );

    _popup->insertSeparator();

    // PENDING(blackie) Only for image display?
    action = new KAction( i18n("Zoom In"), Key_Plus, _display, SLOT( zoomIn() ), _actions, "viewer-zoom-in" );
    action->plug( _popup );

    action = new KAction( i18n("Zoom Out"), Key_Minus, _display, SLOT( zoomOut() ), _actions, "viewer-zoom-out" );
    action->plug( _popup );

    action = new KAction( i18n("Full View"), Key_Period, _display, SLOT( zoomFull() ), _actions, "viewer-zoom-full" );
    action->plug( _popup );

    action = new KAction( i18n("Toggle Full Screen"), Key_Return, this, SLOT( toggleFullScreen() ),
                          _actions, "viewer-toggle-fullscreen" );
    action->plug( _popup );

    _popup->insertSeparator();

    action = new KAction( i18n("Rotate 90 Degrees"), Key_9, this, SLOT( rotate90() ), _actions, "viewer-rotate90" );
    action->plug( _popup );

    action = new KAction( i18n("Rotate 180 Degrees"), Key_8, this, SLOT( rotate180() ), _actions, "viewer-rotate180" );
    action->plug( _popup );

    action = new KAction( i18n("Rotate 270 Degrees"), Key_7, this, SLOT( rotate270() ), _actions, "viewer-rotare270" );
    action->plug( _popup );

    _popup->insertSeparator();

    KToggleAction* taction = new KToggleAction( i18n("Show Info Box"), CTRL+Key_I, _actions, "viewer-show-infobox" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowInfoBox( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Settings::SettingsData::instance()->showInfoBox() );

    // PENDING(blackie) Only for image display
    taction = new KToggleAction( i18n("Show Drawing"), CTRL+Key_D, _actions, "viewer-show-drawing");
    connect( taction, SIGNAL( toggled( bool ) ), _display, SLOT( toggleShowDrawings( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Settings::SettingsData::instance()->showDrawings() );

    taction = new KToggleAction( i18n("Show Description"), 0, _actions, "viewer-show-description" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDescription( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Settings::SettingsData::instance()->showDescription() );

    taction = new KToggleAction( i18n("Show Date"), 0, _actions, "viewer-show-date" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDate( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Settings::SettingsData::instance()->showDate() );

    taction = new KToggleAction( i18n("Show Time"), 0, _actions, "viewer-show-time" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowTime( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Settings::SettingsData::instance()->showTime() );

    taction = new KToggleAction( i18n("Show EXIF"), 0, _actions, "viewer-show-exif" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowEXIF( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Settings::SettingsData::instance()->showEXIF() );

    QValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        ShowOptionAction* action = new ShowOptionAction( (*it)->name(), this );
        action->plug( _popup );
        connect( action, SIGNAL( toggled( const QString&, bool ) ),
                 this, SLOT( toggleShowOption( const QString&, bool ) ) );
    }

    _popup->insertSeparator();

    // -------------------------------------------------- Wall paper
    QPopupMenu *wallpaperPopup = new QPopupMenu( _popup, "context popup menu" );

    action = new KAction( i18n("Centered"), 0, this, SLOT( slotSetWallpaperC() ), wallpaperPopup, "viewer-centered" );
    action->plug( wallpaperPopup );

    action = new KAction( i18n("Tiled"), 0, this, SLOT( slotSetWallpaperT() ), wallpaperPopup, "viewer-tiled" );
    action->plug( wallpaperPopup );

    action = new KAction( i18n("Center Tiled"), 0, this, SLOT( slotSetWallpaperCT() ), wallpaperPopup, "viewer-center-tiled" );
    action->plug( wallpaperPopup );

    action = new KAction( i18n("Centered Maxpect"), 0, this, SLOT( slotSetWallpaperCM() ),
                          wallpaperPopup, "viewer-centered-maxspect" );
    action->plug( wallpaperPopup );

    action = new KAction( i18n("Tiled Maxpect"), 0, this, SLOT( slotSetWallpaperTM() ),
                          wallpaperPopup, "viewer-tiled-maxpect" );
    action->plug( wallpaperPopup );

    action = new KAction( i18n("Scaled"), 0, this, SLOT( slotSetWallpaperS() ), wallpaperPopup, "viewer-scaled" );
    action->plug( wallpaperPopup );

    action = new KAction( i18n("Centered Auto Fit"), 0, this, SLOT( slotSetWallpaperCAF() ),
                          wallpaperPopup, "viewer-centered-auto-fit" );
    action->plug( wallpaperPopup );

    _popup->insertItem( QIconSet(), i18n("Set as Wallpaper"), wallpaperPopup );

    // -------------------------------------------------- Invoke external program
    _externalPopup = new MainWindow::ExternalPopup( _popup );
    _popup->insertItem( QIconSet(), i18n("Invoke External Program"), _externalPopup );
    connect( _externalPopup, SIGNAL( aboutToShow() ), this, SLOT( populateExternalPopup() ) );


    action = new KAction( i18n("Draw on Image"),  0, this, SLOT( startDraw() ), this, "viewer-draw-on-image" );
    action->plug( _popup );

    action = new KAction( i18n("Edit Image Properties..."),  CTRL+Key_1, this, SLOT( editImage() ),
                          _actions, "viewer-edit-image-properties" );
    action->plug( _popup );

    // PENDING(blackie) This should only be enabled for image displays.
    action = new KAction( i18n("Show Category Editor"), 0, this, SLOT( makeCategoryImage() ),
                          _actions, "viewer-show-category-editor" );
    action->plug( _popup );

    action = new KAction( i18n("Close"), Key_Escape, this, SLOT( close() ), _actions, "viewer-close" );
    action->plug( _popup );
    _actions->readShortcutSettings();
}

void Viewer::ViewerWidget::load( const QStringList& list, int index )
{
    _list = list;

    QStringList images;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        if ( !Utilities::isMovie( *it ) )
            images << *it;
    }
    _imageDisplay->setImageList( list );

    _current = index;
    load();

    bool on = ( list.count() > 1 );
    _startStopSlideShow->setEnabled(on);
    _slideShowRunFaster->setEnabled(on);
    _slideShowRunSlower->setEnabled(on);
}

void Viewer::ViewerWidget::load()
{
    bool isVideo = Utilities::isMovie( currentInfo()->fileName() );
    if ( isVideo )
        _display = _videoDisplay;
    else
        _display = _imageDisplay;

    _stack->raiseWidget( _display );

    if ( _display->offersDrawOnImage() )
        _display->drawHandler()->setDrawList( currentInfo()->drawList() );
    _display->setImage( currentInfo(), _forward );
    setCaption( QString::fromLatin1( "KPhotoAlbum - %1" ).arg( currentInfo()->fileName() ) );
    updateInfoBox();

    _nextAction->setEnabled( _current +1 < (int) _list.count() );
    _prevAction->setEnabled( _current > 0 );
    _firstAction->setEnabled( _current > 0 );
    _lastAction->setEnabled( _current +1 < (int) _list.count() );
    if ( isVideo )
        updateCategoryConfig();

    if ( _isRunningSlideShow )
        _slideShowTimer->changeInterval( _slideShowPause );
}

void Viewer::ViewerWidget::contextMenuEvent( QContextMenuEvent * e )
{
    _popup->exec( e->globalPos() );
    e->accept();
}

void Viewer::ViewerWidget::showNext()
{
    save();
    if ( _current +1 < (int) _list.count() )  {
        _current++;
        _forward = true;
        load();
    }
}

void Viewer::ViewerWidget::showPrev()
{
    save();
    if ( _current > 0  )  {
        _current--;
        _forward = false;
        load();
    }
}

void Viewer::ViewerWidget::rotate90()
{
    currentInfo()->rotate( 90 );
    load();
}

void Viewer::ViewerWidget::rotate180()
{
    currentInfo()->rotate( 180 );
    load();
}

void Viewer::ViewerWidget::rotate270()
{
    currentInfo()->rotate( 270 );
    load();
}

void Viewer::ViewerWidget::toggleShowInfoBox( bool b )
{
    Settings::SettingsData::instance()->setShowInfoBox( b );
    _infoBox->setShown(b);
    updateInfoBox();
}

void Viewer::ViewerWidget::toggleShowDescription( bool b )
{
    Settings::SettingsData::instance()->setShowDescription( b );
    updateInfoBox();
}

void Viewer::ViewerWidget::toggleShowDate( bool b )
{
    Settings::SettingsData::instance()->setShowDate( b );
    updateInfoBox();
}

void Viewer::ViewerWidget::toggleShowTime( bool b )
{
    Settings::SettingsData::instance()->setShowTime( b );
    updateInfoBox();
}

void Viewer::ViewerWidget::toggleShowEXIF( bool b )
{
    Settings::SettingsData::instance()->setShowEXIF( b );
    updateInfoBox();
}


void Viewer::ViewerWidget::toggleShowOption( const QString& category, bool b )
{
    DB::ImageDB::instance()->categoryCollection()->categoryForName(category)->setDoShow( b );
    updateInfoBox();
}

void Viewer::ViewerWidget::showFirst()
{
    _forward = true;
    save();
    _current = 0;
    load();
}

void Viewer::ViewerWidget::showLast()
{
    _forward = false;
    save();
     _current = _list.count() -1;
     load();
}

void Viewer::ViewerWidget::save()
{
    if ( _display->offersDrawOnImage() )
        currentInfo()->setDrawList( _display->drawHandler()->drawList() );
}

void Viewer::ViewerWidget::startDraw()
{
    Q_ASSERT( _display->offersDrawOnImage() );
    _display->startDrawing();
    _display->drawHandler()->slotSelect();
    _toolbar->show();
}

void Viewer::ViewerWidget::stopDraw()
{
    Q_ASSERT( _display->offersDrawOnImage() );
    _display->stopDrawing();
    _toolbar->hide();
}

void Viewer::ViewerWidget::slotSetWallpaperC()
{
    setAsWallpaper(1);
}

void Viewer::ViewerWidget::slotSetWallpaperT()
{
    setAsWallpaper(2);
}

void Viewer::ViewerWidget::slotSetWallpaperCT()
{
    setAsWallpaper(3);
}

void Viewer::ViewerWidget::slotSetWallpaperCM()
{
    setAsWallpaper(4);
}

void Viewer::ViewerWidget::slotSetWallpaperTM()
{
    setAsWallpaper(5);
}

void Viewer::ViewerWidget::slotSetWallpaperS()
{
    setAsWallpaper(6);
}

void Viewer::ViewerWidget::slotSetWallpaperCAF()
{
    setAsWallpaper(7);
}

void Viewer::ViewerWidget::setAsWallpaper(int mode)
{
    if(mode>7 || mode<1) return;
    DCOPRef kdesktop("kdesktop","KBackgroundIface");
    kdesktop.send("setWallpaper(QString,int)",currentInfo()->fileName(0),mode);
}

bool Viewer::ViewerWidget::close( bool alsoDelete)
{
    save();
    _slideShowTimer->stop();
    _isRunningSlideShow = false;
    return QWidget::close( alsoDelete );
}

DB::ImageInfoPtr Viewer::ViewerWidget::currentInfo()
{
    return DB::ImageDB::instance()->info(_list[ _current]); // PENDING(blackie) can we postpone this lookup?
}

void Viewer::ViewerWidget::infoBoxMove()
{
    QPoint p = mapFromGlobal( QCursor::pos() );
    Settings::Position oldPos = Settings::SettingsData::instance()->infoBoxPosition();
    Settings::Position pos = oldPos;
    int x = _display->mapFromParent( p ).x();
    int y = _display->mapFromParent( p ).y();
    int w = _display->width();
    int h = _display->height();

    if ( x < w/3 )  {
        if ( y < h/3  )
            pos = Settings::TopLeft;
        else if ( y > h*2/3 )
            pos = Settings::BottomLeft;
        else
            pos = Settings::Left;
    }
    else if ( x > w*2/3 )  {
        if ( y < h/3  )
            pos = Settings::TopRight;
        else if ( y > h*2/3 )
            pos = Settings::BottomRight;
        else
            pos = Settings::Right;
    }
    else {
        if ( y < h/3  )
            pos = Settings::Top;
            else if ( y > h*2/3 )
                pos = Settings::Bottom;
    }
    if ( pos != oldPos )  {
        Settings::SettingsData::instance()->setInfoBoxPosition( pos );
        updateInfoBox();
    }
}

void Viewer::ViewerWidget::moveInfoBox()
{
    _infoBox->setSize();
    Settings::Position pos = Settings::SettingsData::instance()->infoBoxPosition();

    int lx = _display->pos().x();
    int ly = _display->pos().y();
    int lw = _display->width();
    int lh = _display->height();

    int bw = _infoBox->width();
    int bh = _infoBox->height();

    int bx, by;
    // x-coordinate
    if ( pos == Settings::TopRight || pos == Settings::BottomRight || pos == Settings::Right )
        bx = lx+lw-5-bw;
    else if ( pos == Settings::TopLeft || pos == Settings::BottomLeft || pos == Settings::Left )
        bx = lx+5;
    else
        bx = lx+lw/2-bw/2;


    // Y-coordinate
    if ( pos == Settings::TopLeft || pos == Settings::TopRight || pos == Settings::Top )
        by = ly+5;
    else if ( pos == Settings::BottomLeft || pos == Settings::BottomRight || pos == Settings::Bottom )
        by = ly+lh-5-bh;
    else
        by = ly+lh/2-bh/2;

    _infoBox->move(bx,by);
}

void Viewer::ViewerWidget::resizeEvent( QResizeEvent* e )
{
    moveInfoBox();
    QWidget::resizeEvent( e );
}

void Viewer::ViewerWidget::updateInfoBox()
{
    if ( currentInfo() ) {
        QMap<int, QPair<QString,QString> > map;
        QString origText = Utilities::createInfoText( currentInfo(), &map );
        QString text = QString::fromLatin1("<qt>") + origText + QString::fromLatin1("</qt>");
        if ( Settings::SettingsData::instance()->showInfoBox() && !origText.isNull() ) {
            _infoBox->setInfo( text, map );
            _infoBox->show();
        }
        else
            _infoBox->hide();

        moveInfoBox();
    }
}

Viewer::ViewerWidget::~ViewerWidget()
{
    if ( _latest == this )
        _latest = 0;
}

void Viewer::ViewerWidget::createToolBar()
{
    KIconLoader loader;
    KActionCollection* actions = new KActionCollection( this, "actions" );
    _toolbar = new KToolBar( this );
#ifdef TEMPORARILY_REMOVED
    DrawHandler* handler = _display->drawHandler();
    _select = new KToggleAction( i18n("Select"), loader.loadIcon(QString::fromLatin1("selecttool"), KIcon::Toolbar),
                         0, handler, SLOT( slotSelect() ),actions, "_select");
    _select->plug( _toolbar );
    _select->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _line = new KToggleAction( i18n("Line"), loader.loadIcon(QString::fromLatin1("linetool"), KIcon::Toolbar),
                         0, handler, SLOT( slotLine() ),actions, "_line");
    _line->plug( _toolbar );
    _line->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _rect = new KToggleAction( i18n("Rectangle"), loader.loadIcon(QString::fromLatin1("recttool"), KIcon::Toolbar),
                         0, handler, SLOT( slotRectangle() ),actions, "_rect");
    _rect->plug( _toolbar );
    _rect->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _circle = new KToggleAction( i18n("Circle"), loader.loadIcon(QString::fromLatin1("ellipsetool"), KIcon::Toolbar),
                           0, handler, SLOT( slotCircle() ),actions, "_circle");
    _circle->plug( _toolbar );
    _circle->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _delete = KStdAction::cut( handler, SLOT( cut() ), actions, "cutAction" );
    _delete->plug( _toolbar );

    KAction* close = KStdAction::close( this,  SLOT( stopDraw() ),  actions,  "stopDraw" );
    close->plug( _toolbar );
#endif
}

void Viewer::ViewerWidget::toggleFullScreen()
{
    setShowFullScreen( !_showingFullScreen );
}

void Viewer::ViewerWidget::slotStartStopSlideShow()
{
    bool wasRunningSlideShow = _isRunningSlideShow;
    _isRunningSlideShow = !_isRunningSlideShow;

    if ( wasRunningSlideShow ) {
        _slideShowTimer->stop();
        _speedDisplay->end();
    }
    else {
        if ( currentInfo()->mediaType() != DB::Movie )
            _slideShowTimer->start( _slideShowPause, true );
        _speedDisplay->start();
    }
}

void Viewer::ViewerWidget::slotSlideShowNext()
{
    _forward = true;
    save();
    if ( _current +1 < (int) _list.count() )
        _current++;
    else
        _current = 0;

    // Load the next images.
    QTime timer;
    timer.start();
    load();

    // If it is a movie, then don't schedule the next image, as this would make the video stop abruptly.
    if ( currentInfo()->mediaType() != DB::Movie ) {
        // ensure that there is a few milliseconds pause, so that an end slideshow keypress
        // can get through immediately, we don't want it to queue up behind a bunch of timer events,
        // which loaded a number of new images before the slideshow stops
        int ms = QMAX( 200, _slideShowPause - timer.elapsed() );
        _slideShowTimer->start( ms, true );
    }
}

void Viewer::ViewerWidget::slotSlideShowFaster()
{
    _slideShowPause -= 500;
    if ( _slideShowPause < 500 )
        _slideShowPause = 500;
    _speedDisplay->display( _slideShowPause );
    if (_slideShowTimer->isActive() )
        _slideShowTimer->changeInterval( _slideShowPause );
}

void Viewer::ViewerWidget::slotSlideShowSlower()
{
    _slideShowPause += 500;
    _speedDisplay->display( _slideShowPause );
    if (_slideShowTimer->isActive() )
        _slideShowTimer->changeInterval( _slideShowPause );
}

void Viewer::ViewerWidget::editImage()
{
    DB::ImageInfoList list;
    list.append( currentInfo() );
    MainWindow::Window::configureImages( list, true );
}

bool Viewer::ViewerWidget::showingFullScreen() const
{
    return _showingFullScreen;
}

void Viewer::ViewerWidget::setShowFullScreen( bool on )
{
    if ( on ) {
        KWin::setState( winId(), NET::FullScreen );
        moveInfoBox();
    }
    else {
        // We need to size the image when going out of full screen, in case we started directly in full screen
        //
        KWin::clearState( winId(), NET::FullScreen );
        if ( !_sized ) {
            resize( Settings::SettingsData::instance()->viewerSize() );
            _sized = true;
        }
    }
    _showingFullScreen = on;
}

void Viewer::ViewerWidget::makeCategoryImage()
{
    CategoryImageConfig::instance()->setCurrentImage( _imageDisplay->currentViewAsThumbnail(), currentInfo() );
    CategoryImageConfig::instance()->show();
}

void Viewer::ViewerWidget::updateCategoryConfig()
{
    CategoryImageConfig::instance()->setCurrentImage( _imageDisplay->currentViewAsThumbnail(), currentInfo() );
}


void Viewer::ViewerWidget::populateExternalPopup()
{
    _externalPopup->populate( currentInfo(), _list );
}

void Viewer::ViewerWidget::show( bool slideShow )
{
    QSize size;
    bool fullScreen;
    if ( slideShow ) {
        fullScreen = Settings::SettingsData::instance()->launchSlideShowFullScreen();
        size = Settings::SettingsData::instance()->slideShowSize();
    }
    else {
        fullScreen = Settings::SettingsData::instance()->launchViewerFullScreen();
        size = Settings::SettingsData::instance()->viewerSize();
    }

    if ( fullScreen )
        setShowFullScreen( true );
    else
        resize( size );

    QWidget::show();
    if ( slideShow ) {
        // The info dialog will show up at the wrong place if we call this function directly
        // don't ask me why -  4 Sep. 2004 15:13 -- Jesper K. Pedersen
        QTimer::singleShot(0, this, SLOT(slotStartStopSlideShow()) );
    }
    _sized = !fullScreen;
}

KActionCollection* Viewer::ViewerWidget::actions()
{
    return _actions;
}

void Viewer::ViewerWidget::keyPressEvent( QKeyEvent* event )
{
    if ( event->stateAfter() == 0 && event->state() == 0 && ( event->key() >= Key_A && event->key() <= Key_Z ) ) {
        QString token = event->text().upper().left(1);
        if ( currentInfo()->hasOption( QString::fromLatin1("Tokens"), token ) )
            currentInfo()->removeOption( QString::fromLatin1("Tokens"), token );
        else
            currentInfo()->addOption( QString::fromLatin1("Tokens"), token );
        updateInfoBox();
        emit dirty();
    }
    QWidget::keyPressEvent( event );
}

void Viewer::ViewerWidget::videoStopped()
{
    qDebug(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Video Stopped: %d", _isRunningSlideShow );
    if ( _isRunningSlideShow )
        slotSlideShowNext();
}

#include "ViewerWidget.moc"
