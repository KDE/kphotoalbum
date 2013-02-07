/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#include "ViewerWidget.h"
#include <config-kpa-exiv2.h>

#include <kdeversion.h>
#include <QContextMenuEvent>
#include <QtDBus>
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
#include "ImageManager/ThumbnailCache.h"
#include <KMessageBox>
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
#include "MainWindow/DeleteDialog.h"
#include "VideoShooter.h"

#ifdef HAVE_EXIV2
#  include "Exif/InfoDialog.h"
#endif

Viewer::ViewerWidget* Viewer::ViewerWidget::_latest = 0;

Viewer::ViewerWidget* Viewer::ViewerWidget::latest()
{
    return _latest;
}


// Notice the parent is zero to allow other windows to come on top of it.
Viewer::ViewerWidget::ViewerWidget( UsageType type, QMap<Qt::Key, QPair<QString,QString> > *macroStore )
    :QStackedWidget( 0 ), _current(0), _popup(0), _showingFullScreen( false ), _forward( true ), _isRunningSlideShow( false ), _videoPlayerStoppedManually(false), _type(type), _currentCategory(QString::fromLatin1("Tokens")), _inputMacros(macroStore),  _myInputMacros(0)
{
    if ( type == ViewerWindow ) {
        setWindowFlags( Qt::Window );
        setAttribute( Qt::WA_DeleteOnClose );
        _latest = this;
    }

    if (! _inputMacros) {
        _myInputMacros = _inputMacros =
            new QMap<Qt::Key, QPair<QString,QString> >;
    }

    m_screenSaverCookie = -1;
    _currentInputMode = InACategory;

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
    createFilterMenu();

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

    if ( _type == ViewerWindow ) {
        action = _actions->addAction( QString::fromLatin1("viewer-close"), this, SLOT( close() ) );
        action->setText( i18n("Close") );
        action->setShortcut( Qt::Key_Escape );
    }

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


void Viewer::ViewerWidget::inhibitScreenSaver( bool inhibit ) {
    QDBusMessage message;
    if (inhibit) {
        message = QDBusMessage::createMethodCall( QString::fromLatin1("org.freedesktop.ScreenSaver"), QString::fromLatin1("/ScreenSaver"),
                                                  QString::fromLatin1("org.freedesktop.ScreenSaver"), QString::fromLatin1("Inhibit") );

        message << QString( QString::fromLatin1("KPhotoAlbum") );
        message << QString( QString::fromLatin1("Giving a slideshow") );
        QDBusMessage reply = QDBusConnection::sessionBus().call( message );
        if ( reply.type() == QDBusMessage::ReplyMessage )
            m_screenSaverCookie = reply.arguments().first().toInt();
    }
    else {
        if ( m_screenSaverCookie != -1 ) {
            message = QDBusMessage::createMethodCall( QString::fromLatin1("org.freedesktop.ScreenSaver"), QString::fromLatin1("/ScreenSaver"),
                                                      QString::fromLatin1("org.freedesktop.ScreenSaver"), QString::fromLatin1("UnInhibit") );
            message << (uint)m_screenSaverCookie;
            QDBusConnection::sessionBus().send( message );
            m_screenSaverCookie = -1;
        }
    }
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

    action = _actions->addAction( QString::fromLatin1("viewer-delete-current"), this, SLOT( deleteCurrent() ) );
    action->setText( i18n("Delete Image") );
    action->setShortcut( Qt::CTRL + Qt::Key_Delete );
    popup->addAction( action );

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


void Viewer::ViewerWidget::load( const DB::FileNameList& list, int index )
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
    const bool isReadable = QFileInfo( currentInfo()->fileName().absolute() ).isReadable();
    const bool isVideo = isReadable && Utilities::isVideo( currentInfo()->fileName() );

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
    _filterMenu->setEnabled( !isVideo );
#ifdef HAVE_EXIV2
    _showExifViewer->setEnabled( !isVideo );
    if ( _exifViewer )
        _exifViewer->setImage( currentInfo()->fileName() );
#endif

    Q_FOREACH( QAction* videoAction, _videoActions ) {
        videoAction->setVisible( isVideo );
    }

    emit soughtTo( _list[ _current ]);

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
                    .arg( currentInfo()->fileName().absolute() )
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
    filterNone();
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
    removeOrDeleteCurrent(OnlyRemoveFromViewer);
}

void Viewer::ViewerWidget::deleteCurrent()
{
    removeOrDeleteCurrent( RemoveImageFromDatabase );
}

void Viewer::ViewerWidget::removeOrDeleteCurrent( RemoveAction action )
{
    const DB::FileName fileName = _list[_current];

    if ( action == RemoveImageFromDatabase )
        _removed.append(fileName);
    _list.removeAll(fileName);
    if ( _list.isEmpty() )
        close();
    if ( _current == _list.count() )
        showPrev();
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
    invalidateThumbnail();
}

void Viewer::ViewerWidget::rotate180()
{
    currentInfo()->rotate( 180 );
    load();
    invalidateThumbnail();
}

void Viewer::ViewerWidget::rotate270()
{
    currentInfo()->rotate( 270 );
    load();
    invalidateThumbnail();
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
    if ( !_removed.isEmpty() ) {
        MainWindow::DeleteDialog dialog( this );
        dialog.exec( _removed );
    }

    _slideShowTimer->stop();
    _isRunningSlideShow = false;
    return QWidget::close();
    if ( alsoDelete )
        deleteLater();
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
    if ( currentInfo() || _currentInput != QString::fromLatin1("") ||
         (_currentCategory != QString::fromLatin1("") &&
          _currentCategory != QString::fromLatin1("Tokens"))) {
        QMap<int, QPair<QString,QString> > map;
        QString text = Utilities::createInfoText( currentInfo(), &map );
        QString selecttext = QString::fromLatin1("");
        if (_currentCategory == QString::fromLatin1("")) {
            selecttext = i18n("<b>Setting Category: </b>") + _currentInput;
            if (_currentInputList.length() > 0) {
                selecttext += QString::fromLatin1("{") + _currentInputList +
                    QString::fromLatin1("}");
            }
        } else if ( ( _currentInput != QString::fromLatin1("") &&
                   _currentCategory != QString::fromLatin1("Tokens") ) ||
                   _currentCategory != QString::fromLatin1("Tokens")) {
            selecttext = i18n("<b>Assigning: </b>") + _currentCategory +
                QString::fromLatin1("/")  + _currentInput;
            if (_currentInputList.length() > 0) {
                selecttext += QString::fromLatin1("{") + _currentInputList +
                    QString::fromLatin1("}");
            }
        } else if ( _currentInput != QString::fromLatin1("") &&
                   _currentCategory == QString::fromLatin1("Tokens") ) {
            _currentInput = QString::fromLatin1("");
        }
        if (selecttext != QString::fromLatin1(""))
            text = selecttext + QString::fromLatin1("<br />") + text;
        if ( Settings::SettingsData::instance()->showInfoBox() && !text.isNull() && ( _type != InlineViewer ) ) {
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
    inhibitScreenSaver(false);

    if ( _latest == this )
        _latest = 0;

    if ( _myInputMacros )
        delete _myInputMacros;
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
        inhibitScreenSaver(false);
    }
    else {
        _startStopSlideShow->setText( i18n("Stop Slideshow") );
        if ( currentInfo()->mediaType() != DB::Video )
            _slideShowTimer->start( _slideShowPause );
        _speedDisplay->start();
        inhibitScreenSaver(true);
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

void Viewer::ViewerWidget::filterNone()
{
    if ( _display == _imageDisplay ) {
        _imageDisplay->filterNone();
        _filterMono->setChecked( false );
        _filterBW->setChecked( false );
        _filterContrastStretch->setChecked( false );
        _filterHistogramEqualization->setChecked( false );
    }
}

void Viewer::ViewerWidget::filterSelected()
{
    // The filters that drop bit depth below 32 should be the last ones
    // so that filters requiring more bit depth are processed first
    if ( _display == _imageDisplay ) {
        _imageDisplay->filterNone();
        if (_filterBW->isChecked())
            _imageDisplay->filterBW();
        if (_filterContrastStretch->isChecked())
            _imageDisplay->filterContrastStretch();
        if (_filterHistogramEqualization->isChecked())
            _imageDisplay->filterHistogramEqualization();
        if (_filterMono->isChecked())
            _imageDisplay->filterMono();
    }
}

void Viewer::ViewerWidget::filterBW()
{
    if ( _display == _imageDisplay ) {
        if ( _filterBW->isChecked() )
            _filterBW->setChecked( _imageDisplay->filterBW());
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::filterContrastStretch()
{
    if ( _display == _imageDisplay ) {
        if (_filterContrastStretch->isChecked())
            _filterContrastStretch->setChecked( _imageDisplay->filterContrastStretch() );
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::filterHistogramEqualization()
{
    if ( _display == _imageDisplay ) {
        if ( _filterHistogramEqualization->isChecked() )
            _filterHistogramEqualization->setChecked( _imageDisplay->filterHistogramEqualization() );
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::filterMono()
{
    if ( _display == _imageDisplay ) {
        if ( _filterMono->isChecked() )
            _filterMono->setChecked( _imageDisplay->filterMono() );
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::slotSetStackHead()
{
    MainWindow::Window::theMainWindow()->setStackHead(_list[ _current ]);
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

int Viewer::ViewerWidget::find_tag_in_list(const QStringList &list,
                                           QString &namefound)
{
    int found = 0;
    _currentInputList = QString::fromLatin1("");
    for( QStringList::ConstIterator listIter = list.constBegin();
         listIter != list.constEnd(); ++listIter ) {
        if (listIter->startsWith(_currentInput, Qt::CaseInsensitive)) {
            found++;
            if (_currentInputList.length() > 0)
                _currentInputList =
                    _currentInputList + QString::fromLatin1(",");
            _currentInputList =_currentInputList +
                listIter->right(listIter->length() - _currentInput.length());
            if (found > 1 && _currentInputList.length() > 20) {
                // already found more than we want to display
                // bail here for now
                // XXX: non-ideal?  display more?  certainly config 20
                return found;
            } else {
                namefound = *listIter;
            }
        }
    }
    return found;
}

void Viewer::ViewerWidget::keyPressEvent( QKeyEvent* event )
{

    if (event->key() == Qt::Key_Backspace) {
        // remove stuff from the current input string
        _currentInput.remove( _currentInput.length()-1, 1 );
        updateInfoBox();
        MainWindow::DirtyIndicator::markDirty();
        _currentInputList = QString::fromLatin1("");
//     } else if (event->modifier & (Qt::AltModifier | Qt::MetaModifier) &&
//                event->key() == Qt::Key_Enter) {
        return; // we've handled it
    } else if (event->key() == Qt::Key_Comma) {
        // force set the "new" token
        if (_currentCategory != QString::fromLatin1("")) {
            if (_currentInput.left(1) == QString::fromLatin1("\"") ||
                // allow a starting ' or " to signal a brand new category
                // this bypasses the auto-selection of matching characters
                _currentInput.left(1) == QString::fromLatin1("\'")) {
                _currentInput = _currentInput.right(_currentInput.length()-1);
            }
            currentInfo()->addCategoryInfo( DB::ImageDB::instance()->categoryCollection()->nameForText( _currentCategory ), _currentInput );
            DB::CategoryPtr category =
                DB::ImageDB::instance()->categoryCollection()->categoryForName(_currentCategory);
            category->addItem(_currentInput);
        }
        _currentInput = QString::fromLatin1("");
        updateInfoBox();
        MainWindow::DirtyIndicator::markDirty();
        return; // we've handled it
    } else if ( event->modifiers() == 0 && event->key() >= Qt::Key_0 && event->key() <= Qt::Key_5 ) {
        bool ok;
        short rating = event->text().left(1).toShort(&ok, 10);
        if (ok) {
            currentInfo()->setRating(rating * 2);
            updateInfoBox();
            MainWindow::DirtyIndicator::markDirty();
        }
    } else if (event->modifiers() == 0 ||
               event->modifiers() == Qt::ShiftModifier) {
        // search the category for matches
        QString namefound;
        QString incomingKey = event->text().left(1);

        // start searching for a new category name
        if (incomingKey == QString::fromLatin1("/")) {
            if (_currentInput == QString::fromLatin1("") &&
                _currentCategory == QString::fromLatin1("")) {
                if (_currentInputMode == InACategory) {
                    _currentInputMode = AlwaysStartWithCategory;
                } else {
                    _currentInputMode = InACategory;
                }
            } else {
                // reset the category to search through
                _currentInput = QString::fromLatin1("");
                _currentCategory = QString::fromLatin1("");
            }

        // use an assigned key or map to a given key for future reference
        } else if (_currentInput == QString::fromLatin1("") &&
                   // can map to function keys
                   event->key() >= Qt::Key_F1 &&
                   event->key() <= Qt::Key_F35) {

            // we have a request to assign a macro key or use one
            Qt::Key key = (Qt::Key) event->key();
            if (_inputMacros->contains(key)) {
                // Use the requested toggle
                if ( event->modifiers() == Qt::ShiftModifier ) {
                    if ( currentInfo()->hasCategoryInfo( (*_inputMacros)[key].first, (*_inputMacros)[key].second ) ) {
                        currentInfo()->removeCategoryInfo( (*_inputMacros)[key].first, (*_inputMacros)[key].second );
                    }
                } else {
                    currentInfo()->addCategoryInfo( (*_inputMacros)[key].first, (*_inputMacros)[key].second );
                }
            } else {
                (*_inputMacros)[key] = qMakePair(_lastCategory, _lastFound);
            }
            updateInfoBox();
            MainWindow::DirtyIndicator::markDirty();
            // handled it
            return;
        } else if (_currentCategory == QString::fromLatin1("")) {
            // still searching for a category to lock to
            _currentInput += incomingKey;
            QStringList categorynames = DB::ImageDB::instance()->categoryCollection()->categoryTexts();
            if (find_tag_in_list(categorynames, namefound) == 1) {
                // yay, we have exactly one!
                _currentCategory = namefound;
                _currentInput = QString::fromLatin1("");
                _currentInputList = QString::fromLatin1("");
            }
        } else {
            _currentInput += incomingKey;

            DB::CategoryPtr category =
                DB::ImageDB::instance()->categoryCollection()->categoryForName( DB::ImageDB::instance()->categoryCollection()->nameForText( _currentCategory ) );
            QStringList items = category->items();
            if (find_tag_in_list(items, namefound) == 1) {
                // yay, we have exactly one!
                if ( currentInfo()->hasCategoryInfo( category->name(), namefound ) )
                    currentInfo()->removeCategoryInfo( category->name(), namefound );
                else
                    currentInfo()->addCategoryInfo( category->name(), namefound );

                _lastFound = namefound;
                _lastCategory = _currentCategory;
                _currentInput = QString::fromLatin1("");
                _currentInputList = QString::fromLatin1("");
                if (_currentInputMode == AlwaysStartWithCategory)
                    _currentCategory = QString::fromLatin1("");
            }
        }

        updateInfoBox();
        MainWindow::DirtyIndicator::markDirty();
    }
    QWidget::keyPressEvent( event );
    return;
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
    _exifViewer = new Exif::InfoDialog( currentInfo()->fileName(), this );
    _exifViewer->show();
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

void Viewer::ViewerWidget::makeThumbnailImage()
{
    VideoShooter::go(currentInfo(), this);
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

    _makeThumbnailImage = _actions->addAction( QString::fromLatin1("make-thumbnail-image"), this, SLOT(makeThumbnailImage()));
    _makeThumbnailImage->setShortcut(Qt::ControlModifier + Qt::Key_S);
    _makeThumbnailImage->setText( tr("Use current frame in thumbnail view") );
    _popup->addAction(_makeThumbnailImage);
    _videoActions.append(_makeThumbnailImage);

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

void Viewer::ViewerWidget::createFilterMenu()
{
    _filterMenu = new QMenu( _popup );
    _filterMenu->setTitle( i18n("Filters") );

    _filterNone = _actions->addAction( QString::fromLatin1("filter-empty"), this, SLOT( filterNone() ) );
    _filterNone->setText( i18n("Remove All Filters") );
    _filterMenu->addAction( _filterNone );

    _filterBW = _actions->addAction( QString::fromLatin1("filter-bw"), this, SLOT( filterBW() ) );
    _filterBW->setText( i18n("Apply Grayscale Filter") );
    _filterBW->setCheckable( true );
    _filterMenu->addAction( _filterBW );

    _filterContrastStretch = _actions->addAction( QString::fromLatin1("filter-cs"), this, SLOT( filterContrastStretch() ) );
    _filterContrastStretch->setText( i18n("Apply Contrast Stretching Filter") );
    _filterContrastStretch->setCheckable( true );
    _filterMenu->addAction( _filterContrastStretch );

    _filterHistogramEqualization = _actions->addAction( QString::fromLatin1("filter-he"), this, SLOT( filterHistogramEqualization() ) );
    _filterHistogramEqualization->setText( i18n("Apply Histogram Equalization Filter") );
    _filterHistogramEqualization->setCheckable( true );
    _filterMenu->addAction( _filterHistogramEqualization );

    _filterMono = _actions->addAction( QString::fromLatin1("filter-mono"), this, SLOT( filterMono() ) );
    _filterMono->setText( i18n("Apply Monochrome Filter") );
    _filterMono->setCheckable( true );
    _filterMenu->addAction( _filterMono );

    _popup->addMenu( _filterMenu );
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

void Viewer::ViewerWidget::stopPlayback()
{
    _videoDisplay->stop();
}

void Viewer::ViewerWidget::invalidateThumbnail() const
{
    ImageManager::ThumbnailCache::instance()->removeThumbnail( currentInfo()->fileName() );
}

#include "ViewerWidget.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
