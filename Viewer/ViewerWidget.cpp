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

#include <config-kpa-exiv2.h>

#include <kdeversion.h>
#include "Viewer/ViewerWidget.h"
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QList>
#include <QResizeEvent>
#include <QWheelEvent>
#include <kiconloader.h>
#include <kaction.h>
#include <klocale.h>
#include "Utilities/Util.h"
#include <qtimer.h>
#include <kwindowsystem.h>
#include "SpeedDisplay.h"
#include "MainWindow/Window.h"
#include "CategoryImageConfig.h"
#include "MainWindow/ExternalPopup.h"
#include "MainWindow/CategoryImagePopup.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "InfoBox.h"
#include "VideoDisplay.h"
#include "MainWindow/DirtyIndicator.h"
#include "ViewerWidget.h"
#include "VisibleOptionsMenu.h"
#include <qglobal.h>
#include <QTimeLine>
#include <QTimer>
#include "ImageDisplay.h"
#include <qapplication.h>
#include <qeventloop.h>
#include <qfileinfo.h>
#include "TextDisplay.h"
#include <kdebug.h>
#include <KActionCollection>
#include <KStandardAction>
#include <QStackedWidget>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <KProcess>
#include <KStandardDirs>

#ifdef HAVE_EXIV2
#  include "Exif/InfoDialog.h"
#endif

Viewer::ViewerWidget* Viewer::ViewerWidget::_latest = 0;

Viewer::ViewerWidget* Viewer::ViewerWidget::latest()
{
    return _latest;
}


// Notice the parent is zero to allow other windows to come on top of it.
Viewer::ViewerWidget::ViewerWidget()
    :QStackedWidget( 0 ), _current(0), _popup(0), _showingFullScreen( false ), _forward( true ), _isRunningSlideShow( false ), _videoPlayerStoppedManually(false)
{
    setWindowFlags( Qt::Window );
    setAttribute( Qt::WA_DeleteOnClose );

    _latest = this;

    _display = _imageDisplay = new ImageDisplay( this );
    addWidget( _imageDisplay );

    _textDisplay = new TextDisplay( this );
    addWidget( _textDisplay );

    createVideoViewer();

    connect( _imageDisplay, SIGNAL( possibleChange() ), this, SLOT( updateCategoryConfig() ) );
    connect( _imageDisplay, SIGNAL( imageReady() ), this, SLOT( updateInfoBox() ) );
    connect( _imageDisplay, SIGNAL( setCaptionInfo(const QString&) ),
             this, SLOT( setCaptionWithDetail(const QString&) ) );

    // This must not be added to the layout, as it is standing on top of
    // the ImageDisplay
    _infoBox = new InfoBox( this );
    _infoBox->hide();

    setupContextMenu();

    _slideShowTimer = new QTimer( this );
    _slideShowTimer->setSingleShot( true );
    _slideShowPause = Settings::SettingsData::instance()->slideShowInterval() * 1000;
    connect( _slideShowTimer, SIGNAL( timeout() ), this, SLOT( slotSlideShowNextFromTimer() ) );
    _speedDisplay = new SpeedDisplay( this );
    _speedDisplay->hide();

    setFocusPolicy( Qt::StrongFocus );
#ifndef Q_WS_WIN
    const QString xdgScreenSaver = KStandardDirs::findExe( QString::fromAscii("xdg-screensaver") );
    if ( !xdgScreenSaver.isEmpty() ) {
        KProcess proc;
        proc << xdgScreenSaver;
        proc << QString::fromLatin1("suspend");
        proc << QString::number( winId() );
        proc.startDetached();
    }
#endif
    QTimer::singleShot( 2000, this, SLOT(test()) );
}

void Viewer::ViewerWidget::setupContextMenu()
{
    _popup = new QMenu( this );
    _actions = new KActionCollection( this );

    createSlideShowMenu();
    createZoomMenu();
    createRotateMenu();
    createSkipMenu();
    createShowContextMenu();
    createWallPaperMenu();
    createInvokeExternalMenu();
    createVideoMenu();
    createCategoryImageMenu();

    KAction* action = _actions->addAction( QString::fromLatin1("viewer-edit-image-properties"), this, SLOT( editImage() ) );
    action->setText( i18n("Annotate...") );
    action->setShortcut( Qt::CTRL+Qt::Key_1 );
    _popup->addAction( action );

    _setStackHead = _actions->addAction( QString::fromLatin1("viewer-set-stack-head"), this, SLOT( slotSetStackHead() ) );
    _setStackHead->setText( i18n("Set as First Image in Stack") );
    _setStackHead->setShortcut( Qt::CTRL+Qt::Key_4 );
    _popup->addAction( _setStackHead );

#ifdef HAVE_EXIV2
    _showExifViewer = _actions->addAction( QString::fromLatin1("viewer-show-exif-viewer"), this, SLOT( showExifViewer() ) );
    _showExifViewer->setText( i18n("Show EXIF Viewer") );
    _popup->addAction( _showExifViewer );
#endif

    action = _actions->addAction( QString::fromLatin1("viewer-close"), this, SLOT( close() ) );
    action->setText( i18n("Close") );
    action->setShortcut( Qt::Key_Escape );

    _popup->addAction( action );
    _actions->readSettings();

    Q_FOREACH( QAction* action, _actions->actions() ) {
      action->setShortcutContext(Qt::WindowShortcut);
      addAction(action);
    }
}

void Viewer::ViewerWidget::createShowContextMenu()
{
    VisibleOptionsMenu* menu = new VisibleOptionsMenu( this, _actions );
    connect( menu, SIGNAL( visibleOptionsChanged() ), this, SLOT( updateInfoBox()  ) );
    _popup->addMenu( menu );
}

void Viewer::ViewerWidget::createWallPaperMenu()
{
    // Setting wallpaper has still not yet been ported to KPA4
#ifndef DOES_STILL_NOT_WORK_IN_KPA4
    _wallpaperMenu = new QMenu( _popup );
    _wallpaperMenu->setTitle( i18n("Set as Wallpaper") );

    KAction* action = _actions->addAction( QString::fromLatin1("viewer-centered"), this, SLOT( slotSetWallpaperC() ) );
    action->setText( i18n("Centered") );
    _wallpaperMenu->addAction(action);

    action = _actions->addAction( QString::fromLatin1("viewer-tiled"), this, SLOT( slotSetWallpaperT() ) );
    action->setText( i18n("Tiled") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-center-tiled"), this, SLOT( slotSetWallpaperCT() ) );
    action->setText( i18n("Center Tiled") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-centered-maxspect"), this, SLOT( slotSetWallpaperCM() ) );
    action->setText( i18n("Centered Maxpect") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-tiled-maxpect"), this, SLOT( slotSetWallpaperTM() ) );
    action->setText( i18n("Tiled Maxpect") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-scaled"), this, SLOT( slotSetWallpaperS() ) );
    action->setText( i18n("Scaled") );
    _wallpaperMenu->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-centered-auto-fit"), this, SLOT( slotSetWallpaperCAF() ) );
    action->setText( i18n("Centered Auto Fit") );
    _wallpaperMenu->addAction( action );

    _popup->addMenu( _wallpaperMenu );
#endif // DOES_STILL_NOT_WORK_IN_KPA4
}

void Viewer::ViewerWidget::createInvokeExternalMenu()
{
    _externalPopup = new MainWindow::ExternalPopup( _popup );
    _popup->addMenu( _externalPopup );
    connect( _externalPopup, SIGNAL( aboutToShow() ), this, SLOT( populateExternalPopup() ) );
}

void Viewer::ViewerWidget::createRotateMenu()
{
    _rotateMenu = new QMenu( _popup );
    _rotateMenu->setTitle( i18n("Rotate") );

    KAction* action = _actions->addAction( QString::fromLatin1("viewer-rotate90"), this, SLOT( rotate90() ) );
    action->setText( i18n("Rotate clockwise") );
    action->setShortcut( Qt::Key_9 );
    _rotateMenu->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-rotate180"), this, SLOT( rotate180() ) );
    action->setText( i18n("Flip Over") );
    action->setShortcut( Qt::Key_8 );
    _rotateMenu->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-rotare270"), this, SLOT( rotate270() ) );
    action->setText( i18n("Rotate counterclockwise") );
    action->setShortcut( Qt::Key_7 );
    _rotateMenu->addAction( action );

    _popup->addMenu( _rotateMenu );
}

void Viewer::ViewerWidget::createSkipMenu()
{
    QMenu *popup = new QMenu( _popup );
    popup->setTitle( i18n("Skip") );

    KAction* action = _actions->addAction( QString::fromLatin1("viewer-home"), this, SLOT( showFirst() ) );
    action->setText( i18n("First") );
    action->setShortcut( Qt::Key_Home );
    popup->addAction( action );
    _backwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-end"), this, SLOT( showLast() ) );
    action->setText( i18n("Last") );
    action->setShortcut( Qt::Key_End );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-next"), this, SLOT( showNext() ) );
    action->setText( i18n("Show Next") );
    action->setShortcut( Qt::Key_PageDown );
    // bah, they don't use references
    KShortcut viewerNextShortcut = action->shortcut();
    viewerNextShortcut.setAlternate( Qt::Key_Space );
    action->setShortcut( viewerNextShortcut );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-next-10"), this, SLOT( showNext10() ) );
    action->setText( i18n("Skip 10 Forward") );
    action->setShortcut( Qt::CTRL+Qt::Key_PageDown );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-next-100"), this, SLOT( showNext100() ) );
    action->setText( i18n("Skip 100 Forward") );
    action->setShortcut( Qt::SHIFT+Qt::Key_PageDown );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-next-1000"), this, SLOT( showNext1000() ) );
    action->setText( i18n("Skip 1000 Forward") );
    action->setShortcut( Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown );
    popup->addAction( action );
    _forwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-prev"), this, SLOT( showPrev() ) );
    action->setText( i18n("Show Previous") );
    action->setShortcut( Qt::Key_PageUp );
    popup->addAction( action );
    _backwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-prev-10"), this, SLOT( showPrev10() ) );
    action->setText( i18n("Skip 10 Backward") );
    action->setShortcut( Qt::CTRL+Qt::Key_PageUp );
    popup->addAction( action );
    _backwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-prev-100"), this, SLOT( showPrev100() ) );
    action->setText( i18n("Skip 100 Backward") );
    action->setShortcut( Qt::SHIFT+Qt::Key_PageUp );
    popup->addAction( action );
    _backwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-prev-1000"), this, SLOT( showPrev1000() ) );
    action->setText( i18n("Skip 1000 Backward") );
    action->setShortcut( Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp );
    popup->addAction( action );
    _backwardActions.append(action);

    action = _actions->addAction( QString::fromLatin1("viewer-remove-current"), this, SLOT( removeCurrent() ) );
    action->setText( i18n("Remove Image from Display List") );
    action->setShortcut( Qt::Key_Delete );
    popup->addAction( action );

    _popup->addMenu( popup );
}

void Viewer::ViewerWidget::createZoomMenu()
{
    QMenu *popup = new QMenu( _popup );
    popup->setTitle( i18n("Zoom") );

    // PENDING(blackie) Only for image display?
    KAction* action = _actions->addAction( QString::fromLatin1("viewer-zoom-in"), this, SLOT( zoomIn() ) );
    action->setText( i18n("Zoom In") );
    action->setShortcut( Qt::Key_Plus );
    popup->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-zoom-out"), this, SLOT( zoomOut() ) );
    action->setText( i18n("Zoom Out") );
    action->setShortcut( Qt::Key_Minus );
    popup->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-zoom-full"), this, SLOT( zoomFull() ) );
    action->setText( i18n("Full View") );
    action->setShortcut( Qt::Key_Period );
    popup->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-zoom-pixel"), this, SLOT( zoomPixelForPixel() ) );
    action->setText( i18n("Pixel for Pixel View") );
    action->setShortcut( Qt::Key_Equal );
    popup->addAction( action );

    action = _actions->addAction( QString::fromLatin1("viewer-toggle-fullscreen"), this, SLOT( toggleFullScreen() ) );
    action->setText( i18n("Toggle Full Screen") );
    action->setShortcut( Qt::Key_Return );
    popup->addAction( action );

    _popup->addMenu( popup );
}


void Viewer::ViewerWidget::createSlideShowMenu()
{
    QMenu *popup = new QMenu( _popup );
    popup->setTitle( i18n("Slideshow") );

    _startStopSlideShow = _actions->addAction( QString::fromLatin1("viewer-start-stop-slideshow"), this, SLOT( slotStartStopSlideShow() ) );
    _startStopSlideShow->setText( i18n("Run Slideshow") );
    _startStopSlideShow->setShortcut( Qt::CTRL+Qt::Key_R );
    popup->addAction( _startStopSlideShow );

    _slideShowRunFaster = _actions->addAction( QString::fromLatin1("viewer-run-faster"), this, SLOT( slotSlideShowFaster() ) );
    _slideShowRunFaster->setText( i18n("Run Faster") );
    _slideShowRunFaster->setShortcut( Qt::CTRL + Qt::Key_Plus );
    popup->addAction( _slideShowRunFaster );

    _slideShowRunSlower = _actions->addAction( QString::fromLatin1("viewer-run-slower"), this, SLOT( slotSlideShowSlower() ) );
    _slideShowRunSlower->setText( i18n("Run Slower") );
    _slideShowRunSlower->setShortcut( Qt::CTRL+Qt::Key_Minus );
    popup->addAction( _slideShowRunSlower );

    _popup->addMenu( popup );
}


void Viewer::ViewerWidget::load( const QStringList& list, int index )
{
    _list = list;
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
    bool isReadable = QFileInfo( currentInfo()->fileName(DB::AbsolutePath) ).isReadable();
    bool isVideo = isReadable && Utilities::isVideo( currentInfo()->fileName(DB::AbsolutePath) );

    if ( isReadable ) {
        if ( isVideo ) {
            _display = _videoDisplay;
        }
        else
            _display = _imageDisplay;
    } else {
        _display = _textDisplay;
        _textDisplay->setText( i18n("File not available") );
        updateInfoBox();
    }

    setCurrentWidget( _display );
    _infoBox->raise();

    _rotateMenu->setEnabled( !isVideo );
    _wallpaperMenu->setEnabled( !isVideo );
    _categoryImagePopup->setEnabled( !isVideo );
#ifdef HAVE_EXIV2
    _showExifViewer->setEnabled( !isVideo );
#endif

    Q_FOREACH( QAction* videoAction, _videoActions ) {
        videoAction->setVisible( isVideo );
    }

    emit soughtTo( DB::ImageDB::instance()->ID_FOR_FILE( _list[ _current ] ) );

    bool ok = _display->setImage( currentInfo(), _forward );
    if ( !ok ) {
        close( false );
        return;
    }

    setCaptionWithDetail( QString() );

    // PENDING(blackie) This needs to be improved, so that it shows the actions only if there are that many images to jump.
    for( QList<KAction*>::const_iterator it = _forwardActions.constBegin(); it != _forwardActions.constEnd(); ++it )
        (*it)->setEnabled( _current +1 < (int) _list.count() );
    for( QList<KAction*>::const_iterator it = _backwardActions.constBegin(); it != _backwardActions.constEnd(); ++it )
        (*it)->setEnabled( _current > 0 );

    _setStackHead->setEnabled( currentInfo()->isStacked() );

    if ( isVideo )
        updateCategoryConfig();

    if ( _isRunningSlideShow )
        _slideShowTimer->start( _slideShowPause );

    if ( _display == _textDisplay )
        updateInfoBox();
}

void Viewer::ViewerWidget::setCaptionWithDetail( const QString& detail ) {
    setWindowTitle( QString::fromLatin1( "KPhotoAlbum - %1 %2" )
                    .arg( currentInfo()->fileName(DB::AbsolutePath) )
                    .arg( detail ) );
}

void Viewer::ViewerWidget::contextMenuEvent( QContextMenuEvent * e )
{
    if ( _videoDisplay ) {
        if ( _videoDisplay->isPaused() )
            _playPause->setText(i18n("Play"));
        else
            _playPause->setText(i18n("Pause"));

        _stop->setEnabled( _videoDisplay->isPlaying() );
    }

    _popup->exec( e->globalPos() );
    e->setAccepted(true);
}

void Viewer::ViewerWidget::showNextN(int n)
{
    if ( _display == _videoDisplay ) {
        _videoPlayerStoppedManually = true;
        _videoDisplay->stop();
    }

    if ( _current + n < (int) _list.count() )  {
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

void Viewer::ViewerWidget::removeCurrent()
{
    _list.removeAll(_list[_current]);
    qDebug("%d,%d", _current, _list.count() );
    if ( _current == _list.count() ) {
        qDebug("Show prev");
        showPrev();
    }
    else
        showNextN(0);
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
    emit rotated();
}

void Viewer::ViewerWidget::rotate180()
{
    currentInfo()->rotate( 180 );
    load();
    emit rotated();
}

void Viewer::ViewerWidget::rotate270()
{
    currentInfo()->rotate( 270 );
    load();
    emit rotated();
}

void Viewer::ViewerWidget::showFirst()
{
    showPrevN(_list.count());
}

void Viewer::ViewerWidget::showLast()
{
    showNextN(_list.count());
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

void Viewer::ViewerWidget::setAsWallpaper(int /*mode*/)
{
#ifdef DOES_STILL_NOT_WORK_IN_KPA4
    if(mode>7 || mode<1) return;
    DCOPRef kdesktop("kdesktop","KBackgroundIface");
    kdesktop.send("setWallpaper(QString,int)",currentInfo()->fileName(0),mode);
#endif
}

bool Viewer::ViewerWidget::close( bool alsoDelete)
{
    _slideShowTimer->stop();
    _isRunningSlideShow = false;
    return QWidget::close();
    if ( alsoDelete )
        deleteLater();
}

DB::ImageInfoPtr Viewer::ViewerWidget::currentInfo() const
{
    return DB::ImageDB::instance()->info(_list[ _current], DB::AbsolutePath); // PENDING(blackie) can we postpone this lookup?
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
        QString text = Utilities::createInfoText( currentInfo(), &map );
        if ( Settings::SettingsData::instance()->showInfoBox() && !text.isNull() ) {
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
#ifndef Q_WS_WIN
    const QString xdgScreenSaver = KStandardDirs::findExe( QString::fromAscii("xdg-screensaver") );
    if ( !xdgScreenSaver.isEmpty() ) {
        KProcess proc;
        proc << xdgScreenSaver << QLatin1String("resume") << QString::number( winId() );
        // if we don't wait here, xdg-screensaver realizes that the window is
        // already gone and doesn't re-activate the screensaver
        proc.execute();
    }
#endif

    if ( _latest == this )
        _latest = 0;
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
            _slideShowTimer->start( _slideShowPause );
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
    _slideShowTimer->start( ms );
}

void Viewer::ViewerWidget::slotSlideShowNext()
{
    _forward = true;
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

void Viewer::ViewerWidget::slotSetStackHead()
{
    MainWindow::Window::theMainWindow()->setStackHead( DB::ImageDB::instance()->ID_FOR_FILE( _list[ _current ] ) );
}

bool Viewer::ViewerWidget::showingFullScreen() const
{
    return _showingFullScreen;
}

void Viewer::ViewerWidget::setShowFullScreen( bool on )
{
    if ( on ) {
        // To avoid that the image is first loaded in a small size and the reloaded when scaled up, we need to resize the window right away.
        // (this results in odd behaviour (the image
        // 'jumps' because fullscreen > fullwindow) and should be
        // reconsidered. Henner.)
        resize( qApp->desktop()->screenGeometry().size() );
        setWindowState( windowState() | Qt::WindowFullScreen ); // set
        moveInfoBox();
    }
    else {
        // We need to size the image when going out of full screen, in case we started directly in full screen
        //
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset
        resize( Settings::SettingsData::instance()->viewerSize() );
    }
    _showingFullScreen = on;
}

void Viewer::ViewerWidget::updateCategoryConfig()
{
    if ( !CategoryImageConfig::instance()->isVisible() )
        return;

    CategoryImageConfig::instance()->setCurrentImage( _imageDisplay->currentViewAsThumbnail(), currentInfo() );
}


void Viewer::ViewerWidget::populateExternalPopup()
{
    _externalPopup->populate( currentInfo(), _list );
}

void Viewer::ViewerWidget::populateCategoryImagePopup()
{
    _categoryImagePopup->populate( _imageDisplay->currentViewAsThumbnail(), _list[_current] );
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
    if ( event->modifiers() == 0 && event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z ) {
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
    if ( !_videoPlayerStoppedManually && _isRunningSlideShow )
        slotSlideShowNext();
    _videoPlayerStoppedManually=false;
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
#ifdef HAVE_EXIV2
    Exif::InfoDialog* exifDialog = new Exif::InfoDialog( DB::ImageDB::instance()->ID_FOR_FILE(currentInfo()->fileName(DB::AbsolutePath)), this );
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


struct SeekInfo
{
    SeekInfo( const QString& title, const char* name, int value, const QKeySequence& key )
        : title( title ), name(name), value(value), key(key) {}

    QString title;
    const char* name;
    int value;
    QKeySequence key;
};

void Viewer::ViewerWidget::createVideoMenu()
{
    QMenu* menu = new QMenu(_popup);
    menu->setTitle(i18n("Seek"));
    _videoActions.append( _popup->addMenu( menu ) );

    QList<SeekInfo> list;
    list << SeekInfo( i18n("10 minutes backward"), "seek-10-minute", -600000, QKeySequence(QString::fromLatin1("Ctrl+Left")))
         << SeekInfo( i18n("1 minute backward"), "seek-1-minute", -60000, QKeySequence(QString::fromLatin1( "Shift+Left")))
         << SeekInfo( i18n("10 seconds backward"), "seek-10-second", -10000, QKeySequence(QString::fromLatin1( "Left")))
         << SeekInfo( i18n("1 seconds backward"), "seek-1-second", -1000, QKeySequence(QString::fromLatin1( "Up")))
         << SeekInfo( i18n("100 milliseconds backward"), "seek-100-millisecond", -100, QKeySequence(QString::fromLatin1( "Shift+Up")))
         << SeekInfo( i18n("100 milliseconds forward"), "seek+100-millisecond", 100, QKeySequence(QString::fromLatin1( "Shift+Down")))
         << SeekInfo( i18n("1 seconds forward"), "seek+1-second", 1000, QKeySequence(QString::fromLatin1( "Down")))
         << SeekInfo( i18n("10 seconds forward"), "seek+10-second", 10000, QKeySequence(QString::fromLatin1( "Right")))
         << SeekInfo( i18n("1 minute forward"), "seek+1-minute", 60000, QKeySequence(QString::fromLatin1( "Shift+Right")))
         << SeekInfo( i18n("10 minutes forward"), "seek+10-minute", 600000, QKeySequence(QString::fromLatin1( "Ctrl+Right")));

    int count=0;
    Q_FOREACH( const SeekInfo& info, list ) {
        if ( count++ == 5 ) {
            QAction* sep = new QAction( menu );
            sep->setSeparator(true);
            menu->addAction(sep);
        }

        KAction* seek = _actions->addAction( QString::fromLatin1(info.name), _videoDisplay, SLOT(seek()));
        seek->setText(info.title);
        seek->setData(info.value);
        seek->setShortcut( info.key );
        menu->addAction(seek);
    }

    QAction* sep = new QAction(_popup);
    sep->setSeparator(true);
    _popup->addAction( sep );
    _videoActions.append( sep );

    _stop = _actions->addAction( QString::fromLatin1("viewer-video-stop"), _videoDisplay, SLOT( stop() ) );
    _stop->setText( i18n("Stop") );
    _popup->addAction( _stop );
    _videoActions.append(_stop);


    _playPause = _actions->addAction( QString::fromLatin1("viewer-video-pause"), _videoDisplay, SLOT( playPause() ) );
    _playPause->setText( i18n("Toggle playback") );
    _playPause->setShortcut( Qt::Key_P );
    _popup->addAction( _playPause );
    _videoActions.append( _playPause );
    KAction* restart = _actions->addAction( QString::fromLatin1("viewer-video-restart"), _videoDisplay, SLOT( restart() ) );
    restart->setText( i18n("Restart") );
    _popup->addAction( restart );
    _videoActions.append( restart );
}

void Viewer::ViewerWidget::createCategoryImageMenu()
{
    _categoryImagePopup = new MainWindow::CategoryImagePopup( _popup );
    _popup->addMenu( _categoryImagePopup );
    connect( _categoryImagePopup, SIGNAL( aboutToShow() ), this, SLOT( populateCategoryImagePopup() ) );
}

void Viewer::ViewerWidget::test()
{
#ifdef TESTING
    QTimeLine* timeline = new QTimeLine;
    timeline->setStartFrame( _infoBox->y() );
    timeline->setEndFrame( height() );
    connect( timeline, SIGNAL( frameChanged(int) ), this, SLOT( moveInfoBox(int) ) );
    timeline->start();
#endif // TESTING
}

void Viewer::ViewerWidget::moveInfoBox( int y)
{
    _infoBox->move( _infoBox->x(), y );
}

void Viewer::ViewerWidget::createVideoViewer()
{
    _videoDisplay = new VideoDisplay( this );
    addWidget( _videoDisplay );
    connect( _videoDisplay, SIGNAL( stopped() ), this, SLOT( videoStopped() ) );
}

#include "ViewerWidget.moc"
