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

#ifdef TEMPORARILY_REMOVED
#include <config.h>
#endif

#include <kdeversion.h>
#include "Viewer/ViewerWidget.h"
#include <qlayout.h>
#include <qcursor.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <Q3ValueList>
#include <Q3PtrList>
#include <QResizeEvent>
#include <Q3VBoxLayout>
#include <QWheelEvent>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kaction.h>
#include <klocale.h>
#include "Utilities/Util.h"
#include "ShowOptionAction.h"
#include <qtimer.h>
#include "DrawHandler.h"
#include <kwindowsystem.h>
#include "SpeedDisplay.h"
#include "MainWindow/Window.h"
#include "CategoryImageConfig.h"
#ifdef TEMPORARILY_REMOVED
#include <dcopref.h>
#endif
#include "MainWindow/ExternalPopup.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "InfoBox.h"
#include <q3widgetstack.h>
#include "VideoDisplay.h"
#include "MainWindow/DirtyIndicator.h"
#include "ViewerWidget.h"
#include <qapplication.h>
#include <qeventloop.h>
#include <qfileinfo.h>
#include "TextDisplay.h"
#include <kdebug.h>
#include <KActionCollection>

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
Viewer::ViewerWidget::ViewerWidget()
    :QWidget( 0, Qt::WType_TopLevel ), _current(0), _popup(0), _showingFullScreen( false ), _forward( true ), _isRunningSlideShow( false )
{
#ifdef TEMPORARILY_REMOVED
    setWFlags( Qt::WDestructiveClose );
    setPaletteBackgroundColor( Qt::black );
#else
    kDebug() << "TEMPORARILY REMOVED " << k_funcinfo << endl;
#endif
    _latest = this;

    Q3VBoxLayout* layout = new Q3VBoxLayout( this );

    _stack = new Q3WidgetStack( this, "stack" );

    _display = _imageDisplay = new ImageDisplay( _stack ); // Must be created before the toolbar.
    _textDisplay = new TextDisplay( _stack );
    _stack->addWidget( _textDisplay );
    _videoDisplay = new VideoDisplay( _stack );
    connect( _videoDisplay, SIGNAL( stopped() ), this, SLOT( videoStopped() ) );

    connect( _imageDisplay, SIGNAL( possibleChange() ), this, SLOT( updateCategoryConfig() ) );
    connect( _imageDisplay, SIGNAL( imageReady() ), this, SLOT( updateInfoBox() ) );
    connect( _imageDisplay, SIGNAL( setCaptionInfo(const QString&) ),
             this, SLOT( setCaptionWithDetail(const QString&) ) );
    createToolBar();
#ifdef TEMPORARILY_REMOVED
    _toolbar->hide();
    layout->addWidget( _toolbar );
#else
    kDebug() << "TEMPORARILY REMOVED " << k_funcinfo << endl;
#endif


    layout->addWidget( _stack );

    // This must not be added to the layout, as it is standing on top of
    // the ImageDisplay
    _infoBox = new InfoBox( this );
    _infoBox->hide();

    setupContextMenu();

    _slideShowTimer = new QTimer( this );
    _slideShowPause = Settings::SettingsData::instance()->slideShowInterval() * 1000;
    connect( _slideShowTimer, SIGNAL( timeout() ), this, SLOT( slotSlideShowNextFromTimer() ) );
    _speedDisplay = new SpeedDisplay( this );

    setFocusPolicy( Qt::StrongFocus );
}


void Viewer::ViewerWidget::setupContextMenu()
{
    _popup = new QMenu( this );
#ifdef OLD_CODE
    _actions = new KActionCollection( this, "viewer", KGlobal::instance() );
#endif
    _actions = new KActionCollection( this );

    createSlideShowMenu();
    createZoomMenu();
    createRotateMenu();
    createSkipMenu();
    createShowContextMenu();
    createWallPaperMenu();
    createInvokeExternalMenu();

    _drawOnImages = _actions->addAction( "viewer-draw-on-image", this, SLOT( startDraw() ) );
    _drawOnImages->setText( i18n("Draw on Image") );
    _popup->addAction(_drawOnImages);

    QAction* action = _actions->addAction( "viewer-edit-image-properties", this, SLOT( editImage() ) );
    action->setText( i18n("Annotate...") );
    action->setShortcut( Qt::CTRL+Qt::Key_1 );
    _popup->addAction( action );

    // PENDING(blackie) This should only be enabled for image displays.
    _categoryEditor = _actions->addAction( "viewer-show-category-editor", this, SLOT( makeCategoryImage() ) );
    _categoryEditor->setText( i18n("Show Category Editor") );
    _popup->addAction(_categoryEditor);

#ifdef HASEXIV2
    _showExifViewer = _actions->addAction( "viewer-show-exif-viewer", this, SLOT( showExifViewer() ) );
    _showExifViewer->setText( i18n("Show EXIF Viewer") );
    _popup->addAction( _showExifViewer );
#endif

    action = _actions->addAction( "viewer-close", this, SLOT( close() ) );
    action->setText( i18n("Close") );
    action->setShortcut( Qt::Key_Escape );

    _popup->addAction( action );
#ifdef TEMPORARILY_REMOVED
    _actions->readShortcutSettings();
#else
    kDebug() << "TEMPORARILY REMOVED " << k_funcinfo << endl;
#endif

    createVideoActions();
}

void Viewer::ViewerWidget::createShowContextMenu()
{
    QMenu *showPopup = new QMenu( _popup );

    KToggleAction* taction = 0;

    taction = _actions->add<KToggleAction>( "viewer-show-infobox", this, SLOT( toggleShowInfoBox( bool ) ) );
    taction->setText( i18n("Show Info Box") );
    taction->setShortcut( Qt::CTRL+Qt::Key_I );
    showPopup->addAction( taction );
    taction->setChecked( Settings::SettingsData::instance()->showInfoBox() );
#ifdef TEMPORARILY_REMOVED

    // PENDING(blackie) Only for image display
    taction = _actions->add<KToggleAction>( "viewer-show-drawing", this, SLOT( toggleShowDrawings( bool ) ) );
    taction->setText( i18n("Show Drawing") );
    taction->setShortcut( Qt::CTRL+Qt::Key_D );
    taction->addAction( taction );
    taction->setChecked( Settings::SettingsData::instance()->showDrawings() );

    taction = _actions->add<KToggleAction>( "viewer-show-description", this, SLOT( toggleShowDescription( bool ) ) );
    taction->setText( i18n("Show Description") );
    taction->setShortcut( 0 );
    taction->addAction( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showDescription() );

    taction = _actions<->addKToggleAction>("viewer-show-date", this, SLOT( toggleShowDate( bool ) ) );
    taction->setText( i18n("Show Date") );
    taction->addAction( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showDate() );

    taction = _actions<->addKToggleAction>("viewer-show-time", this, SLOT( toggleShowTime( bool ) ) );
    taction->setText( i18n("Show Time") );
    taction->addAction( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showTime() );

    taction = _actions<->addKToggleAction>("viewer-show-filename", this, SLOT( toggleShowFilename( bool ) ) );
    taction->setText( i18n("Show Filename") );
    taction->addAction( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showFilename() );

    taction = _actions<->addKToggleAction>("viewer-show-exif", this, SLOT( toggleShowEXIF( bool ) ) );
    taction->setText( i18n("Show EXIF") );
    taction->addAction( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showEXIF() );

    taction = _actions<->addKToggleAction>("viewer-show-imagesize", this, SLOT( toggleShowImageSize( bool ) ) );
    taction->setText( i18n("Show Image Size") );
    taction->addAction( showPopup );
    taction->setChecked( Settings::SettingsData::instance()->showImageSize() );

    _popup->insertItem( QIcon(), i18n("Show"), showPopup );

    Q3ValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( Q3ValueList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        ShowOptionAction* action = new ShowOptionAction( (*it)->name(), _actions );
        action->plug( showPopup );
        connect( action, SIGNAL( toggled( const QString&, bool ) ),
                 this, SLOT( toggleShowOption( const QString&, bool ) ) );
    }
#else
    kDebug() << "TEMPORARILY REMOVED " << k_funcinfo << endl;
#endif
}

void Viewer::ViewerWidget::createWallPaperMenu()
{
#ifdef TEMPORARILY_REMOVED
    _wallpaperMenu = new QMenu( _popup, "context popup menu" );

    KAction* action = _actions->addAction( "viewer-centered", this, SLOT( slotSetWallpaperC() ) );
    KAction->setText( i18n("Centered") );
    _wallpaperMenu->addAction(action);

    action = _actions->addAction( "viewer-tiled", this, SLOT( slotSetWallpaperT() ) );
    action->setText( i18n("Tiled") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( "viewer-center-tiled", this, SLOT( slotSetWallpaperCT() ) );
    action->setText( i18n("Center Tiled") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( "viewer-centered-maxspect", this, SLOT( slotSetWallpaperCM() ) );
    action->setText( i18n("Centered Maxpect") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( "viewer-tiled-maxpect", this, SLOT( slotSetWallpaperTM() ) );
    action->setText( i18n("Tiled Maxpect") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( "viewer-scaled", this, SLOT( slotSetWallpaperS() ) );
    action->setText( i18n("Scaled") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( "viewer-centered-auto-fit", this, SLOT( slotSetWallpaperCAF() ) );
    action->setText( i18n("Centered Auto Fit") );
    _wallpaperMenu->addAction( action );

    _popup->insertItem( QIcon(), i18n("Set as Wallpaper"), _wallpaperMenu );
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

void Viewer::ViewerWidget::createInvokeExternalMenu()
{
    _externalPopup = new MainWindow::ExternalPopup( _popup );
    _popup->insertItem( QIcon(), i18n("Invoke External Program"), _externalPopup );
    connect( _externalPopup, SIGNAL( aboutToShow() ), this, SLOT( populateExternalPopup() ) );
}

void Viewer::ViewerWidget::createRotateMenu()
{
#ifdef TEMPORARILY_REMOVED
    _rotateMenu = new QMenu( _popup );

    KAction* action = _actions->addAction( "viewer-rotate90", this, SLOT( rotate90() ) );
    KAction->setText( i18n("Rotate 90 Degrees") );
    KAction->setIcon( Qt::Key_9 );
    _rotateMenu->addAction( action );

    action = _actions->addAction( "viewer-rotate180", this, SLOT( rotate180() ) );
    action->setText( i18n("Rotate 180 Degrees") );
    action->setIcon( Qt::Key_8 );
    _rotateMenu->addAction( action );

    action = _actions->addAction( "viewer-rotare270", this, SLOT( rotate270() ) );
    action->setText( i18n("Rotate 270 Degrees") );
    action->setIcon( Qt::Key_7 );
    _rotateMenu->addAction( action );

    _popup->insertItem( QIcon(), i18n("Rotate"), _rotateMenu );
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

void Viewer::ViewerWidget::createSkipMenu()
{
#ifdef TEMPORARILY_REMOVED
    QMenu *popup = new QMenu( _popup );

    KAction* action = _actions->addAction( "viewer-home", this, SLOT( showFirst() ) );
    KAction->setText( i18n("First") );
    KAction->setIcon( Qt::Key_Home );
    popup->addAction( action );
    _backwardActions.append(action);

    action = _actions->addAction( "viewer-end", this, SLOT( showLast() ) );
    action->setText( i18n("Last") );
    action->setIcon( Qt::Key_End );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( "viewer-next", this, SLOT( showNext() ) );
    action->setText( i18n("Show Next") );
    action->setIcon( Qt::Key_PageDown );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( "viewer-next-10", this, SLOT( showNext10() ) );
    action->setText( i18n("Skip 10 Forward") );
    action->setIcon( Qt::CTRL+Qt::Key_PageDown );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( "viewer-next-100", this, SLOT( showNext100() ) );
    action->setText( i18n("Skip 100 Forward") );
    action->setIcon( SHIFT+Qt::Key_PageDown );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( "viewer-next-1000", this, SLOT( showNext1000() ) );
    action->setText( i18n("Skip 1000 Forward") );
    action->setIcon( Qt::CTRL+SHIFT+Qt::Key_PageDown );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( "viewer-prev", this, SLOT( showPrev() ) );
    action->setText( i18n("Show Previous") );
    action->setIcon( Qt::Key_PageUp );
    popup->addAction( action );
    _backwardActions.append(action);

    action = _actions->addAction( "viewer-prev-10", this, SLOT( showPrev10() ) );
    action->setText( i18n("Skip 10 Backward") );
    action->setIcon( Qt::CTRL+Qt::Key_PageUp );
    popup->addAction( action );
    _backwardActions.append(action);

    action = _actions->addAction( "viewer-prev-100", this, SLOT( showPrev100() ) );
    action->setText( i18n("Skip 100 Backward") );
    action->setIcon( SHIFT+Qt::Key_PageUp );
    popup->addAction( action );
    _backwardActions.append(action);

    action = _actions->addAction( "viewer-prev-1000", this, SLOT( showPrev1000() ) );
    action->setText( i18n("Skip 1000 Backward") );
    action->setIcon( Qt::CTRL+SHIFT+Qt::Key_PageUp );
    popup->addAction( action );
    _backwardActions.append(action);

    _popup->insertItem( QIcon(), i18n("Skip"), popup );
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

void Viewer::ViewerWidget::createZoomMenu()
{
#ifdef TEMPORARILY_REMOVED
    QMenu *popup = new QMenu( _popup );

    // PENDING(blackie) Only for image display?
    KAction* action = _actions->addAction( "viewer-zoom-in", this, SLOT( zoomIn() ) );
    KAction->setText( i18n("Zoom In") );
    KAction->setIcon( Qt::Key_Plus );
    popup->addAction( action );

    action = _actions->addAction( "viewer-zoom-out", this, SLOT( zoomOut() ) );
    action->setText( i18n("Zoom Out") );
    action->setIcon( Qt::Key_Minus );
    popup->addAction( action );

    action = _actions->addAction( "viewer-zoom-full", this, SLOT( zoomFull() ) );
    action->setText( i18n("Full View") );
    action->setIcon( Qt::Key_Period );
    popup->addAction( action );

    action = _actions->addAction( "viewer-zoom-pixel", this, SLOT( zoomPixelForPixel() ) );
    action->setText( i18n("Pixel for Pixel View") );
    action->setIcon( Qt::Key_Equal );
    popup->addAction( action );

    action = _actions->addAction( "viewer-toggle-fullscreen", this, SLOT( toggleFullScreen() ) );
    action->setText( i18n("Toggle Full Screen") );
    action->setIcon( Qt::Key_Return );
    popup->addAction( action );

    _popup->insertItem( QIcon(), i18n("Zoom"), popup );
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}


void Viewer::ViewerWidget::createSlideShowMenu()
{
#ifdef TEMPORARILY_REMOVED
    QMenu *popup = new QMenu( _popup );

    _startStopSlideShow = _actions->addAction( "viewer-start-stop-slideshow", this, SLOT( slotStartStopSlideShow() ) );
    _startStopSlideShow->setText( i18n("Run Slideshow") );
    _startStopSlideShow->setIcon( Qt::CTRL+Qt::Key_R );
    popup->addAction( _startStopSlideShow );

    _slideShowRunFaster = _actions->addAction( "viewer-run-faster", this, SLOT( slotSlideShowFaster() ) );
    _slideShowRunFaster->setText( i18n("Run Faster") );
    _slideShowRunFaster->setIcon( Qt::CTRL + Qt::Key_Plus );
    popup->addAction( _slideShowRunFaster );

    _slideShowRunSlower = _actions->addAction( "viewer-run-slower", this, SLOT( slotSlideShowSlower() ) );
    _slideShowRunSlower->setText( i18n("Run Slower") );
    _slideShowRunSlower->setIcon( Qt::CTRL+Qt::Key_Minus );
    popup->addAction( _slideShowRunSlower );

    _popup->insertItem( QIcon(), i18n("Slideshow"), popup );
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}


void Viewer::ViewerWidget::load( const QStringList& list, int index )
{
    _list = list;
    _imageDisplay->setImageList( list );
    _current = index;
    load();

    bool on = ( list.count() > 1 );
#ifdef TEMPORARILY_REMOVED
    _startStopSlideShow->setEnabled(on);
    _slideShowRunFaster->setEnabled(on);
    _slideShowRunSlower->setEnabled(on);
#else
    kDebug() << "TEMPORARILY REMOVED " << k_funcinfo << endl;
#endif
}

void Viewer::ViewerWidget::load()
{
    bool isReadable = QFileInfo( currentInfo()->fileName() ).isReadable();
    bool isVideo = isReadable && Utilities::isVideo( currentInfo()->fileName() );

    if ( isReadable ) {
        if ( isVideo )
            _display = _videoDisplay;
        else
            _display = _imageDisplay;
    } else {
        _display = _textDisplay;
        _textDisplay->setText( i18n("File not available") );
        updateInfoBox();
    }

    _stack->raiseWidget( _display );

#ifdef TEMPORARILY_REMOVED
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
#else
    kDebug() << "TEMPORARILY REMOVED " << k_funcinfo << endl;
#endif

    if ( _display->offersDrawOnImage() )
        _display->drawHandler()->setDrawList( currentInfo()->drawList() );
    bool ok = _display->setImage( currentInfo(), _forward );
    if ( !ok ) {
        close( false );
        return;
    }

    setCaptionWithDetail( QString() );

    // PENDING(blackie) This needs to be improved, so that it shows the actions only if there are that many images to jump.
    for( Q3PtrList<QAction>::const_iterator it = _forwardActions.begin(); it != _forwardActions.end(); ++it )
      (*it)->setEnabled( _current +1 < (int) _list.count() );
    for( Q3PtrList<QAction>::const_iterator it = _backwardActions.begin(); it != _forwardActions.end(); ++it )
      (*it)->setEnabled( _current > 0 );
    if ( isVideo )
        updateCategoryConfig();

    if ( _isRunningSlideShow )
        _slideShowTimer->start( _slideShowPause );

    if ( _display == _textDisplay )
        updateInfoBox();
}

void Viewer::ViewerWidget::setCaptionWithDetail( const QString& detail ) {
    setCaption( QString::fromLatin1( "KPhotoAlbum - %1 %2" )
                .arg( currentInfo()->fileName() )
                .arg( detail ) );
}

void Viewer::ViewerWidget::contextMenuEvent( QContextMenuEvent * e )
{
    _popup->exec( e->globalPos() );
    e->accept();
}

void Viewer::ViewerWidget::showNextN(int n)
{
    if ( _display == _videoDisplay )
        _videoDisplay->stop();

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
    if ( _display == _videoDisplay )
        _videoDisplay->stop();

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
#ifdef TEMPORARILY_REMOVED
    if(mode>7 || mode<1) return;
    DCOPRef kdesktop("kdesktop","KBackgroundIface");
    kdesktop.send("setWallpaper(QString,int)",currentInfo()->fileName(0),mode);
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

bool Viewer::ViewerWidget::close( bool alsoDelete)
{
    save();
    _slideShowTimer->stop();
    _isRunningSlideShow = false;
    return QWidget::close( alsoDelete );
}

DB::ImageInfoPtr Viewer::ViewerWidget::currentInfo() const
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
        QString text = QString::fromLatin1("<p>") + origText + QString::fromLatin1("</p>");
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
#ifdef TEMPORARILY_REMOVED
    KIconLoader loader;
    KActionCollection* actions = new KActionCollection( this, "actions" );
    _toolbar = new KToolBar( this );
    DrawHandler* handler = _imageDisplay->drawHandler();
{
    _select = actions->add<KToggleAction>( "_select", handler, SLOT( slotSelect() ));
    _select->setText( i18n("Select") );
    _select->setIcon( loader.loadIcon(QString::fromLatin1("selecttool"), K3Icon::Toolbar) );
    _toolbar->addAction( _select );
    _select->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _line = actions->add<KToggleAction>( "_line", handler, SLOT( slotLine() ) );
    _line->setText( i18n("Line") );
    _line->setIcon( loader.loadIcon(QString::fromLatin1("linetool"), K3Icon::Toolbar) );
    _toolbar->addAction( _line );
    _line->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _rect = actions->add<KToggleAction>( "_rect", handler, SLOT( slotRectangle() ) );
    _rect->setText( i18n("Rectangle") );
    _rect->setIcon( loader.loadIcon(QString::fromLatin1("recttool"), K3Icon::Toolbar) );
    _toolbar->addAction( _rect );
    _rect->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _circle = actions->add<KToggleAction>( "_circle", handler, SLOT( slotCircle() ) );
    _circle->setText( i18n("Circle") );
    _circle->setIcon( loader.loadIcon(QString::fromLatin1("ellipsetool"), K3Icon::Toolbar) );
    _toolbar->addAction( _circle );
    _circle->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _delete = KStandardAction::cut( handler, SLOT( cut() ), actions, "cutAction" );
    _toolbar->addAction( _delete );

    KAction* close = KStandardAction::close( this,  SLOT( stopDraw() ),  actions,  "stopDraw" );
    _toolbar->addAction( close );
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
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
        _startStopSlideShow->setText( i18n("Run Slideshow") );
        _slideShowTimer->stop();
        if ( _list.count() != 1 )
            _speedDisplay->end();
    }
    else {
        _startStopSlideShow->setText( i18n("Stop Slideshow") );
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
    int ms = qMax( 200, _slideShowPause - timer.elapsed() );
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
    _slideShowPause = qMax( _slideShowPause, 500 );
    _speedDisplay->display( _slideShowPause );
    if (_slideShowTimer->isActive() )
        _slideShowTimer->start( _slideShowPause );
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
#ifdef TEMPORARILY_REMOVED
    if ( on ) {
        // To avoid that the image is first loaded in a small size and the reloaded when scaled up, we need to resize the window right away.
        // (this results in odd behaviour (the image
        // 'jumps' because fullscreen > fullwindow) and should be
        // reconsidered. Henner.)
        resize( qApp->desktop()->screenGeometry().size() );
        KWindowSystem::setState( winId(), NET::FullScreen );
        moveInfoBox();
    }
    else {
        // We need to size the image when going out of full screen, in case we started directly in full screen
        //
        KWindowSystem::clearState( winId(), NET::FullScreen );
        resize( Settings::SettingsData::instance()->viewerSize() );
    }
    _showingFullScreen = on;
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
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
}

KActionCollection* Viewer::ViewerWidget::actions()
{
    return _actions;
}

void Viewer::ViewerWidget::keyPressEvent( QKeyEvent* event )
{
    if ( event->stateAfter() == 0 && event->state() == 0 && ( event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z ) ) {
        QString token = event->text().toUpper().left(1);
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

    _play = _actions->addAction( "viewer-video-play", this, SLOT( play() ) );
    _play->setText( i18n("Play") );

    _stop = _actions->addAction( "viewer-video-stop", this, SLOT( stop() ) );
    _stop->setText( i18n("Stop") );

    _pause = _actions->addAction( "viewer-video-pause", this, SLOT( pause() ) );
    _pause->setText( i18n("Pause") );

    _restart = _actions->addAction( "viewer-video-restart", this, SLOT( restart() ) );
    _restart->setText( i18n("Restart") );
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
