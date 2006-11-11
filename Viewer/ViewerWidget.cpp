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

#include <config.h>

#include <kdeversion.h>
#include "Viewer/ViewerWidget.h"
#include <qlayout.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kaction.h>
#include <klocale.h>
#include "Utilities/Util.h"
#include "ShowOptionAction.h"
#include <qtimer.h>
#include "DrawHandler.h"
#include <kwin.h>
#include "SpeedDisplay.h"
#include "MainWindow/Window.h"
#include "CategoryImageConfig.h"
#include <dcopref.h>
#include "MainWindow/ExternalPopup.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "InfoBox.h"
#include <qwidgetstack.h>
#include "VideoDisplay.h"
#include "MainWindow/DirtyIndicator.h"
#include "ViewerWidget.h"
#include <qapplication.h>
#include <qeventloop.h>

#ifdef HASEXIV2
#  include "Exif/InfoDialog.h"
#endif

/**
 * \namespace Viewer
 * \brief Viewer used for displaying images and videos
 *
 * This class implements the viewer used to display images and videos.
 *
 * The class consists of these components:
 * <ul>
 * <li>\ref ViewerWidget - This is the topmost widget used as the viewer.
 * <li>\ref Display, \ref ImageDisplay, and \ref VideoDisplay - Widgets hierarchy which takes care of the actual displaying of content.
 * <li> \ref DisplayAreaHandler, \ref DrawHandler, and \ref ViewHandler - Class Hierarchy which interprets mouse gestures.
 * <li> \ref Draw, \ref LineDraw, \ref CircleDraw, and \ref RectDraw - Class Hierarchy containing information about drawing on images.
 * <li> \ref InfoBox - Widget implementing the informatiom box
 * <li> \ref SpeedDisplay - Widget implementing the toplevel display used when adjusting slideshow speed.
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
    setPaletteBackgroundColor( black );
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
    connect( _slideShowTimer, SIGNAL( timeout() ), this, SLOT( slotSlideShowNextFromTimer() ) );
    _speedDisplay = new SpeedDisplay( this );

    setFocusPolicy( StrongFocus );
}


void Viewer::ViewerWidget::setupContextMenu()
{
    _popup = new QPopupMenu( this, "context popup menu" );
    _actions = new KActionCollection( this, "viewer", KGlobal::instance() );

    createSlideShowMenu();
    createZoomMenu();
    createRotateMenu();
    createSkipMenu();
    createShowContextMenu();
    createWallPaperMenu();
    createInvokeExternalMenu();

    _drawOnImages = new KAction( i18n("Draw on Image"),  0, this, SLOT( startDraw() ), this, "viewer-draw-on-image" );
    _drawOnImages->plug( _popup );

    KAction* action = new KAction( i18n("Edit Image Properties..."),  CTRL+Key_1, this, SLOT( editImage() ),
                          _actions, "viewer-edit-image-properties" );
    action->plug( _popup );

    // PENDING(blackie) This should only be enabled for image displays.
    _categoryEditor = new KAction( i18n("Show Category Editor"), 0, this, SLOT( makeCategoryImage() ),
                                   _actions, "viewer-show-category-editor" );
    _categoryEditor->plug( _popup );

#ifdef HASEXIV2
    _showExifViewer = new KAction( i18n("Show EXIF Viewer"), 0, this, SLOT( showExifViewer() ),
                          _actions, "viewer-show-exif-viewer" );
    _showExifViewer->plug( _popup );
#endif

    action = new KAction( i18n("Close"), Key_Escape, this, SLOT( close() ), _actions, "viewer-close" );
    action->plug( _popup );
    _actions->readShortcutSettings();

    createVideoActions();
}

void Viewer::ViewerWidget::createShowContextMenu()
{
    QPopupMenu *showPopup = new QPopupMenu( _popup );

    KToggleAction* taction = new KToggleAction( i18n("Show Info Box"), CTRL+Key_I, _actions, "viewer-show-infobox" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowInfoBox( bool ) ) );
    taction->plug( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showInfoBox() );

    // PENDING(blackie) Only for image display
    taction = new KToggleAction( i18n("Show Drawing"), CTRL+Key_D, _actions, "viewer-show-drawing");
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDrawings( bool ) ) );
    taction->plug( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showDrawings() );

    taction = new KToggleAction( i18n("Show Description"), 0, _actions, "viewer-show-description" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDescription( bool ) ) );
    taction->plug( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showDescription() );

    taction = new KToggleAction( i18n("Show Date"), 0, _actions, "viewer-show-date" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDate( bool ) ) );
    taction->plug( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showDate() );

    taction = new KToggleAction( i18n("Show Time"), 0, _actions, "viewer-show-time" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowTime( bool ) ) );
    taction->plug( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showTime() );

    taction = new KToggleAction( i18n("Show Filename"), 0, _actions, "viewer-show-filename" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowFilename( bool ) ) );
    taction->plug( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showFilename() );

    taction = new KToggleAction( i18n("Show EXIF"), 0, _actions, "viewer-show-exif" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowEXIF( bool ) ) );
    taction->plug( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showEXIF() );

    taction = new KToggleAction( i18n("Show Image Size"), 0, _actions, "viewer-show-imagesize" );
    connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowImageSize( bool ) ) );
    taction->plug( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showImageSize() );


    _popup->insertItem( QIconSet(), i18n("Show"), showPopup );

    QValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        ShowOptionAction* action = new ShowOptionAction( (*it)->name(), _actions );
        action->plug( showPopup );
        connect( action, SIGNAL( toggled( const QString&, bool ) ),
                 this, SLOT( toggleShowOption( const QString&, bool ) ) );
    }
}

void Viewer::ViewerWidget::createWallPaperMenu()
{
    _wallpaperMenu = new QPopupMenu( _popup, "context popup menu" );

    KAction* action = new KAction( i18n("Centered"), 0, this, SLOT( slotSetWallpaperC() ), _actions, "viewer-centered" );
    action->plug( _wallpaperMenu );

    action = new KAction( i18n("Tiled"), 0, this, SLOT( slotSetWallpaperT() ), _actions, "viewer-tiled" );
    action->plug( _wallpaperMenu );

    action = new KAction( i18n("Center Tiled"), 0, this, SLOT( slotSetWallpaperCT() ), _actions, "viewer-center-tiled" );
    action->plug( _wallpaperMenu );

    action = new KAction( i18n("Centered Maxpect"), 0, this, SLOT( slotSetWallpaperCM() ),
                          _actions, "viewer-centered-maxspect" );
    action->plug( _wallpaperMenu );

    action = new KAction( i18n("Tiled Maxpect"), 0, this, SLOT( slotSetWallpaperTM() ),
                          _actions, "viewer-tiled-maxpect" );
    action->plug( _wallpaperMenu );

    action = new KAction( i18n("Scaled"), 0, this, SLOT( slotSetWallpaperS() ), _actions, "viewer-scaled" );
    action->plug( _wallpaperMenu );

    action = new KAction( i18n("Centered Auto Fit"), 0, this, SLOT( slotSetWallpaperCAF() ),
                          _actions, "viewer-centered-auto-fit" );
    action->plug( _wallpaperMenu );

    _popup->insertItem( QIconSet(), i18n("Set as Wallpaper"), _wallpaperMenu );
}

void Viewer::ViewerWidget::createInvokeExternalMenu()
{
    _externalPopup = new MainWindow::ExternalPopup( _popup );
    _popup->insertItem( QIconSet(), i18n("Invoke External Program"), _externalPopup );
    connect( _externalPopup, SIGNAL( aboutToShow() ), this, SLOT( populateExternalPopup() ) );
}

void Viewer::ViewerWidget::createRotateMenu()
{
    _rotateMenu = new QPopupMenu( _popup );

    KAction* action = new KAction( i18n("Rotate 90 Degrees"), Key_9, this, SLOT( rotate90() ), _actions, "viewer-rotate90" );
    action->plug( _rotateMenu );

    action = new KAction( i18n("Rotate 180 Degrees"), Key_8, this, SLOT( rotate180() ), _actions, "viewer-rotate180" );
    action->plug( _rotateMenu );

    action = new KAction( i18n("Rotate 270 Degrees"), Key_7, this, SLOT( rotate270() ), _actions, "viewer-rotare270" );
    action->plug( _rotateMenu );

    _popup->insertItem( QIconSet(), i18n("Rotate"), _rotateMenu );
}

void Viewer::ViewerWidget::createSkipMenu()
{
    QPopupMenu *popup = new QPopupMenu( _popup );

    KAction* action = new KAction( i18n("First"), Key_Home, this, SLOT( showFirst() ), _actions, "viewer-home" );
    action->plug( popup );
    _backwardActions.append(action);

    action = new KAction( i18n("Last"), Key_End, this, SLOT( showLast() ), _actions, "viewer-end" );
    action->plug( popup );
    _forwardActions.append(action);

    action = new KAction( i18n("Show Next"), Key_PageDown, this, SLOT( showNext() ), _actions, "viewer-next" );
    action->plug( popup );
    _forwardActions.append(action);

    action = new KAction( i18n("Skip 10 Forward"), CTRL+Key_PageDown, this, SLOT( showNext10() ), _actions, "viewer-next-10" );
    action->plug( popup );
    _forwardActions.append(action);

    action = new KAction( i18n("Skip 100 Forward"), SHIFT+Key_PageDown, this, SLOT( showNext100() ), _actions, "viewer-next-100" );
    action->plug( popup );
    _forwardActions.append(action);

    action = new KAction( i18n("Skip 1000 Forward"), CTRL+SHIFT+Key_PageDown, this, SLOT( showNext1000() ), _actions, "viewer-next-1000" );
    action->plug( popup );
    _forwardActions.append(action);

    action = new KAction( i18n("Show Previous"), Key_PageUp, this, SLOT( showPrev() ), _actions, "viewer-prev" );
    action->plug( popup );
    _backwardActions.append(action);

    action = new KAction( i18n("Skip 10 Backward"), CTRL+Key_PageUp, this, SLOT( showPrev10() ), _actions, "viewer-prev-10" );
    action->plug( popup );
    _backwardActions.append(action);

    action = new KAction( i18n("Skip 100 Backward"), SHIFT+Key_PageUp, this, SLOT( showPrev100() ), _actions, "viewer-prev-100" );
    action->plug( popup );
    _backwardActions.append(action);

    action = new KAction( i18n("Skip 1000 Backward"), CTRL+SHIFT+Key_PageUp, this, SLOT( showPrev1000() ), _actions, "viewer-prev-1000" );
    action->plug( popup );
    _backwardActions.append(action);

    _popup->insertItem( QIconSet(), i18n("Skip Images"), popup );
}

void Viewer::ViewerWidget::createZoomMenu()
{
    QPopupMenu *popup = new QPopupMenu( _popup );

    // PENDING(blackie) Only for image display?
    KAction* action = new KAction( i18n("Zoom In"), Key_Plus, this, SLOT( zoomIn() ), _actions, "viewer-zoom-in" );
    action->plug( popup );

    action = new KAction( i18n("Zoom Out"), Key_Minus, this, SLOT( zoomOut() ), _actions, "viewer-zoom-out" );
    action->plug( popup );

    action = new KAction( i18n("Full View"), Key_Period, this, SLOT( zoomFull() ), _actions, "viewer-zoom-full" );
    action->plug( popup );

    action = new KAction( i18n("Pixel for Pixel View"), Key_Equal, this, SLOT( zoomPixelForPixel() ), _actions, "viewer-zoom-pixel" );
    action->plug( popup );

    action = new KAction( i18n("Toggle Full Screen"), Key_Return, this, SLOT( toggleFullScreen() ),
                          _actions, "viewer-toggle-fullscreen" );
    action->plug( popup );

    _popup->insertItem( QIconSet(), i18n("Zoom"), popup );
}


void Viewer::ViewerWidget::createSlideShowMenu()
{
    QPopupMenu *popup = new QPopupMenu( _popup );

    _startStopSlideShow = new KAction( i18n("Run Slideshow"), CTRL+Key_R, this, SLOT( slotStartStopSlideShow() ),
                                       _actions, "viewer-start-stop-slideshow" );
    _startStopSlideShow->plug( popup );

    _slideShowRunFaster = new KAction( i18n("Run Faster"), CTRL + Key_Plus, this, SLOT( slotSlideShowFaster() ),
                                       _actions, "viewer-run-faster" );
    _slideShowRunFaster->plug( popup );

    _slideShowRunSlower = new KAction( i18n("Run Slower"), CTRL+Key_Minus, this, SLOT( slotSlideShowSlower() ),
                                       _actions, "viewer-run-slower" );
    _slideShowRunSlower->plug( popup );

    _popup->insertItem( QIconSet(), i18n("Slideshow"), popup );
}


void Viewer::ViewerWidget::load( const QStringList& list, int index )
{
    _list = list;

    QStringList images;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        if ( !Utilities::isVideo( *it ) )
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
    bool isVideo = Utilities::isVideo( currentInfo()->fileName() );
    if ( isVideo )
        _display = _videoDisplay;
    else
        _display = _imageDisplay;

    _stack->raiseWidget( _display );

    _drawOnImages->setEnabled( !isVideo );
    _rotateMenu->setEnabled( !isVideo );
    _wallpaperMenu->setEnabled( !isVideo );
    _categoryEditor->setEnabled( !isVideo );
#ifdef HASEXIV2
    _showExifViewer->setEnabled( !isVideo );
#endif

    _popup->setItemVisible( _videoSeperatorId, isVideo );
    _play->unplug( _popup );
    _stop->unplug( _popup );
    _pause->unplug( _popup );
    _restart->unplug( _popup );
    if ( isVideo ) {
        _play->plug( _popup );
        _stop->plug( _popup );
        _pause->plug( _popup );
        _restart->plug( _popup );
    }

    if ( _display->offersDrawOnImage() )
        _display->drawHandler()->setDrawList( currentInfo()->drawList() );
    bool ok = _display->setImage( currentInfo(), _forward );
    if ( !ok ) {
        close( false );
        return;
    }

    setCaption( QString::fromLatin1( "KPhotoAlbum - %1" ).arg( currentInfo()->fileName() ) );
    updateInfoBox();

    // PENDING(blackie) This needs to be improved, so that it shows the actions only if there are that many images to jump.
    for( QPtrList<KAction>::const_iterator it = _forwardActions.begin(); it != _forwardActions.end(); ++it )
      (*it)->setEnabled( _current +1 < (int) _list.count() );
    for( QPtrList<KAction>::const_iterator it = _backwardActions.begin(); it != _forwardActions.end(); ++it )
      (*it)->setEnabled( _current > 0 );
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

void Viewer::ViewerWidget::showNextN(int n)
{
    save();
    if ( _current +1 < (int) _list.count() )  {
        _current += n;
	if (_current >= (int) _list.count())
	  _current = (int) _list.count() - 1;
        _forward = true;
        load();
    }
}

void Viewer::ViewerWidget::showNext()
{
    showNextN(1);
}

void Viewer::ViewerWidget::showNext10()
{
    showNextN(10);
}

void Viewer::ViewerWidget::showNext100()
{
    showNextN(100);
}

void Viewer::ViewerWidget::showNext1000()
{
    showNextN(1000);
}

void Viewer::ViewerWidget::showPrevN(int n)
{
    save();
    if ( _current > 0  )  {
        _current -= n;
	if (_current < 0)
	  _current = 0;
        _forward = false;
        load();
    }
}

void Viewer::ViewerWidget::showPrev()
{
    showPrevN(1);
}

void Viewer::ViewerWidget::showPrev10()
{
    showPrevN(10);
}

void Viewer::ViewerWidget::showPrev100()
{
    showPrevN(100);
}

void Viewer::ViewerWidget::showPrev1000()
{
    showPrevN(1000);
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

void Viewer::ViewerWidget::toggleShowFilename( bool b )
{
    Settings::SettingsData::instance()->setShowFilename( b );
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

void Viewer::ViewerWidget::toggleShowImageSize( bool b )
{
    Settings::SettingsData::instance()->setShowImageSize( b );
    updateInfoBox();
}


void Viewer::ViewerWidget::toggleShowOption( const QString& category, bool b )
{
    DB::ImageDB::instance()->categoryCollection()->categoryForName(category)->setDoShow( b );
    updateInfoBox();
}

void Viewer::ViewerWidget::showFirst()
{
    showPrevN(_list.count());
}

void Viewer::ViewerWidget::showLast()
{
    showNextN(_list.count());
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
    DrawHandler* handler = _imageDisplay->drawHandler();
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

void Viewer::ViewerWidget::toggleFullScreen()
{
    setShowFullScreen( !_showingFullScreen );
}

void Viewer::ViewerWidget::slotStartStopSlideShow()
{
    bool wasRunningSlideShow = _isRunningSlideShow;
    _isRunningSlideShow = !_isRunningSlideShow && _list.count() != 1;

    if ( wasRunningSlideShow ) {
        _slideShowTimer->stop();
        if ( _list.count() != 1 )
            _speedDisplay->end();
    }
    else {
        if ( currentInfo()->mediaType() != DB::Video )
            _slideShowTimer->start( _slideShowPause, true );
        _speedDisplay->start();
    }
}

void Viewer::ViewerWidget::slotSlideShowNextFromTimer()
{
    // Load the next images.
    QTime timer;
    timer.start();
    if ( _display == _imageDisplay )
        slotSlideShowNext();

    // ensure that there is a few milliseconds pause, so that an end slideshow keypress
    // can get through immediately, we don't want it to queue up behind a bunch of timer events,
    // which loaded a number of new images before the slideshow stops
    int ms = QMAX( 200, _slideShowPause - timer.elapsed() );
    _slideShowTimer->start( ms, true );
}

void Viewer::ViewerWidget::slotSlideShowNext()
{
    _forward = true;
    save();
    if ( _current +1 < (int) _list.count() )
        _current++;
    else
        _current = 0;

    load();
}

void Viewer::ViewerWidget::slotSlideShowFaster()
{
    changeSlideShowInterval(-500);
}

void Viewer::ViewerWidget::slotSlideShowSlower()
{
    changeSlideShowInterval(+500);
}

void Viewer::ViewerWidget::changeSlideShowInterval( int delta )
{
    if ( _list.count() == 1 )
        return;

    _slideShowPause += delta;
    _slideShowPause = QMAX( _slideShowPause, 500 );
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
    // To avoid that the image is first loaded in a small size and the reloaded when scaled up, we need to resize the window right away.
    resize( qApp->desktop()->screenGeometry().size() );
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
    if ( slideShow != _isRunningSlideShow) {
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
        if ( currentInfo()->hasCategoryInfo( QString::fromLatin1("Tokens"), token ) )
            currentInfo()->removeCategoryInfo( QString::fromLatin1("Tokens"), token );
        else
            currentInfo()->addCategoryInfo( QString::fromLatin1("Tokens"), token );
        DB::ImageDB::instance()->categoryCollection()->categoryForName( QString::fromLatin1("Tokens") )->addItem( token );
        updateInfoBox();
        MainWindow::DirtyIndicator::markDirty();
    }
    QWidget::keyPressEvent( event );
}

void Viewer::ViewerWidget::videoStopped()
{
    if ( _isRunningSlideShow )
        slotSlideShowNext();
}


void Viewer::ViewerWidget::wheelEvent( QWheelEvent* event )
{
   if ( event->delta() < 0) {
     showNext();
   } else {
     showPrev();
   }
}

void Viewer::ViewerWidget::showExifViewer()
{
#ifdef HASEXIV2
    Exif::InfoDialog* exifDialog = new Exif::InfoDialog( currentInfo()->fileName(), this );
    exifDialog->show();
#endif

}

void Viewer::ViewerWidget::zoomIn()
{
    _display->zoomIn();
}

void Viewer::ViewerWidget::zoomOut()
{
    _display->zoomOut();
}

void Viewer::ViewerWidget::zoomFull()
{
    _display->zoomFull();
}

void Viewer::ViewerWidget::zoomPixelForPixel()
{
    _display->zoomPixelForPixel();
}

void Viewer::ViewerWidget::toggleShowDrawings( bool b )
{
    if ( _display == _imageDisplay )
        _imageDisplay->toggleShowDrawings( b );
}

void Viewer::ViewerWidget::createVideoActions()
{
    _videoSeperatorId = _popup->insertSeparator();

    _play = new KAction( i18n("Play"), 0, this, SLOT( play() ), _actions, "viewer-video-play" );
    _stop = new KAction( i18n("Stop"), 0, this, SLOT( stop() ), _actions, "viewer-video-stop" );
    _pause = new KAction( i18n("Pause"), 0, this, SLOT( pause() ), _actions, "viewer-video-pause" );
    _restart = new KAction( i18n("Restart"), 0, this, SLOT( restart() ), _actions, "viewer-video-restart" );
}

void Viewer::ViewerWidget::play()
{
    _videoDisplay->play();
}

void Viewer::ViewerWidget::stop()
{
    _videoDisplay->stop();
}

void Viewer::ViewerWidget::pause()
{
    _videoDisplay->pause();
}

void Viewer::ViewerWidget::restart()
{
    _videoDisplay->restart();
}


#include "ViewerWidget.moc"
