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

#include <kdeversion.h>
#include "viewer.h"
#include <qlayout.h>
#include <qlabel.h>
#include "imageinfo.h"
#include "imagemanager.h"
#include <qsizepolicy.h>
#include <qsimplerichtext.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qrect.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include "displayarea.h"
#include <qtoolbar.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kaction.h>
#include <klocale.h>
#include "util.h"
#include <qsignalmapper.h>
#include "showoptionaction.h"
#include <qtimer.h>
#include "drawhandler.h"
#include <kwin.h>
#include <kglobalsettings.h>
#include "speeddisplay.h"
#include <qdesktopwidget.h>
#include "mainview.h"
#include <qdatetime.h>
#include "categoryimageconfig.h"
#include <dcopref.h>
#include "externalpopup.h"
#include <kaccel.h>
#include <kkeydialog.h>
#include <kapplication.h>
#include <kglobal.h>
#include "categorycollection.h"

Viewer* Viewer::_latest = 0;

Viewer* Viewer::latest()
{
    return _latest;
}


// Notice the parent is zero to allow other windows to come on top of it.
Viewer::Viewer( const char* name )
    :QWidget( 0,  name, WType_TopLevel ), _current(0), _popup(0), _showingFullScreen( false ), _forward( true )
{
    setWFlags( WDestructiveClose );
    _latest = this;

    QVBoxLayout* layout = new QVBoxLayout( this );

    _display = new DisplayArea( this ); // Must be created before the toolbar.
    connect( _display, SIGNAL( possibleChange() ), this, SLOT( updateCategoryConfig() ) );
    createToolBar();
    _toolbar->hide();


    layout->addWidget( _toolbar );
    layout->addWidget( _display );

    // This must not be added to the layout, as it is standing on top of
    // the DisplayArea
    _infoBox = new InfoBox( this );
    _infoBox->setShown( Options::instance()->showInfoBox() );

    setupContextMenu();

    _slideShowTimer = new QTimer( this );
    _slideShowPause = Options::instance()->slideShowInterval() * 1000;
    connect( _slideShowTimer, SIGNAL( timeout() ), this, SLOT( slotSlideShowNext() ) );
    _speedDisplay = new SpeedDisplay( this );

    setFocusPolicy( StrongFocus );
}


void Viewer::setupContextMenu()
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
    taction->setChecked( Options::instance()->showInfoBox() );

    taction = new KToggleAction( i18n("Show Drawing"), CTRL+Key_D, _actions, "viewer-show-drawing");
    connect( taction, SIGNAL( toggled( bool ) ), _display, SLOT( toggleShowDrawings( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Options::instance()->showDrawings() );

    taction = new KToggleAction( i18n("Show Description"), 0, _actions, "viewer-show-description" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDescription( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Options::instance()->showDescription() );

    taction = new KToggleAction( i18n("Show Date"), 0, _actions, "viewer-show-date" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDate( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Options::instance()->showDate() );

    taction = new KToggleAction( i18n("Show Time"), 0, _actions, "viewer-show-time" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowTime( bool ) ) );
    taction->plug( _popup );
    taction->setChecked( Options::instance()->showTime() );

    QValueList<Category*> categories = CategoryCollection::instance()->categories();
    for( QValueList<Category*>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
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
    _externalPopup = new ExternalPopup( _popup );
    _popup->insertItem( QIconSet(), i18n("Invoke External Program"), _externalPopup );
    connect( _externalPopup, SIGNAL( aboutToShow() ), this, SLOT( populateExternalPopup() ) );


    action = new KAction( i18n("Draw on Image"),  0, this, SLOT( startDraw() ), this, "viewer-draw-on-image" );
    action->plug( _popup );

    action = new KAction( i18n("Edit Image Properties..."),  CTRL+Key_1, this, SLOT( editImage() ),
                          _actions, "viewer-edit-image-properties" );
    action->plug( _popup );

    action = new KAction( i18n("Show Category Editor"), 0, this, SLOT( makeCategoryImage() ),
                          _actions, "viewer-show-category-editor" );
    action->plug( _popup );

    action = new KAction( i18n("Close"), Key_Escape, this, SLOT( close() ), _actions, "viewer-close" );
    action->plug( _popup );
    _actions->readShortcutSettings();
}

void Viewer::load( const ImageInfoList& list, int index )
{
    _list = list;
    _display->setImageList( list );
    _current = index;
    load();

    bool on = ( list.count() > 1 );
    _startStopSlideShow->setEnabled(on);
    _slideShowRunFaster->setEnabled(on);
    _slideShowRunSlower->setEnabled(on);
}

void Viewer::load()
{
    _display->drawHandler()->setDrawList( currentInfo()->drawList() );
    _display->setImage( currentInfo(), _forward );
    setCaption( QString::fromLatin1( "KimDaBa - %1" ).arg( currentInfo()->fileName() ) );
    updateInfoBox();

    _nextAction->setEnabled( _current +1 < (int) _list.count() );
    _prevAction->setEnabled( _current > 0 );
    _firstAction->setEnabled( _current > 0 );
    _lastAction->setEnabled( _current +1 < (int) _list.count() );
    updateCategoryConfig();
}

void Viewer::contextMenuEvent( QContextMenuEvent * e )
{
    _popup->exec( e->globalPos() );
    e->accept();
}

void Viewer::showNext()
{
    save();
    if ( _current +1 < (int) _list.count() )  {
        _current++;
        _forward = true;
        load();
    }
}

void Viewer::showPrev()
{
    save();
    if ( _current > 0  )  {
        _current--;
        _forward = false;
        load();
    }
}

void Viewer::rotate90()
{
    currentInfo()->rotate( 90 );
    load();
}

void Viewer::rotate180()
{
    currentInfo()->rotate( 180 );
    load();
}

void Viewer::rotate270()
{
    currentInfo()->rotate( 270 );
    load();
}

void Viewer::toggleShowInfoBox( bool b )
{
    Options::instance()->setShowInfoBox( b );
    _infoBox->setShown(b);
    updateInfoBox();
}

void Viewer::toggleShowDescription( bool b )
{
    Options::instance()->setShowDescription( b );
    updateInfoBox();
}

void Viewer::toggleShowDate( bool b )
{
    Options::instance()->setShowDate( b );
    updateInfoBox();
}

void Viewer::toggleShowTime( bool b )
{
    Options::instance()->setShowTime( b );
    updateInfoBox();
}

void Viewer::toggleShowOption( const QString& category, bool b )
{
    Options::instance()->setShowOption( category, b );
    updateInfoBox();
}

void Viewer::showFirst()
{
    _forward = true;
    save();
    _current = 0;
    load();
}

void Viewer::showLast()
{
    _forward = false;
    save();
     _current = _list.count() -1;
     load();
}

void Viewer::save()
{
    currentInfo()->setDrawList( _display->drawHandler()->drawList() );
}

void Viewer::startDraw()
{
    _display->startDrawing();
    _display->drawHandler()->slotSelect();
    _toolbar->show();
}

void Viewer::stopDraw()
{
    _display->stopDrawing();
    _toolbar->hide();
}

void Viewer::slotSetWallpaperC()
{
    setAsWallpaper(1);
}

void Viewer::slotSetWallpaperT()
{
    setAsWallpaper(2);
}

void Viewer::slotSetWallpaperCT()
{
    setAsWallpaper(3);
}

void Viewer::slotSetWallpaperCM()
{
    setAsWallpaper(4);
}

void Viewer::slotSetWallpaperTM()
{
    setAsWallpaper(5);
}

void Viewer::slotSetWallpaperS()
{
    setAsWallpaper(6);
}

void Viewer::slotSetWallpaperCAF()
{
    setAsWallpaper(7);
}

void Viewer::setAsWallpaper(int mode)
{
    if(mode>7 || mode<1) return;
    DCOPRef kdesktop("kdesktop","KBackgroundIface");
    kdesktop.send("setWallpaper(QString,int)",currentInfo()->fileName(0),mode);
}

bool Viewer::close( bool alsoDelete)
{
    save();
    _slideShowTimer->stop();
    return QWidget::close( alsoDelete );
}

ImageInfo* Viewer::currentInfo()
{
    return _list.at( _current );
}

void Viewer::infoBoxMove()
{
    QPoint p = mapFromGlobal( QCursor::pos() );
    Options::Position oldPos = Options::instance()->infoBoxPosition();
    Options::Position pos = oldPos;
    int x = _display->mapFromParent( p ).x();
    int y = _display->mapFromParent( p ).y();
    int w = _display->width();
    int h = _display->height();

    if ( x < w/3 )  {
        if ( y < h/3  )
            pos = Options::TopLeft;
        else if ( y > h*2/3 )
            pos = Options::BottomLeft;
        else
            pos = Options::Left;
    }
    else if ( x > w*2/3 )  {
        if ( y < h/3  )
            pos = Options::TopRight;
        else if ( y > h*2/3 )
            pos = Options::BottomRight;
        else
            pos = Options::Right;
    }
    else {
        if ( y < h/3  )
            pos = Options::Top;
            else if ( y > h*2/3 )
                pos = Options::Bottom;
    }
    if ( pos != oldPos )  {
        Options::instance()->setInfoBoxPosition( pos );
        updateInfoBox();
    }
}

void Viewer::moveInfoBox()
{
    _infoBox->setSize();
    Options::Position pos = Options::instance()->infoBoxPosition();

    int lx = _display->pos().x();
    int ly = _display->pos().y();
    int lw = _display->width();
    int lh = _display->height();

    int bw = _infoBox->width();
    int bh = _infoBox->height();

    int bx, by;
    // x-coordinate
    if ( pos == Options::TopRight || pos == Options::BottomRight || pos == Options::Right )
        bx = lx+lw-5-bw;
    else if ( pos == Options::TopLeft || pos == Options::BottomLeft || pos == Options::Left )
        bx = lx+5;
    else
        bx = lx+lw/2-bw/2;


    // Y-coordinate
    if ( pos == Options::TopLeft || pos == Options::TopRight || pos == Options::Top )
        by = ly+5;
    else if ( pos == Options::BottomLeft || pos == Options::BottomRight || pos == Options::Bottom )
        by = ly+lh-5-bh;
    else
        by = ly+lh/2-bh/2;

    _infoBox->move(bx,by);
}

void Viewer::resizeEvent( QResizeEvent* e )
{
    moveInfoBox();
    QWidget::resizeEvent( e );
}

void Viewer::updateInfoBox()
{
    if ( currentInfo() ) {
        QMap<int, QPair<QString,QString> > map;
        QString origText = Util::createInfoText( currentInfo(), &map );
        QString text = QString::fromLatin1("<qt>") + origText + QString::fromLatin1("</qt>");
        if ( Options::instance()->showInfoBox() && !origText.isNull() ) {
            _infoBox->setInfo( text, map );
            _infoBox->show();
        }
        else
            _infoBox->hide();

        moveInfoBox();
    }
}

Viewer::~Viewer()
{
    if ( _latest == this )
        _latest = 0;
}

void Viewer::createToolBar()
{
    KIconLoader loader;
    KActionCollection* actions = new KActionCollection( this, "actions" );
    _toolbar = new KToolBar( this );
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
}

void Viewer::toggleFullScreen()
{
    setShowFullScreen( !_showingFullScreen );
}

void Viewer::slotStartStopSlideShow()
{
    if (_slideShowTimer->isActive() ) {
        _slideShowTimer->stop();
        _speedDisplay->end();
    }
    else {
        _slideShowTimer->start( _slideShowPause, true );
        _speedDisplay->start();
    }
}

void Viewer::slotSlideShowNext()
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

    // ensure that there is a few milliseconds pause, so that an end slideshow keypress
    // can get through immediately, we don't want it to queue up behind a bunch of timer events,
    // which loaded a number of new images before the slideshow stops
    int ms = QMAX( 200, _slideShowPause - timer.elapsed() );
    _slideShowTimer->start( ms, true );
}

void Viewer::slotSlideShowFaster()
{
    _slideShowPause -= 500;
    if ( _slideShowPause < 500 )
        _slideShowPause = 500;
    _speedDisplay->display( _slideShowPause );
    if (_slideShowTimer->isActive() )
        _slideShowTimer->changeInterval( _slideShowPause );
}

void Viewer::slotSlideShowSlower()
{
    _slideShowPause += 500;
    _speedDisplay->display( _slideShowPause );
    if (_slideShowTimer->isActive() )
        _slideShowTimer->changeInterval( _slideShowPause );
}

void Viewer::editImage()
{
    ImageInfoList list;
    list.append( currentInfo() );
    MainView::configureImages( list, true );
}

bool Viewer::showingFullScreen() const
{
    return _showingFullScreen;
}

void Viewer::setShowFullScreen( bool on )
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
            resize( Options::instance()->viewerSize() );
            _sized = true;
        }
    }
    _showingFullScreen = on;
}

void Viewer::makeCategoryImage()
{
    CategoryImageConfig::instance()->setCurrentImage( _display->currentViewAsThumbnail(), currentInfo() );
    CategoryImageConfig::instance()->show();
}

void Viewer::updateCategoryConfig()
{
    CategoryImageConfig::instance()->setCurrentImage( _display->currentViewAsThumbnail(), currentInfo() );
}


void Viewer::populateExternalPopup()
{
    _externalPopup->populate( currentInfo(), _list );
}

void Viewer::show( bool slideShow )
{
    QSize size;
    bool fullScreen;
    if ( slideShow ) {
        fullScreen = Options::instance()->launchSlideShowFullScreen();
        size = Options::instance()->slideShowSize();
    }
    else {
        fullScreen = Options::instance()->launchViewerFullScreen();
        size = Options::instance()->viewerSize();
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

KActionCollection* Viewer::actions()
{
    return _actions;
}

void Viewer::keyPressEvent( QKeyEvent* event )
{
    if ( event->stateAfter() == 0 && event->state() == 0 && ( event->key() >= Key_A && event->key() <= Key_Z ) ) {
        QString token = event->text().upper().left(1);
        if ( currentInfo()->hasOption( QString::fromLatin1("Tokens"), token ) )
            currentInfo()->removeOption( QString::fromLatin1("Tokens"), token );
        else
            currentInfo()->addOption( QString::fromLatin1("Tokens"), token );
        updateInfoBox();
    }
    QWidget::keyPressEvent( event );
}

#include "viewer.moc"
