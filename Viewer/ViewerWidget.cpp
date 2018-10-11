/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDesktopWidget>
#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>
#include <qglobal.h>
#include <QKeyEvent>
#include <QList>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QTimeLine>
#include <QTimer>
#include <QWheelEvent>

#include <KActionCollection>
#include <KIconLoader>
#include <KIO/CopyJob>
#include <KLocalizedString>

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <Exif/InfoDialog.h>
#include <ImageManager/ThumbnailCache.h>
#include <MainWindow/CategoryImagePopup.h>
#include <MainWindow/DeleteDialog.h>
#include <MainWindow/DirtyIndicator.h>
#include <MainWindow/ExternalPopup.h>
#include <MainWindow/Window.h>
#include <Utilities/Util.h>
#include <Utilities/DescriptionUtil.h>

#include "CategoryImageConfig.h"
#include "ImageDisplay.h"
#include "InfoBox.h"
#include "SpeedDisplay.h"
#include "TaggedArea.h"
#include "TextDisplay.h"
#include "VideoDisplay.h"
#include "VideoShooter.h"
#include "VisibleOptionsMenu.h"

Viewer::ViewerWidget* Viewer::ViewerWidget::s_latest = nullptr;

Viewer::ViewerWidget* Viewer::ViewerWidget::latest()
{
    return s_latest;
}


// Notice the parent is zero to allow other windows to come on top of it.
Viewer::ViewerWidget::ViewerWidget( UsageType type, QMap<Qt::Key, QPair<QString,QString> > *macroStore )
    :QStackedWidget( nullptr )
    , m_current(0), m_popup(nullptr), m_showingFullScreen( false ), m_forward( true )
    , m_isRunningSlideShow( false ), m_videoPlayerStoppedManually(false), m_type(type)
    , m_currentCategory(DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory)->name())
    , m_inputMacros(macroStore),  m_myInputMacros(nullptr)
{
    if ( type == ViewerWindow ) {
        setWindowFlags( Qt::Window );
        setAttribute( Qt::WA_DeleteOnClose );
        s_latest = this;
    }

    if (! m_inputMacros) {
        m_myInputMacros = m_inputMacros =
            new QMap<Qt::Key, QPair<QString,QString> >;
    }

    m_screenSaverCookie = -1;
    m_currentInputMode = InACategory;

    m_display = m_imageDisplay = new ImageDisplay( this );
    addWidget( m_imageDisplay );

    m_textDisplay = new TextDisplay( this );
    addWidget( m_textDisplay );

    createVideoViewer();

    connect(m_imageDisplay, &ImageDisplay::possibleChange, this, &ViewerWidget::updateCategoryConfig);
    connect(m_imageDisplay, &ImageDisplay::imageReady, this, &ViewerWidget::updateInfoBox);
    connect(m_imageDisplay, &ImageDisplay::setCaptionInfo, this, &ViewerWidget::setCaptionWithDetail);
    connect(m_imageDisplay, &ImageDisplay::viewGeometryChanged, this, &ViewerWidget::remapAreas);

    // This must not be added to the layout, as it is standing on top of
    // the ImageDisplay
    m_infoBox = new InfoBox( this );
    m_infoBox->hide();

    setupContextMenu();

    m_slideShowTimer = new QTimer( this );
    m_slideShowTimer->setSingleShot( true );
    m_slideShowPause = Settings::SettingsData::instance()->slideShowInterval() * 1000;
    connect(m_slideShowTimer, &QTimer::timeout, this, &ViewerWidget::slotSlideShowNextFromTimer);
    m_speedDisplay = new SpeedDisplay( this );
    m_speedDisplay->hide();

    setFocusPolicy( Qt::StrongFocus );

    QTimer::singleShot( 2000, this, SLOT(test()) );
}

void Viewer::ViewerWidget::setupContextMenu()
{
    m_popup = new QMenu( this );
    m_actions = new KActionCollection( this );

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

    QAction * action = m_actions->addAction( QString::fromLatin1("viewer-edit-image-properties"), this, SLOT(editImage()) );
    action->setText( i18nc("@action:inmenu","Annotate...") );
    action->setShortcut( Qt::CTRL+Qt::Key_1 );
    m_popup->addAction( action );

    m_setStackHead = m_actions->addAction( QString::fromLatin1("viewer-set-stack-head"), this, SLOT(slotSetStackHead()) );
    m_setStackHead->setText( i18nc("@action:inmenu","Set as First Image in Stack") );
    m_setStackHead->setShortcut( Qt::CTRL+Qt::Key_4 );
    m_popup->addAction( m_setStackHead );

    m_showExifViewer = m_actions->addAction( QString::fromLatin1("viewer-show-exif-viewer"), this, SLOT(showExifViewer()) );
    m_showExifViewer->setText( i18nc("@action:inmenu","Show Exif Viewer") );
    m_popup->addAction( m_showExifViewer );

    m_copyTo = m_actions->addAction( QString::fromLatin1("viewer-copy-to"), this, SLOT(copyTo()) );
    m_copyTo->setText( i18nc("@action:inmenu","Copy Image to...") );
    m_copyTo->setShortcut( Qt::Key_F7 );
    m_popup->addAction( m_copyTo );

    if ( m_type == ViewerWindow ) {
        action = m_actions->addAction( QString::fromLatin1("viewer-close"), this, SLOT(close()) );
        action->setText( i18nc("@action:inmenu","Close") );
        action->setShortcut( Qt::Key_Escape );
    }

    m_popup->addAction( action );
    m_actions->readSettings();

    Q_FOREACH( QAction* action, m_actions->actions() ) {
      action->setShortcutContext(Qt::WindowShortcut);
      addAction(action);
    }
}

void Viewer::ViewerWidget::createShowContextMenu()
{
    VisibleOptionsMenu* menu = new VisibleOptionsMenu( this, m_actions );
    connect(menu, &VisibleOptionsMenu::visibleOptionsChanged, this, &ViewerWidget::updateInfoBox);
    m_popup->addMenu( menu );
}

void Viewer::ViewerWidget::createWallPaperMenu()
{
    // Setting wallpaper has still not yet been ported to KPA4
#ifndef DOES_STILL_NOT_WORK_IN_KPA4
    m_wallpaperMenu = new QMenu( m_popup );
    m_wallpaperMenu->setTitle( i18nc("@title:inmenu","Set as Wallpaper") );

    QAction * action = m_actions->addAction( QString::fromLatin1("viewer-centered"), this, SLOT(slotSetWallpaperC()) );
    action->setText( i18nc("@action:inmenu","Centered") );
    m_wallpaperMenu->addAction(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-tiled"), this, SLOT(slotSetWallpaperT()) );
    action->setText( i18nc("@action:inmenu","Tiled") );
    m_wallpaperMenu->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-center-tiled"), this, SLOT(slotSetWallpaperCT()) );
    action->setText( i18nc("@action:inmenu","Center Tiled") );
    m_wallpaperMenu->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-centered-maxspect"), this, SLOT(slotSetWallpaperCM()) );
    action->setText( i18nc("@action:inmenu","Centered Maxpect") );
    m_wallpaperMenu->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-tiled-maxpect"), this, SLOT(slotSetWallpaperTM()) );
    action->setText( i18nc("@action:inmenu","Tiled Maxpect") );
    m_wallpaperMenu->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-scaled"), this, SLOT(slotSetWallpaperS()) );
    action->setText( i18nc("@action:inmenu","Scaled") );
    m_wallpaperMenu->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-centered-auto-fit"), this, SLOT(slotSetWallpaperCAF()) );
    action->setText( i18nc("@action:inmenu","Centered Auto Fit") );
    m_wallpaperMenu->addAction( action );

    m_popup->addMenu( m_wallpaperMenu );
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
    m_externalPopup = new MainWindow::ExternalPopup( m_popup );
    m_popup->addMenu( m_externalPopup );
    connect(m_externalPopup, &MainWindow::ExternalPopup::aboutToShow, this, &ViewerWidget::populateExternalPopup);
}

void Viewer::ViewerWidget::createRotateMenu()
{
    m_rotateMenu = new QMenu( m_popup );
    m_rotateMenu->setTitle( i18nc("@title:inmenu","Rotate") );

    QAction * action = m_actions->addAction( QString::fromLatin1("viewer-rotate90"), this, SLOT(rotate90()) );
    action->setText( i18nc("@action:inmenu","Rotate clockwise") );
    action->setShortcut( Qt::Key_9 );
    m_rotateMenu->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-rotate180"), this, SLOT(rotate180()) );
    action->setText( i18nc("@action:inmenu","Flip Over") );
    action->setShortcut( Qt::Key_8 );
    m_rotateMenu->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-rotare270"), this, SLOT(rotate270()) );
    //                                                            ^ this is a typo, isn't it?!
    action->setText( i18nc("@action:inmenu","Rotate counterclockwise") );
    action->setShortcut( Qt::Key_7 );
    m_rotateMenu->addAction( action );

    m_popup->addMenu( m_rotateMenu );
}

void Viewer::ViewerWidget::createSkipMenu()
{
    QMenu *popup = new QMenu( m_popup );
    popup->setTitle( i18nc("@title:inmenu As in 'skip 2 images'","Skip") );

    QAction * action = m_actions->addAction( QString::fromLatin1("viewer-home"), this, SLOT(showFirst()) );
    action->setText( i18nc("@action:inmenu Go to first image","First") );
    action->setShortcut( Qt::Key_Home );
    popup->addAction( action );
    m_backwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-end"), this, SLOT(showLast()) );
    action->setText( i18nc("@action:inmenu Go to last image","Last") );
    action->setShortcut( Qt::Key_End );
    popup->addAction( action );
    m_forwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-next"), this, SLOT(showNext()) );
    action->setText( i18nc("@action:inmenu","Show Next") );
    action->setShortcuts(QList<QKeySequence>() << Qt::Key_PageDown << Qt::Key_Space);
    popup->addAction( action );
    m_forwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-next-10"), this, SLOT(showNext10()) );
    action->setText( i18nc("@action:inmenu","Skip 10 Forward") );
    action->setShortcut( Qt::CTRL+Qt::Key_PageDown );
    popup->addAction( action );
    m_forwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-next-100"), this, SLOT(showNext100()) );
    action->setText( i18nc("@action:inmenu","Skip 100 Forward") );
    action->setShortcut( Qt::SHIFT+Qt::Key_PageDown );
    popup->addAction( action );
    m_forwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-next-1000"), this, SLOT(showNext1000()) );
    action->setText( i18nc("@action:inmenu","Skip 1000 Forward") );
    action->setShortcut( Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown );
    popup->addAction( action );
    m_forwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-prev"), this, SLOT(showPrev()) );
    action->setText( i18nc("@action:inmenu","Show Previous") );
    action->setShortcuts(QList<QKeySequence>() << Qt::Key_PageUp << Qt::Key_Backspace);
    popup->addAction( action );
    m_backwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-prev-10"), this, SLOT(showPrev10()) );
    action->setText( i18nc("@action:inmenu","Skip 10 Backward") );
    action->setShortcut( Qt::CTRL+Qt::Key_PageUp );
    popup->addAction( action );
    m_backwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-prev-100"), this, SLOT(showPrev100()) );
    action->setText( i18nc("@action:inmenu","Skip 100 Backward") );
    action->setShortcut( Qt::SHIFT+Qt::Key_PageUp );
    popup->addAction( action );
    m_backwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-prev-1000"), this, SLOT(showPrev1000()) );
    action->setText( i18nc("@action:inmenu","Skip 1000 Backward") );
    action->setShortcut( Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp );
    popup->addAction( action );
    m_backwardActions.append(action);

    action = m_actions->addAction( QString::fromLatin1("viewer-delete-current"), this, SLOT(deleteCurrent()) );
    action->setText( i18nc("@action:inmenu","Delete Image") );
    action->setShortcut( Qt::CTRL + Qt::Key_Delete );
    popup->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-remove-current"), this, SLOT(removeCurrent()) );
    action->setText( i18nc("@action:inmenu","Remove Image from Display List") );
    action->setShortcut( Qt::Key_Delete );
    popup->addAction( action );

    m_popup->addMenu( popup );
}

void Viewer::ViewerWidget::createZoomMenu()
{
    QMenu *popup = new QMenu( m_popup );
    popup->setTitle( i18nc("@action:inmenu","Zoom") );

    // PENDING(blackie) Only for image display?
    QAction * action = m_actions->addAction( QString::fromLatin1("viewer-zoom-in"), this, SLOT(zoomIn()) );
    action->setText( i18nc("@action:inmenu","Zoom In") );
    action->setShortcut( Qt::Key_Plus );
    popup->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-zoom-out"), this, SLOT(zoomOut()) );
    action->setText( i18nc("@action:inmenu","Zoom Out") );
    action->setShortcut( Qt::Key_Minus );
    popup->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-zoom-full"), this, SLOT(zoomFull()) );
    action->setText( i18nc("@action:inmenu","Full View") );
    action->setShortcut( Qt::Key_Period );
    popup->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-zoom-pixel"), this, SLOT(zoomPixelForPixel()) );
    action->setText( i18nc("@action:inmenu","Pixel for Pixel View") );
    action->setShortcut( Qt::Key_Equal );
    popup->addAction( action );

    action = m_actions->addAction( QString::fromLatin1("viewer-toggle-fullscreen"), this, SLOT(toggleFullScreen()) );
    action->setText( i18nc("@action:inmenu","Toggle Full Screen") );
    action->setShortcuts(QList<QKeySequence>() << Qt::Key_F11 << Qt::Key_Return);
    popup->addAction( action );

    m_popup->addMenu( popup );
}


void Viewer::ViewerWidget::createSlideShowMenu()
{
    QMenu *popup = new QMenu( m_popup );
    popup->setTitle( i18nc("@title:inmenu","Slideshow") );

    m_startStopSlideShow = m_actions->addAction( QString::fromLatin1("viewer-start-stop-slideshow"), this, SLOT(slotStartStopSlideShow()) );
    m_startStopSlideShow->setText( i18nc("@action:inmenu","Run Slideshow") );
    m_startStopSlideShow->setShortcut( Qt::CTRL+Qt::Key_R );
    popup->addAction( m_startStopSlideShow );

    m_slideShowRunFaster = m_actions->addAction( QString::fromLatin1("viewer-run-faster"), this, SLOT(slotSlideShowFaster()) );
    m_slideShowRunFaster->setText( i18nc("@action:inmenu","Run Faster") );
    m_slideShowRunFaster->setShortcut( Qt::CTRL + Qt::Key_Plus ); // if you change this, please update the info in Viewer::SpeedDisplay
    popup->addAction( m_slideShowRunFaster );

    m_slideShowRunSlower = m_actions->addAction( QString::fromLatin1("viewer-run-slower"), this, SLOT(slotSlideShowSlower()) );
    m_slideShowRunSlower->setText( i18nc("@action:inmenu","Run Slower") );
    m_slideShowRunSlower->setShortcut( Qt::CTRL+Qt::Key_Minus ); // if you change this, please update the info in Viewer::SpeedDisplay
    popup->addAction( m_slideShowRunSlower );

    m_popup->addMenu( popup );
}


void Viewer::ViewerWidget::load( const DB::FileNameList& list, int index )
{
    m_list = list;
    m_imageDisplay->setImageList( list );
    m_current = index;
    load();

    bool on = ( list.count() > 1 );
    m_startStopSlideShow->setEnabled(on);
    m_slideShowRunFaster->setEnabled(on);
    m_slideShowRunSlower->setEnabled(on);
}

void Viewer::ViewerWidget::load()
{
    const bool isReadable = QFileInfo( currentInfo()->fileName().absolute() ).isReadable();
    const bool isVideo = isReadable && Utilities::isVideo( currentInfo()->fileName() );

    if ( isReadable ) {
        if ( isVideo ) {
            m_display = m_videoDisplay;
        }
        else
            m_display = m_imageDisplay;
    } else {
        m_display = m_textDisplay;
        m_textDisplay->setText( i18n("File not available") );
        updateInfoBox();
    }

    setCurrentWidget( m_display );
    m_infoBox->raise();

    m_rotateMenu->setEnabled( !isVideo );
    m_wallpaperMenu->setEnabled( !isVideo );
    m_categoryImagePopup->setEnabled( !isVideo );
    m_filterMenu->setEnabled( !isVideo );
    m_showExifViewer->setEnabled( !isVideo );
    if ( m_exifViewer )
        m_exifViewer->setImage( currentInfo()->fileName() );

    Q_FOREACH( QAction* videoAction, m_videoActions ) {
        videoAction->setVisible( isVideo );
    }

    emit soughtTo( m_list[ m_current ]);

    bool ok = m_display->setImage( currentInfo(), m_forward );
    if ( !ok ) {
        close( false );
        return;
    }

    setCaptionWithDetail( QString() );

    // PENDING(blackie) This needs to be improved, so that it shows the actions only if there are that many images to jump.
    for( QList<QAction *>::const_iterator it = m_forwardActions.constBegin(); it != m_forwardActions.constEnd(); ++it )
        (*it)->setEnabled( m_current +1 < (int) m_list.count() );
    for( QList<QAction *>::const_iterator it = m_backwardActions.constBegin(); it != m_backwardActions.constEnd(); ++it )
        (*it)->setEnabled( m_current > 0 );

    m_setStackHead->setEnabled( currentInfo()->isStacked() );

    if ( isVideo )
        updateCategoryConfig();

    if ( m_isRunningSlideShow )
        m_slideShowTimer->start( m_slideShowPause );

    if ( m_display == m_textDisplay )
        updateInfoBox();

    // Add all tagged areas
    addTaggedAreas();
}

void Viewer::ViewerWidget::setCaptionWithDetail( const QString& detail ) {
    setWindowTitle( i18nc("@title:window %1 is the filename, %2 its detail info", "%1 %2",
                          currentInfo()->fileName().absolute(),
                          detail ) );
}

void Viewer::ViewerWidget::contextMenuEvent( QContextMenuEvent * e )
{
    if ( m_videoDisplay ) {
        if ( m_videoDisplay->isPaused() )
            m_playPause->setText(i18nc("@action:inmenu Start video playback","Play"));
        else
            m_playPause->setText(i18nc("@action:inmenu Pause video playback","Pause"));

        m_stop->setEnabled( m_videoDisplay->isPlaying() );
    }

    m_popup->exec( e->globalPos() );
    e->setAccepted(true);
}

void Viewer::ViewerWidget::showNextN(int n)
{
    filterNone();
    if ( m_display == m_videoDisplay ) {
        m_videoPlayerStoppedManually = true;
        m_videoDisplay->stop();
    }

    if ( m_current + n < (int) m_list.count() )  {
        m_current += n;
        if (m_current >= (int) m_list.count())
            m_current = (int) m_list.count() - 1;
        m_forward = true;
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
    const DB::FileName fileName = m_list[m_current];

    if ( action == RemoveImageFromDatabase )
        m_removed.append(fileName);
    m_list.removeAll(fileName);
    if ( m_list.isEmpty() )
        close();
    if ( m_current == m_list.count() )
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
    if ( m_display == m_videoDisplay )
        m_videoDisplay->stop();

    if ( m_current > 0  )  {
        m_current -= n;
    if (m_current < 0)
      m_current = 0;
        m_forward = false;
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
    MainWindow::DirtyIndicator::markDirty();
    emit imageRotated(m_list[ m_current]);
}

void Viewer::ViewerWidget::rotate180()
{
    currentInfo()->rotate( 180 );
    load();
    invalidateThumbnail();
    MainWindow::DirtyIndicator::markDirty();
    emit imageRotated(m_list[ m_current]);
}

void Viewer::ViewerWidget::rotate270()
{
    currentInfo()->rotate( 270 );
    load();
    invalidateThumbnail();
    MainWindow::DirtyIndicator::markDirty();
    emit imageRotated(m_list[ m_current]);
}

void Viewer::ViewerWidget::showFirst()
{
    showPrevN(m_list.count());
}

void Viewer::ViewerWidget::showLast()
{
    showNextN(m_list.count());
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
    if ( !m_removed.isEmpty() ) {
        MainWindow::DeleteDialog dialog( this );
        dialog.exec( m_removed );
    }

    m_slideShowTimer->stop();
    m_isRunningSlideShow = false;
    return QWidget::close();
    if ( alsoDelete )
        deleteLater();
}

DB::ImageInfoPtr Viewer::ViewerWidget::currentInfo() const
{
    return DB::ImageDB::instance()->info(m_list[ m_current]); // PENDING(blackie) can we postpone this lookup?
}

void Viewer::ViewerWidget::infoBoxMove()
{
    QPoint p = mapFromGlobal( QCursor::pos() );
    Settings::Position oldPos = Settings::SettingsData::instance()->infoBoxPosition();
    Settings::Position pos = oldPos;
    int x = m_display->mapFromParent( p ).x();
    int y = m_display->mapFromParent( p ).y();
    int w = m_display->width();
    int h = m_display->height();

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
    m_infoBox->setSize();
    Settings::Position pos = Settings::SettingsData::instance()->infoBoxPosition();

    int lx = m_display->pos().x();
    int ly = m_display->pos().y();
    int lw = m_display->width();
    int lh = m_display->height();

    int bw = m_infoBox->width();
    int bh = m_infoBox->height();

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

    m_infoBox->move(bx,by);
}

void Viewer::ViewerWidget::resizeEvent( QResizeEvent* e )
{
    moveInfoBox();
    QWidget::resizeEvent( e );
}

void Viewer::ViewerWidget::updateInfoBox()
{
    QString tokensCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory)->name();
    if ( currentInfo() || !m_currentInput.isEmpty() ||
         (!m_currentCategory.isEmpty() && m_currentCategory != tokensCategory)) {
        QMap<int, QPair<QString,QString> > map;
        QString text = Utilities::createInfoText( currentInfo(), &map );
        QString selecttext = QString::fromLatin1("");
        if (m_currentCategory.isEmpty()) {
            selecttext = i18nc("Basically 'enter a category name'","<b>Setting Category: </b>") + m_currentInput;
            if (m_currentInputList.length() > 0) {
                selecttext += QString::fromLatin1("{") + m_currentInputList +
                    QString::fromLatin1("}");
            }
        } else if ( ( !m_currentInput.isEmpty() &&
                   m_currentCategory != tokensCategory)) {
            selecttext = i18nc("Basically 'enter a tag name'","<b>Assigning: </b>") + m_currentCategory +
                QString::fromLatin1("/")  + m_currentInput;
            if (m_currentInputList.length() > 0) {
                selecttext += QString::fromLatin1("{") + m_currentInputList +
                    QString::fromLatin1("}");
            }
        } else if ( !m_currentInput.isEmpty() &&
                   m_currentCategory == tokensCategory) {
            m_currentInput = QString::fromLatin1("");
        }
        if (!selecttext.isEmpty())
            text = selecttext + QString::fromLatin1("<br />") + text;
        if ( Settings::SettingsData::instance()->showInfoBox() && !text.isNull() && ( m_type != InlineViewer ) ) {
            m_infoBox->setInfo( text, map );
            m_infoBox->show();

        }
        else
            m_infoBox->hide();

        moveInfoBox();
    }
}

Viewer::ViewerWidget::~ViewerWidget()
{
    inhibitScreenSaver(false);

    if ( s_latest == this )
        s_latest = nullptr;

    if ( m_myInputMacros )
        delete m_myInputMacros;
}


void Viewer::ViewerWidget::toggleFullScreen()
{
    setShowFullScreen( !m_showingFullScreen );
}

void Viewer::ViewerWidget::slotStartStopSlideShow()
{
    bool wasRunningSlideShow = m_isRunningSlideShow;
    m_isRunningSlideShow = !m_isRunningSlideShow && m_list.count() != 1;

    if ( wasRunningSlideShow ) {
        m_startStopSlideShow->setText( i18nc("@action:inmenu","Run Slideshow") );
        m_slideShowTimer->stop();
        if ( m_list.count() != 1 )
            m_speedDisplay->end();
        inhibitScreenSaver(false);
    }
    else {
        m_startStopSlideShow->setText( i18nc("@action:inmenu","Stop Slideshow") );
        if ( currentInfo()->mediaType() != DB::Video )
            m_slideShowTimer->start( m_slideShowPause );
        m_speedDisplay->start();
        inhibitScreenSaver(true);
    }
}

void Viewer::ViewerWidget::slotSlideShowNextFromTimer()
{
    // Load the next images.
    QTime timer;
    timer.start();
    if ( m_display == m_imageDisplay )
        slotSlideShowNext();

    // ensure that there is a few milliseconds pause, so that an end slideshow keypress
    // can get through immediately, we don't want it to queue up behind a bunch of timer events,
    // which loaded a number of new images before the slideshow stops
    int ms = qMax( 200, m_slideShowPause - timer.elapsed() );
    m_slideShowTimer->start( ms );
}

void Viewer::ViewerWidget::slotSlideShowNext()
{
    m_forward = true;
    if ( m_current +1 < (int) m_list.count() )
        m_current++;
    else
        m_current = 0;

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
    if ( m_list.count() == 1 )
        return;

    m_slideShowPause += delta;
    m_slideShowPause = qMax( m_slideShowPause, 500 );
    m_speedDisplay->display( m_slideShowPause );
    if (m_slideShowTimer->isActive() )
        m_slideShowTimer->start( m_slideShowPause );
}


void Viewer::ViewerWidget::editImage()
{
    DB::ImageInfoList list;
    list.append( currentInfo() );
    MainWindow::Window::configureImages( list, true );
}

void Viewer::ViewerWidget::filterNone()
{
    if ( m_display == m_imageDisplay ) {
        m_imageDisplay->filterNone();
        m_filterMono->setChecked( false );
        m_filterBW->setChecked( false );
        m_filterContrastStretch->setChecked( false );
        m_filterHistogramEqualization->setChecked( false );
    }
}

void Viewer::ViewerWidget::filterSelected()
{
    // The filters that drop bit depth below 32 should be the last ones
    // so that filters requiring more bit depth are processed first
    if ( m_display == m_imageDisplay ) {
        m_imageDisplay->filterNone();
        if (m_filterBW->isChecked())
            m_imageDisplay->filterBW();
        if (m_filterContrastStretch->isChecked())
            m_imageDisplay->filterContrastStretch();
        if (m_filterHistogramEqualization->isChecked())
            m_imageDisplay->filterHistogramEqualization();
        if (m_filterMono->isChecked())
            m_imageDisplay->filterMono();
    }
}

void Viewer::ViewerWidget::filterBW()
{
    if ( m_display == m_imageDisplay ) {
        if ( m_filterBW->isChecked() )
            m_filterBW->setChecked( m_imageDisplay->filterBW());
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::filterContrastStretch()
{
    if ( m_display == m_imageDisplay ) {
        if (m_filterContrastStretch->isChecked())
            m_filterContrastStretch->setChecked( m_imageDisplay->filterContrastStretch() );
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::filterHistogramEqualization()
{
    if ( m_display == m_imageDisplay ) {
        if ( m_filterHistogramEqualization->isChecked() )
            m_filterHistogramEqualization->setChecked( m_imageDisplay->filterHistogramEqualization() );
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::filterMono()
{
    if ( m_display == m_imageDisplay ) {
        if ( m_filterMono->isChecked() )
            m_filterMono->setChecked( m_imageDisplay->filterMono() );
        else
            filterSelected();
    }
}

void Viewer::ViewerWidget::slotSetStackHead()
{
    MainWindow::Window::theMainWindow()->setStackHead(m_list[ m_current ]);
}

bool Viewer::ViewerWidget::showingFullScreen() const
{
    return m_showingFullScreen;
}

void Viewer::ViewerWidget::setShowFullScreen( bool on )
{
    if ( on ) {
        setWindowState( windowState() | Qt::WindowFullScreen ); // set
        moveInfoBox();
    }
    else {
        // We need to size the image when going out of full screen, in case we started directly in full screen
        //
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset
        resize( Settings::SettingsData::instance()->viewerSize() );
    }
    m_showingFullScreen = on;
}

void Viewer::ViewerWidget::updateCategoryConfig()
{
    if ( !CategoryImageConfig::instance()->isVisible() )
        return;

    CategoryImageConfig::instance()->setCurrentImage( m_imageDisplay->currentViewAsThumbnail(), currentInfo() );
}


void Viewer::ViewerWidget::populateExternalPopup()
{
    m_externalPopup->populate( currentInfo(), m_list );
}

void Viewer::ViewerWidget::populateCategoryImagePopup()
{
    m_categoryImagePopup->populate( m_imageDisplay->currentViewAsThumbnail(), m_list[m_current] );
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
    if ( slideShow != m_isRunningSlideShow) {
        // The info dialog will show up at the wrong place if we call this function directly
        // don't ask me why -  4 Sep. 2004 15:13 -- Jesper K. Pedersen
        QTimer::singleShot(0, this, SLOT(slotStartStopSlideShow()) );
    }
}

KActionCollection* Viewer::ViewerWidget::actions()
{
    return m_actions;
}

int Viewer::ViewerWidget::find_tag_in_list(const QStringList &list,
                                           QString &namefound)
{
    int found = 0;
    m_currentInputList = QString::fromLatin1("");
    for( QStringList::ConstIterator listIter = list.constBegin();
         listIter != list.constEnd(); ++listIter ) {
        if (listIter->startsWith(m_currentInput, Qt::CaseInsensitive)) {
            found++;
            if (m_currentInputList.length() > 0)
                m_currentInputList =
                    m_currentInputList + QString::fromLatin1(",");
            m_currentInputList =m_currentInputList +
                listIter->right(listIter->length() - m_currentInput.length());
            if (found > 1 && m_currentInputList.length() > 20) {
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
        m_currentInput.remove( m_currentInput.length()-1, 1 );
        updateInfoBox();
        MainWindow::DirtyIndicator::markDirty();
        m_currentInputList = QString::fromLatin1("");
//     } else if (event->modifier & (Qt::AltModifier | Qt::MetaModifier) &&
//                event->key() == Qt::Key_Enter) {
        return; // we've handled it
    } else if (event->key() == Qt::Key_Comma) {
        // force set the "new" token
        if (!m_currentCategory.isEmpty()) {
            if (m_currentInput.left(1) == QString::fromLatin1("\"") ||
                // allow a starting ' or " to signal a brand new category
                // this bypasses the auto-selection of matching characters
                m_currentInput.left(1) == QString::fromLatin1("\'")) {
                m_currentInput = m_currentInput.right(m_currentInput.length()-1);
            }
            if (m_currentInput.isEmpty())
                return;
            currentInfo()->addCategoryInfo(m_currentCategory, m_currentInput);
            DB::CategoryPtr category =
                DB::ImageDB::instance()->categoryCollection()->categoryForName(m_currentCategory);
            category->addItem(m_currentInput);
        }
        m_currentInput = QString::fromLatin1("");
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
            if (m_currentInput.isEmpty() &&
                m_currentCategory.isEmpty()) {
                if (m_currentInputMode == InACategory) {
                    m_currentInputMode = AlwaysStartWithCategory;
                } else {
                    m_currentInputMode = InACategory;
                }
            } else {
                // reset the category to search through
                m_currentInput = QString::fromLatin1("");
                m_currentCategory = QString::fromLatin1("");
            }

        // use an assigned key or map to a given key for future reference
        } else if (m_currentInput.isEmpty() &&
                   // can map to function keys
                   event->key() >= Qt::Key_F1 &&
                   event->key() <= Qt::Key_F35) {

            // we have a request to assign a macro key or use one
            Qt::Key key = (Qt::Key) event->key();
            if (m_inputMacros->contains(key)) {
                // Use the requested toggle
                if ( event->modifiers() == Qt::ShiftModifier ) {
                    if ( currentInfo()->hasCategoryInfo( (*m_inputMacros)[key].first, (*m_inputMacros)[key].second ) ) {
                        currentInfo()->removeCategoryInfo( (*m_inputMacros)[key].first, (*m_inputMacros)[key].second );
                    }
                } else {
                    currentInfo()->addCategoryInfo( (*m_inputMacros)[key].first, (*m_inputMacros)[key].second );
                }
            } else {
                (*m_inputMacros)[key] = qMakePair(m_lastCategory, m_lastFound);
            }
            updateInfoBox();
            MainWindow::DirtyIndicator::markDirty();
            // handled it
            return;
        } else if (m_currentCategory.isEmpty()) {
            // still searching for a category to lock to
            m_currentInput += incomingKey;
            QStringList categorynames = DB::ImageDB::instance()->categoryCollection()->categoryTexts();
            if (find_tag_in_list(categorynames, namefound) == 1) {
                // yay, we have exactly one!
                m_currentCategory = namefound;
                m_currentInput = QString::fromLatin1("");
                m_currentInputList = QString::fromLatin1("");
            }
        } else {
            m_currentInput += incomingKey;

            DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()
                                                              ->categoryForName(m_currentCategory);
            QStringList items = category->items();
            if (find_tag_in_list(items, namefound) == 1) {
                // yay, we have exactly one!
                if ( currentInfo()->hasCategoryInfo( category->name(), namefound ) )
                    currentInfo()->removeCategoryInfo( category->name(), namefound );
                else
                    currentInfo()->addCategoryInfo( category->name(), namefound );

                m_lastFound = namefound;
                m_lastCategory = m_currentCategory;
                m_currentInput = QString::fromLatin1("");
                m_currentInputList = QString::fromLatin1("");
                if (m_currentInputMode == AlwaysStartWithCategory)
                    m_currentCategory = QString::fromLatin1("");
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
    if ( !m_videoPlayerStoppedManually && m_isRunningSlideShow )
        slotSlideShowNext();
    m_videoPlayerStoppedManually=false;
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
    m_exifViewer = new Exif::InfoDialog( currentInfo()->fileName(), this );
    m_exifViewer->show();
}

void Viewer::ViewerWidget::zoomIn()
{
    m_display->zoomIn();
}

void Viewer::ViewerWidget::zoomOut()
{
    m_display->zoomOut();
}

void Viewer::ViewerWidget::zoomFull()
{
    m_display->zoomFull();
}

void Viewer::ViewerWidget::zoomPixelForPixel()
{
    m_display->zoomPixelForPixel();
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
    QMenu* menu = new QMenu(m_popup);
    menu->setTitle(i18nc("@title:inmenu","Seek"));
    m_videoActions.append( m_popup->addMenu( menu ) );

    QList<SeekInfo> list;
    list << SeekInfo( i18nc("@action:inmenu","10 minutes backward"), "seek-10-minute", -600000, QKeySequence(QString::fromLatin1("Ctrl+Left")))
         << SeekInfo( i18nc("@action:inmenu","1 minute backward"), "seek-1-minute", -60000, QKeySequence(QString::fromLatin1( "Shift+Left")))
         << SeekInfo( i18nc("@action:inmenu","10 seconds backward"), "seek-10-second", -10000, QKeySequence(QString::fromLatin1( "Left")))
         << SeekInfo( i18nc("@action:inmenu","1 seconds backward"), "seek-1-second", -1000, QKeySequence(QString::fromLatin1( "Up")))
         << SeekInfo( i18nc("@action:inmenu","100 milliseconds backward"), "seek-100-millisecond", -100, QKeySequence(QString::fromLatin1( "Shift+Up")))
         << SeekInfo( i18nc("@action:inmenu","100 milliseconds forward"), "seek+100-millisecond", 100, QKeySequence(QString::fromLatin1( "Shift+Down")))
         << SeekInfo( i18nc("@action:inmenu","1 seconds forward"), "seek+1-second", 1000, QKeySequence(QString::fromLatin1( "Down")))
         << SeekInfo( i18nc("@action:inmenu","10 seconds forward"), "seek+10-second", 10000, QKeySequence(QString::fromLatin1( "Right")))
         << SeekInfo( i18nc("@action:inmenu","1 minute forward"), "seek+1-minute", 60000, QKeySequence(QString::fromLatin1( "Shift+Right")))
         << SeekInfo( i18nc("@action:inmenu","10 minutes forward"), "seek+10-minute", 600000, QKeySequence(QString::fromLatin1( "Ctrl+Right")));

    int count=0;
    Q_FOREACH( const SeekInfo& info, list ) {
        if ( count++ == 5 ) {
            QAction* sep = new QAction( menu );
            sep->setSeparator(true);
            menu->addAction(sep);
        }

        QAction * seek = m_actions->addAction( QString::fromLatin1(info.name), m_videoDisplay, SLOT(seek()));
        seek->setText(info.title);
        seek->setData(info.value);
        seek->setShortcut( info.key );
        menu->addAction(seek);
    }

    QAction* sep = new QAction(m_popup);
    sep->setSeparator(true);
    m_popup->addAction( sep );
    m_videoActions.append( sep );

    m_stop = m_actions->addAction( QString::fromLatin1("viewer-video-stop"), m_videoDisplay, SLOT(stop()) );
    m_stop->setText( i18nc("@action:inmenu Stop video playback","Stop") );
    m_popup->addAction( m_stop );
    m_videoActions.append(m_stop);


    m_playPause = m_actions->addAction( QString::fromLatin1("viewer-video-pause"), m_videoDisplay, SLOT(playPause()) );
    // text set in contextMenuEvent()
    m_playPause->setShortcut( Qt::Key_P );
    m_popup->addAction( m_playPause );
    m_videoActions.append( m_playPause );

    m_makeThumbnailImage = m_actions->addAction( QString::fromLatin1("make-thumbnail-image"), this, SLOT(makeThumbnailImage()));
    m_makeThumbnailImage->setShortcut(Qt::ControlModifier + Qt::Key_S);
    m_makeThumbnailImage->setText( i18nc("@action:inmenu","Use current frame in thumbnail view") );
    m_popup->addAction(m_makeThumbnailImage);
    m_videoActions.append(m_makeThumbnailImage);

    QAction * restart = m_actions->addAction( QString::fromLatin1("viewer-video-restart"), m_videoDisplay, SLOT(restart()) );
    restart->setText( i18nc("@action:inmenu Restart video playback.","Restart") );
    m_popup->addAction( restart );
    m_videoActions.append( restart );
}

void Viewer::ViewerWidget::createCategoryImageMenu()
{
    m_categoryImagePopup = new MainWindow::CategoryImagePopup( m_popup );
    m_popup->addMenu( m_categoryImagePopup );
    connect(m_categoryImagePopup, &MainWindow::CategoryImagePopup::aboutToShow, this, &ViewerWidget::populateCategoryImagePopup);
}

void Viewer::ViewerWidget::createFilterMenu()
{
    m_filterMenu = new QMenu( m_popup );
    m_filterMenu->setTitle( i18nc("@title:inmenu","Filters") );

    m_filterNone = m_actions->addAction( QString::fromLatin1("filter-empty"), this, SLOT(filterNone()) );
    m_filterNone->setText( i18nc("@action:inmenu","Remove All Filters") );
    m_filterMenu->addAction( m_filterNone );

    m_filterBW = m_actions->addAction( QString::fromLatin1("filter-bw"), this, SLOT(filterBW()) );
    m_filterBW->setText( i18nc("@action:inmenu","Apply Grayscale Filter") );
    m_filterBW->setCheckable( true );
    m_filterMenu->addAction( m_filterBW );

    m_filterContrastStretch = m_actions->addAction( QString::fromLatin1("filter-cs"), this, SLOT(filterContrastStretch()) );
    m_filterContrastStretch->setText( i18nc("@action:inmenu","Apply Contrast Stretching Filter") );
    m_filterContrastStretch->setCheckable( true );
    m_filterMenu->addAction( m_filterContrastStretch );

    m_filterHistogramEqualization = m_actions->addAction( QString::fromLatin1("filter-he"), this, SLOT(filterHistogramEqualization()) );
    m_filterHistogramEqualization->setText( i18nc("@action:inmenu","Apply Histogram Equalization Filter") );
    m_filterHistogramEqualization->setCheckable( true );
    m_filterMenu->addAction( m_filterHistogramEqualization );

    m_filterMono = m_actions->addAction( QString::fromLatin1("filter-mono"), this, SLOT(filterMono()) );
    m_filterMono->setText( i18nc("@action:inmenu","Apply Monochrome Filter") );
    m_filterMono->setCheckable( true );
    m_filterMenu->addAction( m_filterMono );

    m_popup->addMenu( m_filterMenu );
}


void Viewer::ViewerWidget::test()
{
#ifdef TESTING
    QTimeLine* timeline = new QTimeLine;
    timeline->setStartFrame( _infoBox->y() );
    timeline->setEndFrame( height() );
    connect(timeline, &QTimeLine::frameChanged, this, &ViewerWidget::moveInfoBox);
    timeline->start();
#endif // TESTING
}

void Viewer::ViewerWidget::moveInfoBox( int y)
{
    m_infoBox->move( m_infoBox->x(), y );
}

void Viewer::ViewerWidget::createVideoViewer()
{
    m_videoDisplay = new VideoDisplay( this );
    addWidget( m_videoDisplay );
    connect(m_videoDisplay, &VideoDisplay::stopped, this, &ViewerWidget::videoStopped);
}

void Viewer::ViewerWidget::stopPlayback()
{
    m_videoDisplay->stop();
}

void Viewer::ViewerWidget::invalidateThumbnail() const
{
    ImageManager::ThumbnailCache::instance()->removeThumbnail( currentInfo()->fileName() );
}

void Viewer::ViewerWidget::addTaggedAreas()
{
    // Clean all areas we probably already have
    foreach (TaggedArea *area, findChildren<TaggedArea *>()) {
        area->deleteLater();
    }

    QMap<QString, QMap<QString, QRect>> taggedAreas = currentInfo()->taggedAreas();
    QMapIterator<QString, QMap<QString, QRect>> areasInCategory(taggedAreas);
    QString category;
    QString tag;

    while (areasInCategory.hasNext()) {
        areasInCategory.next();
        category = areasInCategory.key();

        QMapIterator<QString, QRect> areaData(areasInCategory.value());
        while (areaData.hasNext()) {
            areaData.next();
            tag = areaData.key();

            // Add a new frame for the area
            TaggedArea *newArea = new TaggedArea(this);
            newArea->setTagInfo(category, category, tag);
            newArea->setActualGeometry(areaData.value());
            newArea->show();

            connect(m_infoBox, &InfoBox::tagHovered, newArea, &TaggedArea::checkShowArea);
            connect(m_infoBox, &InfoBox::noTagHovered, newArea, &TaggedArea::resetViewStyle);
        }
    }

    // Be sure to display the areas, as viewGeometryChanged is not always emitted on load

    QSize imageSize = currentInfo()->size();
    QSize windowSize = this->size();

    // On load, the image is never zoomed, so it's a bit easier ;-)
    double scaleWidth = double(imageSize.width()) / windowSize.width();
    double scaleHeight = double(imageSize.height()) / windowSize.height();
    int offsetTop = 0;
    int offsetLeft = 0;
    if (scaleWidth > scaleHeight) {
        offsetTop = (windowSize.height() - imageSize.height() / scaleWidth);
    } else {
        offsetLeft = (windowSize.width() - imageSize.width() / scaleHeight);
    }

    remapAreas(
        QSize(windowSize.width() - offsetLeft, windowSize.height() - offsetTop),
        QRect(QPoint(0, 0), QPoint(imageSize.width(), imageSize.height())),
        1
    );
}

void Viewer::ViewerWidget::remapAreas(QSize viewSize, QRect zoomWindow, double sizeRatio)
{
    QSize currentWindowSize = this->size();
    int outerOffsetLeft = (currentWindowSize.width() - viewSize.width()) / 2;
    int outerOffsetTop = (currentWindowSize.height() - viewSize.height()) / 2;

    if (sizeRatio != 1) {
        zoomWindow = QRect(
            QPoint(
                double(zoomWindow.left()) * sizeRatio,
                double(zoomWindow.top()) * sizeRatio
            ),
            QPoint(
                double(zoomWindow.left() + zoomWindow.width()) * sizeRatio,
                double(zoomWindow.top() + zoomWindow.height()) * sizeRatio
            )
        );
    }

    double scaleHeight = double(viewSize.height()) / zoomWindow.height();
    double scaleWidth = double(viewSize.width()) / zoomWindow.width();

    int innerOffsetLeft = -zoomWindow.left() * scaleWidth;
    int innerOffsetTop = -zoomWindow.top() * scaleHeight;

    Q_FOREACH(TaggedArea *area, findChildren<TaggedArea *>()) {
        QRect actualGeometry = area->actualGeometry();
        QRect screenGeometry;

        screenGeometry.setWidth(actualGeometry.width() * scaleWidth);
        screenGeometry.setHeight(actualGeometry.height() * scaleHeight);
        screenGeometry.moveTo(
            actualGeometry.left() * scaleWidth + outerOffsetLeft + innerOffsetLeft,
            actualGeometry.top() * scaleHeight + outerOffsetTop + innerOffsetTop
        );

        area->setGeometry(screenGeometry);
    }
}

void Viewer::ViewerWidget::copyTo()
{
    QUrl src = QUrl::fromLocalFile(currentInfo()->fileName().absolute());
    if (m_lastCopyToTarget.isNull()) {
        // get directory of src file
        m_lastCopyToTarget = QFileInfo(src.path()).path();
    }

    QFileDialog dialog( this );
    dialog.setWindowTitle( i18nc("@title:window", "Copy Image to...") );
    // use directory of src as start-location:
    dialog.setDirectory(m_lastCopyToTarget);
    dialog.selectFile(src.fileName());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setLabelText(QFileDialog::Accept, i18nc("@action:button", "Copy"));

    if (dialog.exec()) {
        QUrl dst = dialog.selectedUrls().first();
        KIO::CopyJob *job = KIO::copy(src, dst);
        connect(job, &KIO::CopyJob::finished, job, &QObject::deleteLater);
        // get directory of dst file
        m_lastCopyToTarget = QFileInfo(dst.path()).path();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
