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

#include "Window.h"
#include "Settings/SettingsDialog.h"
#include <qapplication.h>
#include "ThumbnailView/ThumbnailWidget.h"
#include "ThumbnailView/ThumbnailBuilder.h"
#include "AnnotationDialog/Dialog.h"
#include <qdir.h>
#include <qmessagebox.h>
#include "Viewer/ViewerWidget.h"
#include "WelcomeDialog.h"
#include <qcursor.h>
#include "Utilities/ShowBusyCursor.h"
#include <klocale.h>
#include <qhbox.h>
#include <qwidgetstack.h>
#include "HTMLGenerator/HTMLDialog.h"
#include <kstatusbar.h>
#include "ImageCounter.h"
#include <qtimer.h>
#include <kmessagebox.h>
#include "Settings/SettingsData.h"
#include "Browser/BrowserWidget.h"
#include "DB/ImageDB.h"
#include "Utilities/Util.h"
#include "Utilities/List.h"
#include <kapplication.h>
#include <ktip.h>
#include <kprocess.h>
#include "DeleteDialog.h"
#include <ksimpleconfig.h>
#include <kcmdlineargs.h>
#include <qpopupmenu.h>
#include <kiconloader.h>
#include <kpassdlg.h>
#include <kkeydialog.h>
#include <kdebug.h>
#include "ExternalPopup.h"
#include "DonateDialog.h"
#include <kstdaction.h>
#include "DeleteThumbnailsDialog.h"
#include <kedittoolbar.h>
#include "ImportExport/Export.h"
#include "ImportExport/Import.h"
#include <config.h>
#ifdef HASKIPI
#  include "Plugins/Interface.h"
#  include <libkipi/pluginloader.h>
#  include <libkipi/plugin.h>
#endif
#ifdef HASEXIV2
#  include "Exif/ReReadDialog.h"
#  include "Exif/WriteDialog.h"
#endif
#include "ImageManager/ImageLoader.h"
#include "SplashScreen.h"
#include <qobjectlist.h>
#include <SearchBar.h>
#include "TokenEditor.h"
#include "DB/CategoryCollection.h"
#include <qlayout.h>
#include "DateBar/DateBarWidget.h"
#include "DB/ImageDateCollection.h"
#include "InvalidDateFinder.h"
#include "DB/ImageInfo.h"
#include "Survey/MySurvey.h"
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HASEXIV2
#  include "Exif/Info.h"
#  include "Exif/InfoDialog.h"
#  include "Exif/Database.h"
#endif

#include "FeatureDialog.h"
#include "ImageManager/ImageRequest.h"
#include "ImageManager/Manager.h"

#ifdef SQLDB_SUPPORT
#  include "SQLDB/Database.h"
#  include "SQLDB/ConfigFileHandler.h"
#  include "SQLDB/QueryErrors.h"
#  include <kexidb/kexidb_export.h>
#  include <kexidb/connectiondata.h>
#endif
#include <kprogress.h>
#include <krun.h>
#include "DirtyIndicator.h"
#include "Utilities/ShowBusyCursor.h"
#include <kurldrag.h>
#include <qclipboard.h>
#include <stdexcept>
#include <typeinfo>

MainWindow::Window* MainWindow::Window::_instance = 0;

MainWindow::Window::Window( QWidget* parent, const char* name )
    :KMainWindow( parent,  name ), _annotationDialog(0),
     _deleteDialog( 0 ), _htmlDialog(0), _tokenEditor( 0 )
{
    SplashScreen::instance()->message( i18n("Loading Database") );
    _instance = this;

    bool gotConfigFile = load();
    if ( !gotConfigFile )
        exit(0);
    SplashScreen::instance()->message( i18n("Loading Main Window") );

    // To avoid a race conditions where both the image loader thread creates an instance of
    // Options, and where the main thread crates an instance, we better get it created now.
    Settings::SettingsData::instance();

    QWidget* top = new QWidget( this, "top" );
    QVBoxLayout* lay = new QVBoxLayout( top, 6 );
    setCentralWidget( top );

    _stack = new QWidgetStack( top, "_stack" );
    lay->addWidget( _stack, 1 );

    _dateBar = new DateBar::DateBarWidget( top, "datebar" );
    lay->addWidget( _dateBar );

    QFrame* line = new QFrame( top );
    line->setFrameStyle( QFrame::HLine | QFrame::Plain );
    line->setLineWidth(1);
    lay->addWidget( line );

    _browser = new Browser::BrowserWidget( _stack, "browser" );
    connect( _browser, SIGNAL( showingOverview() ), this, SLOT( showBrowser() ) );
    connect( _browser, SIGNAL( pathChanged( const QString& ) ), this, SLOT( pathChanged( const QString& ) ) );
    connect( _browser, SIGNAL( pathChanged( const QString& ) ), this, SLOT( updateDateBar( const QString& ) ) );
    _thumbnailView = new ThumbnailView::ThumbnailWidget( _stack, "_thumbnailView" );
    connect( _dateBar, SIGNAL( dateSelected( const DB::ImageDate&, bool ) ), _thumbnailView, SLOT( gotoDate( const DB::ImageDate&, bool ) ) );
    connect( _dateBar, SIGNAL( toolTipInfo( const QString& ) ), this, SLOT( showDateBarTip( const QString& ) ) );
    connect( Settings::SettingsData::instance(), SIGNAL( histogramSizeChanged( const QSize& ) ), _dateBar, SLOT( setHistogramBarSize( const QSize& ) ) );


    connect( _dateBar, SIGNAL( dateRangeChange( const DB::ImageDate& ) ),
             this, SLOT( setDateRange( const DB::ImageDate& ) ) );
    connect( _dateBar, SIGNAL( dateRangeCleared() ), this, SLOT( clearDateRange() ) );

    connect( _thumbnailView, SIGNAL( showImage( const QString& ) ), this, SLOT( showImage( const QString& ) ) );
    connect( _thumbnailView, SIGNAL( showSelection() ), this, SLOT( slotView() ) );
    connect( _thumbnailView, SIGNAL( currentDateChanged( const QDateTime& ) ), _dateBar, SLOT( setDate( const QDateTime& ) ) );

    connect( _thumbnailView, SIGNAL( fileNameUnderCursorChanged( const QString& ) ), this, SLOT( slotSetFileName( const QString& ) ) );

    _stack->addWidget( _browser );
    _stack->addWidget( _thumbnailView );
    _stack->raiseWidget( _browser );

    _optionsDialog = 0;
    setupMenuBar();

    // Set up the search tool bar
    SearchBar* bar = new SearchBar( this );

    connect( bar, SIGNAL( textChanged( const QString& ) ), _browser, SLOT( slotLimitToMatch( const QString& ) ) );
    connect( bar, SIGNAL( returnPressed() ), _browser, SLOT( slotInvokeSeleted() ) );
    connect( bar, SIGNAL( scrollLine( int ) ), _browser, SLOT( scrollLine( int ) ) );
    connect( bar, SIGNAL( scrollPage( int ) ), _browser, SLOT( scrollPage( int ) ) );
    connect( _browser, SIGNAL( viewChanged() ), bar, SLOT( reset() ) );
    connect( _browser, SIGNAL( showsContentView( bool ) ), bar, SLOT( setEnabled( bool ) ) );

    // Setting up status bar
    QFont f( statusBar()->font() ); // Avoid flicker in the statusbar when moving over dates from the datebar
    f.setStyleHint( QFont::TypeWriter );
    f.setFamily( QString::fromLatin1( "courier" ) );
    f.setBold( true );
    statusBar()->setFont( f );

    QHBox* indicators = new QHBox( statusBar(), "indicator" );
    _dirtyIndicator = new DirtyIndicator( indicators );

    _lockedIndicator = new QLabel( indicators, "_lockedIndicator" );
    setLocked( Settings::SettingsData::instance()->isLocked(), true );

    statusBar()->addWidget( indicators, 0, true );

    _partial = new ImageCounter( statusBar(), "partial image counter" );
    statusBar()->addWidget( _partial, 0, true );

    ImageCounter* total = new ImageCounter( statusBar(), "total image counter" );
    statusBar()->addWidget( total, 0, true );

    // Misc
    _autoSaveTimer = new QTimer( this );
    connect( _autoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
    startAutoSaveTimer();

    connect( DB::ImageDB::instance(), SIGNAL( totalChanged( uint ) ), total, SLOT( setTotal( uint ) ) );
    connect( DB::ImageDB::instance(), SIGNAL( totalChanged( uint ) ), this, SLOT( updateDateBar() ) );
    connect( DB::ImageDB::instance(), SIGNAL( totalChanged( uint ) ), _browser, SLOT( home() ) );
    connect( _browser, SIGNAL( showingOverview() ), _partial, SLOT( showingOverview() ) );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL( categoryCollectionChanged() ), this, SLOT( slotOptionGroupChanged() ) );
    connect( _thumbnailView, SIGNAL( selectionChanged() ), this, SLOT( slotThumbNailSelectionChanged() ) );

    connect( _dirtyIndicator, SIGNAL( dirty() ), _thumbnailView, SLOT(repaintScreen() ) );

    total->setTotal( DB::ImageDB::instance()->totalCount() );
    statusBar()->message(i18n("Welcome to KPhotoAlbum"), 5000 );

    QTimer::singleShot( 0, this, SLOT( delayedInit() ) );
    slotThumbNailSelectionChanged();
}

MainWindow::Window::~Window()
{
    DB::ImageDB::deleteInstance();
}

void MainWindow::Window::delayedInit()
{
    SplashScreen* splash = SplashScreen::instance();
    setupPluginMenu();

    if ( Settings::SettingsData::instance()->searchForImagesOnStartup() ) {
        splash->message( i18n("Searching for New Files") );
        qApp->processEvents();
        DB::ImageDB::instance()->slotRescan();
    }

    if ( !Settings::SettingsData::instance()->delayLoadingPlugins() ) {
        splash->message( i18n( "Loading Plug-ins" ) );
        loadPlugins();
    }

    splash->done();
    show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->isSet( "import" ) ) {
        // I need to do this in delayed init to get the import window on top of the normal window
        ImportExport::Import::imageImport( KCmdLineArgs::makeURL( args->getOption("import") ) );
    }
    else {
        // I need to postpone this otherwise the tip dialog will not get focus on start up
        KTipDialog::showTip( this );

        possibleRunSuvey();
    }

#ifdef HASEXIV2
    Exif::Database* exifDB = Exif::Database::instance(); // Load the database
    if ( exifDB->isAvailable() && !exifDB->isOpen() ) {
        KMessageBox::sorry( this, i18n("EXIF database cannot be opened. Check that the image root directory is writable.") );
    }
#endif

    tellPeopleAboutTheVideos();
    checkIfAllFeaturesAreInstalled();

}


bool MainWindow::Window::slotExit()
{
    if ( Utilities::runningDemo() ) {
        QString txt = i18n("<p><b>Delete Your Temporary Demo Database</b></p>"
                           "<p>I hope you enjoyed the KPhotoAlbum demo. The demo database was copied to "
                           "/tmp, should it be deleted now? If you do not delete it, it will waste disk space; "
                           "on the other hand, if you want to come back and try the demo again, you "
                           "might want to keep it around with the changes you made through this session.</p>" );
        int ret = KMessageBox::questionYesNoCancel( this, txt, i18n("Delete Demo Database"),
                                                    KStdGuiItem::yes(), KStdGuiItem::no(),
                                                    QString::fromLatin1("deleteDemoDatabase") );
        if ( ret == KMessageBox::Cancel )
            return false;
        else if ( ret == KMessageBox::Yes ) {
            Utilities::deleteDemo();
            goto doQuit;
        }
        else {
            // pass through to the check for dirtyness.
        }
    }

    if ( _dirtyIndicator->isSaveDirty() || !DB::ImageDB::instance()->isClipboardEmpty() ) {
        int ret = KMessageBox::warningYesNoCancel( this, i18n("Do you want to save the changes?"),
                                                   i18n("Save Changes?") );
        if ( ret == KMessageBox::Cancel )
            return false;
        if ( ret == KMessageBox::Yes ) {
            slotSave();
        }
        if ( ret == KMessageBox::No ) {
            QDir().remove( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1(".#index.xml") );
        }
    }

 doQuit:
    qApp->quit();
    return true;
}

void MainWindow::Window::slotOptions()
{
    if ( ! _optionsDialog ) {
        _optionsDialog = new Settings::SettingsDialog( this );
        connect( _optionsDialog, SIGNAL( changed() ), this, SLOT( reloadThumbnailsAndFlushCache() ) );
        connect( _optionsDialog, SIGNAL( changed() ), this, SLOT( startAutoSaveTimer() ) );
    }
    _optionsDialog->show();
}


void MainWindow::Window::slotConfigureAllImages()
{
    configureImages( false );
}


void MainWindow::Window::slotConfigureImagesOneAtATime()
{
    configureImages( true );
}



void MainWindow::Window::configureImages( bool oneAtATime )
{
    QStringList list = selected();
    if ( list.count() == 0 )  {
        QMessageBox::warning( this,  i18n("No Selection"),  i18n("No item is selected.") );
    }
    else {
        DB::ImageInfoList images;
        for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            images.append( DB::ImageDB::instance()->info( *it ) );
        }
        configureImages( images, oneAtATime );
    }
}

void MainWindow::Window::configureImages( const DB::ImageInfoList& list, bool oneAtATime )
{
    _instance->configImages( list, oneAtATime );
}


void MainWindow::Window::configImages( const DB::ImageInfoList& list, bool oneAtATime )
{
    createAnnotationDialog();
    _annotationDialog->configure( list,  oneAtATime );
    if ( _annotationDialog->thumbnailShouldReload() )
        reloadThumbnails(true);
    else if ( _annotationDialog->thumbnailTextShouldReload() )
        _thumbnailView->reload(false, false);
}


void MainWindow::Window::slotSearch()
{
    createAnnotationDialog();
    DB::ImageSearchInfo searchInfo = _annotationDialog->search();
    if ( !searchInfo.isNull() )
        _browser->addSearch( searchInfo );
}

void MainWindow::Window::createAnnotationDialog()
{
    Utilities::ShowBusyCursor dummy;
    if ( !_annotationDialog.isNull() )
        return;

    _annotationDialog = new AnnotationDialog::Dialog( 0,  "_annotationDialog" );
}

void MainWindow::Window::deleteAnnotationDialog()
{
    _annotationDialog->deleteLater();
    _annotationDialog = 0;
}

void MainWindow::Window::slotSave()
{
    Utilities::ShowBusyCursor dummy;
    statusBar()->message(i18n("Saving..."), 5000 );
    DB::ImageDB::instance()->save( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("index.xml"), false );
    _dirtyIndicator->saved();
    QDir().remove( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1(".#index.xml") );
    statusBar()->message(i18n("Saving... Done"), 5000 );
}

void MainWindow::Window::slotCopySelectedURLs()
{
    const QStringList& sel = selectedOnDisk();
    KURL::List urls;

    for (QStringList::const_iterator it = sel.begin(); it != sel.end(); ++it) {
        urls.append( KURL( *it ) );
    }

    QApplication::clipboard()->setData( new KURLDrag( urls ) );
}

void MainWindow::Window::slotDeleteSelected()
{
    if ( ! _deleteDialog )
        _deleteDialog = new DeleteDialog( this );
    if ( _deleteDialog->exec( selected() ) != QDialog::Accepted )
        return;

    Utilities::ShowBusyCursor dummy;
    DirtyIndicator::markDirty();

    QStringList images = _thumbnailView->imageList( ThumbnailView::ThumbnailWidget::SortedOrder );
    StringSet allImages( DB::ImageDB::instance()->images() );
    QStringList newSet;
    for( QStringList::Iterator it = images.begin(); it != images.end(); ++it ) {
        if ( allImages.contains( *it ) )
            newSet.append(*it);
    }
    showThumbNails( newSet );
}


void MainWindow::Window::slotReReadExifInfo()
{
#ifdef HASEXIV2
    QStringList files = selectedOnDisk();
    static Exif::ReReadDialog* dialog = 0;
    if ( ! dialog )
        dialog = new Exif::ReReadDialog( this );
    if ( dialog->exec( files ) == QDialog::Accepted )
            DirtyIndicator::markDirty();
#endif
}

void MainWindow::Window::slotWriteExifInfo()
{
#ifdef HASEXIV2
    QStringList files = selectedOnDisk();
    static Exif::WriteDialog* dialog = 0;
    if ( ! dialog )
        dialog = new Exif::WriteDialog( this );
    dialog->exec( files );
#endif
}

QStringList MainWindow::Window::selected( bool keepSortOrderOfDatabase )
{
    if ( _thumbnailView == _stack->visibleWidget() )
        return _thumbnailView->selection( keepSortOrderOfDatabase );
    else
        return QStringList();
}

void MainWindow::Window::slotViewNewWindow()
{
    slotView( false, false );
}

/*
 * Returns a list of files that are both selected and on disk. If there are no
 * selected files, returns all files form current context that are on disk.
 * */
QStringList MainWindow::Window::selectedOnDisk()
{
    QStringList list = selected();
    if ( list.count() == 0 )
        return DB::ImageDB::instance()->currentScope( true );

    QStringList listOnDisk;
    for( QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it ) {
        if ( DB::ImageInfo::imageOnDisk( *it ) )
            listOnDisk.append( *it );
    }

    return listOnDisk;
}

void MainWindow::Window::slotView( bool reuse, bool slideShow, bool random )
{
    launchViewer( selected(), reuse, slideShow, random );
}

void MainWindow::Window::launchViewer( QStringList files, bool reuse, bool slideShow, bool random )
{
    int seek = -1;
    if ( files.count() == 0 ) {
        files = _thumbnailView->imageList( ThumbnailView::ThumbnailWidget::ViewOrder );
    } else if ( files.count() == 1 ) {
        // we fake it so it appears the user has selected all images
        // and magically scrolls to the originally selected one
        const QString fileName = ((const QStringList&)files).first();
        files = _thumbnailView->imageList( ThumbnailView::ThumbnailWidget::ViewOrder );
        seek = files.findIndex(fileName);
    }

    if ( !files.count() )
        files = DB::ImageDB::instance()->currentScope( false );

    if ( !files.count() ) {
        KMessageBox::sorry( this, i18n("There are no images to be shown.") );
        return;
    }

    if (random)
        files = Utilities::shuffleList(files);

    Viewer::ViewerWidget* viewer;
    if ( reuse && Viewer::ViewerWidget::latest() ) {
        viewer = Viewer::ViewerWidget::latest();
        viewer->raise();
        viewer->setActiveWindow();
    }
    else
        viewer = new Viewer::ViewerWidget( "viewer" );

    viewer->show( slideShow );
    if (seek == -1)
        seek = 0;
    viewer->load( files, seek );
    viewer->raise();
}

void MainWindow::Window::slotSortByDateAndTime()
{
    DB::ImageDB::instance()->sortAndMergeBackIn( selected( true /* sort with oldest first */ ) );
    showThumbNails( DB::ImageDB::instance()->search( Browser::BrowserWidget::instance()->currentContext() ) );
    DirtyIndicator::markDirty();
}


QString MainWindow::Window::welcome()
{
    WelComeDialog dialog( this );
    dialog.exec();
    return dialog.configFileName();
}

void MainWindow::Window::closeEvent( QCloseEvent* e )
{
    bool quit = true;
    quit = slotExit();
    // If I made it here, then the user canceled
    if ( !quit )
        e->ignore();
    else
        e->accept();
}


void MainWindow::Window::slotLimitToSelected()
{
    Utilities::ShowBusyCursor dummy;
    showThumbNails( selected() );
}

void MainWindow::Window::setupMenuBar()
{
    // File menu
    KStdAction::save( this, SLOT( slotSave() ), actionCollection() );
    KStdAction::quit( this, SLOT( slotExit() ), actionCollection() );
    _generateHtml = new KAction( i18n("Generate HTML..."), 0, this, SLOT( slotExportToHTML() ), actionCollection(), "exportHTML" );

    new KAction( i18n( "Import..."), 0, this, SLOT( slotImport() ), actionCollection(), "import" );
    new KAction( i18n( "Export/Copy Images..."), 0, this, SLOT( slotExport() ), actionCollection(), "export" );


    // Go menu
    KAction* a = KStdAction::back( _browser, SLOT( back() ), actionCollection() );
    connect( _browser, SIGNAL( canGoBack( bool ) ), a, SLOT( setEnabled( bool ) ) );
    a->setEnabled( false );

    a = KStdAction::forward( _browser, SLOT( forward() ), actionCollection() );
    connect( _browser, SIGNAL( canGoForward( bool ) ), a, SLOT( setEnabled( bool ) ) );
    a->setEnabled( false );

    a = KStdAction::home( _browser, SLOT( home() ), actionCollection() );

    a = KStdAction::redisplay( _browser, SLOT( go() ), actionCollection() );

    // The Edit menu
#ifdef CODE_FOR_OLD_CUT_AND_PASTE_IN_THUMBNAIL_VIEW
    _cut = KStdAction::cut( _thumbNailViewOLD, SLOT( slotCut() ), actionCollection() );
    _paste = KStdAction::paste( _thumbNailViewOLD, SLOT( slotPaste() ), actionCollection() );
#endif
    KStdAction::copy( this, SLOT( slotCopySelectedURLs() ), actionCollection() );
    _selectAll = KStdAction::selectAll( _thumbnailView, SLOT( selectAll() ), actionCollection() );
    KStdAction::find( this, SLOT( slotSearch() ), actionCollection() );
    _deleteSelected = new KAction( i18n( "Delete Selected" ), QString::fromLatin1("editdelete"), Key_Delete, this, SLOT( slotDeleteSelected() ),
                                   actionCollection(), "deleteSelected" );
    new KAction( i18n("Remove Tokens"), 0, this, SLOT( slotRemoveTokens() ), actionCollection(), "removeTokens" );
    _configOneAtATime = new KAction( i18n( "Annotate Individual Items" ), CTRL+Key_1, this, SLOT( slotConfigureImagesOneAtATime() ),
                                     actionCollection(), "oneProp" );
    _configAllSimultaniously = new KAction( i18n( "Annotate Multiple Items at a Time" ), CTRL+Key_2, this, SLOT( slotConfigureAllImages() ),
                                            actionCollection(), "allProp" );
    _rotLeft = new KAction( i18n( "Rotate counterclockwise" ), 0, this, SLOT( slotRotateSelectedLeft() ), actionCollection(), "rotateLeft" );
    _rotRight = new KAction( i18n( "Rotate clockwise" ), 0, this, SLOT( slotRotateSelectedRight() ), actionCollection(), "rotateRight" );

    // The Images menu
    _view = new KAction( i18n("View"), CTRL+Key_I, this, SLOT( slotView() ),
                                 actionCollection(), "viewImages" );

    _viewInNewWindow = new KAction( i18n("View (In New Window)"), 0, this, SLOT( slotViewNewWindow() ),
                                           actionCollection(), "viewImagesNewWindow" );
    _runSlideShow = new KAction( i18n("Run Slide Show"), QString::fromLatin1("video"), CTRL+Key_R, this, SLOT( slotRunSlideShow() ),
                                 actionCollection(), "runSlideShow" );
    _runRandomSlideShow = new KAction( i18n( "Run Randomized Slide Show" ), 0, this, SLOT( slotRunRandomizedSlideShow() ),
                                       actionCollection(), "runRandomizedSlideShow" );
    KToggleAction* incr = new KToggleAction( i18n("Show &Oldest First"), 0, this,
                                             SLOT( slotOrderIncr() ), actionCollection(), "orderIncr" );
    KToggleAction* decr = new KToggleAction( i18n("Show &Newest First"), 0, this,
                                             SLOT( slotOrderDecr() ), actionCollection(), "orderDecr" );
    incr->setExclusiveGroup( QString::fromLatin1( "Sort Direction") );
    decr->setExclusiveGroup(QString::fromLatin1( "Sort Direction") );
    incr->setChecked( !Settings::SettingsData::instance()->showNewestThumbnailFirst() );
    decr->setChecked( Settings::SettingsData::instance()->showNewestThumbnailFirst() );

    _sortByDateAndTime = new KAction( i18n("Sort Selected by Date && Time"), 0, this, SLOT( slotSortByDateAndTime() ), actionCollection(), "sortImages" );
    _limitToMarked = new KAction( i18n("Limit View to Marked"), 0, this, SLOT( slotLimitToSelected() ),
                                  actionCollection(), "limitToMarked" );
    _jumpToContext = new KAction( i18n("Jump to Context"), CTRL+Key_J, this, SLOT( slotJumpToContext() ), actionCollection(), "jumpToContext" );
    _jumpToContext->setIconSet( KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kphotoalbum" ), KIcon::Small ) );

    _lock = new KAction( i18n("Lock Images"), 0, this, SLOT( lockToDefaultScope() ),
                         actionCollection(), "lockToDefaultScope" );
    _unlock = new KAction( i18n("Unlock"), 0, this, SLOT( unlockFromDefaultScope() ),
                           actionCollection(), "unlockFromDefaultScope" );
    new KAction( i18n("Change Password..."), 0, this, SLOT( changePassword() ),
                 actionCollection(), "changeScopePasswd" );

    _setDefaultPos = new KAction( i18n("Lock Away All Other Items"), 0, this, SLOT( setDefaultScopePositive() ),
                                  actionCollection(), "setDefaultScopePositive" );
    _setDefaultNeg = new KAction( i18n("Lock Away Current Set of Items"), 0, this, SLOT( setDefaultScopeNegative() ),
                                  actionCollection(), "setDefaultScopeNegative" );

    // Maintenance
    new KAction( i18n("Display Images and Videos Not on Disk"), 0, this, SLOT( slotShowNotOnDisk() ), actionCollection(), "findUnavailableImages" );
    new KAction( i18n("Display Images and Videos with Incomplete Dates..."), 0, this, SLOT( slotShowImagesWithInvalidDate() ), actionCollection(), "findImagesWithInvalidDate" );
    new KAction( i18n("Display Images and Videos with Changed MD5 Sum"), 0, this, SLOT( slotShowImagesWithChangedMD5Sum() ), actionCollection(), "findImagesWithChangedMD5Sum" );

    new KAction( i18n("Recalculate Checksum"), 0, this, SLOT( slotRecalcCheckSums() ), actionCollection(), "rebuildMD5s" );
    new KAction( i18n("Rescan for Images and Videos"), 0, DB::ImageDB::instance(), SLOT( slotRescan() ), actionCollection(), "rescan" );
#ifdef HASEXIV2
    new KAction( i18n("Re-Read Metadata from Files..."), 0, this, SLOT( slotReReadExifInfo() ), actionCollection(), "reReadExifInfo" );
    new KAction( i18n("Write Metadata to Files..."), 0, this, SLOT( slotWriteExifInfo() ), actionCollection(), "writeExifInfo" );
#endif

#ifdef SQLDB_SUPPORT
    new KAction( i18n("Convert Backend...(Experimental!)" ), 0, this, SLOT( convertBackend() ), actionCollection(), "convertBackend" );
#endif


    new KAction( i18n("Build Thumbnails"), 0, this, SLOT( slotBuildThumbnails() ), actionCollection(), "buildThumbs" );
    new KAction( i18n("Remove All KimDaBa 2.1 Thumbnails"), 0, this, SLOT( slotRemoveAllThumbnails() ), actionCollection(), "removeAllThumbs" );

    // Settings
    KStdAction::preferences( this, SLOT( slotOptions() ), actionCollection() );
    KStdAction::keyBindings( this, SLOT( slotConfigureKeyBindings() ), actionCollection() );
    KStdAction::configureToolbars( this, SLOT( slotConfigureToolbars() ), actionCollection() );
    new KAction( i18n("Enable All Messages"), 0, this, SLOT( slotReenableMessages() ), actionCollection(), "readdAllMessages" );

    _viewMenu = new KActionMenu( i18n("Configure View"), QString::fromLatin1( "view_choose" ),
                                         actionCollection(), "configureView" );
    _viewMenu->setDelayed( false );
    connect( _browser, SIGNAL( showsContentView( bool ) ), _viewMenu, SLOT( setEnabled( bool ) ) );
    _smallListView = new KRadioAction( i18n("List View"), KShortcut(), _browser, SLOT( slotSmallListView() ),
                                                    _viewMenu );
    _viewMenu->insert( _smallListView );
    _smallListView->setExclusiveGroup( QString::fromLatin1("configureview") );

    _largeListView = new KRadioAction( i18n("List View with Custom Icons"), KShortcut(), _browser, SLOT( slotLargeListView() ),
                                                    _viewMenu );
    _viewMenu->insert( _largeListView );
    _largeListView->setExclusiveGroup( QString::fromLatin1("configureview") );

    _smallIconView = new KRadioAction( i18n("Icon View"), KShortcut(), _browser, SLOT( slotSmallIconView() ),
                                                    _viewMenu );
    _viewMenu->insert( _smallIconView );
    _smallIconView->setExclusiveGroup( QString::fromLatin1("configureview") );

    _largeIconView = new KRadioAction( i18n("Icon View with Custom Icons"), KShortcut(), _browser, SLOT( slotLargeIconView() ),
                                                    _viewMenu );
    _viewMenu->insert( _largeIconView );
    _largeIconView->setExclusiveGroup( QString::fromLatin1("configureview") );


    connect( _browser, SIGNAL( currentViewTypeChanged( DB::Category::ViewType ) ),
             this, SLOT( slotUpdateViewMenu( DB::Category::ViewType ) ) );
    // The help menu
    KStdAction::tipOfDay( this, SLOT(showTipOfDay()), actionCollection() );
    KToggleAction* taction = new KToggleAction( i18n("Show Tooltips in Thumbnails Window"), CTRL+Key_T, actionCollection(), "showToolTipOnImages" );
    connect( taction, SIGNAL( toggled( bool ) ), _thumbnailView, SLOT( showToolTipsOnImages( bool ) ) );
    new KAction( i18n("Run KPhotoAlbum Demo"), 0, this, SLOT( runDemo() ), actionCollection(), "runDemo" );
    new KAction( i18n("Answer KPhotoAlbum Survey..."), 0, this, SLOT( runSurvey() ), actionCollection(), "runSurvey" );
    new KAction( i18n("Donate Money..."), 0, this, SLOT( donateMoney() ), actionCollection(), "donate" );
    new KAction( i18n("KPhotoAlbum Feature Status"), 0, this, SLOT( showFeatures() ), actionCollection(), "features" );

    // Context menu actions
#ifdef HASEXIV2
    _showExifDialog = new KAction( i18n("Show Exif Info"), 0, this, SLOT( slotShowExifInfo() ), actionCollection(), "showExifInfo" );
#endif
    _recreateThumbnails = new KAction( i18n("Recreate Selected Thumbnails"), 0, this, SLOT( slotRecreateThumbnail() ), actionCollection(), "recreateThumbnails" );

#ifdef CODE_FOR_OLD_CUT_AND_PASTE_IN_THUMBNAIL_VIEW
    connect( _thumbNailViewOLD, SIGNAL( changed() ), this, SLOT( slotChanges() ) );
#endif
    createGUI( QString::fromLatin1( "kphotoalbumui.rc" ), false );
}

void MainWindow::Window::slotExportToHTML()
{
    if ( ! _htmlDialog )
        _htmlDialog = new HTMLGenerator::HTMLDialog( this, "htmlExportDialog" );
    _htmlDialog->exec( selectedOnDisk() );
}

void MainWindow::Window::startAutoSaveTimer()
{
    int i = Settings::SettingsData::instance()->autoSave();
    _autoSaveTimer->stop();
    if ( i != 0 ) {
        _autoSaveTimer->start( i * 1000 * 60  );
    }
}

void MainWindow::Window::slotAutoSave()
{
    if ( _dirtyIndicator->isAutoSaveDirty() ) {
        Utilities::ShowBusyCursor dummy;
        statusBar()->message(i18n("Auto saving...."));
        DB::ImageDB::instance()->save( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1(".#index.xml"), true );
        statusBar()->message(i18n("Auto saving.... Done"), 5000);
        _dirtyIndicator->autoSaved();
    }
}


void MainWindow::Window::showThumbNails()
{
    reloadThumbnails(false);
    _stack->raiseWidget( _thumbnailView );
    _thumbnailView->setFocus();
    updateStates( true );
}

void MainWindow::Window::showBrowser()
{
    _stack->raiseWidget( _browser );
    _browser->setFocus();
    updateStates( false );
}


void MainWindow::Window::slotOptionGroupChanged()
{
    delete _annotationDialog;
    _annotationDialog = 0;
    DirtyIndicator::markDirty();
}

void MainWindow::Window::showTipOfDay()
{
    KTipDialog::showTip( this, QString::null, true );
}

void MainWindow::Window::pathChanged( const QString& path )
{
    static bool itemVisible = false;
    QString text = path;

    if ( text.length() > 80 )
        text = text.left(80) + QString::fromLatin1( "..." );

    if ( text.isEmpty() ) {
        if ( itemVisible ) {
            statusBar()->removeItem( 0 );
            itemVisible = false;
        }
    }
    else if ( !itemVisible ) {
        statusBar()->insertItem( text, 0 );
        itemVisible = true;
    }
    else
        statusBar()->changeItem( text, 0 );

}

void MainWindow::Window::runDemo()
{
    KProcess* process = new KProcess;
    *process << "kphotoalbum" << "-demo";
    process->start();
}

bool MainWindow::Window::load()
{
    // Let first try to find a config file.
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString configFile = QString::null;

    if ( args->isSet( "c" ) ) {
        configFile = args->getOption( "c" );
    }
    else if ( args->isSet( "demo" ) )
        configFile = Utilities::setupDemo();
    else {
        bool showWelcome = false;
        KConfig* config = kapp->config();
        if ( config->hasKey( QString::fromLatin1("configfile") ) ) {
            configFile = config->readEntry( QString::fromLatin1("configfile") );
            if ( !QFileInfo( configFile ).exists() )
                showWelcome = true;
        }
        else {
            // KimDaBa compatibility
            KSimpleConfig oldConfig( QString::fromLatin1("kimdaba") );
            if ( oldConfig.hasKey( QString::fromLatin1("configfile") ) ) {
                configFile = oldConfig.readEntry( QString::fromLatin1("configfile") );
                if ( !QFileInfo( configFile ).exists() )
                    showWelcome = true;
                kapp->config()->writeEntry( QString::fromLatin1("configfile"), configFile );
            }
            else
                showWelcome = true;
        }

        if ( showWelcome ) {
            SplashScreen::instance()->hide();
            configFile = welcome();
        }
    }
    if ( configFile.isNull() )
        return false;

    if (configFile.startsWith( QString::fromLatin1( "~" ) ) )
        configFile = QDir::home().path() + QString::fromLatin1( "/" ) + configFile.mid(1);

    Settings::SettingsData::setup( QFileInfo( configFile ).dirPath( true ) );

    if ( Settings::SettingsData::instance()->showSplashScreen() ) {
        SplashScreen::instance()->show();
        qApp->processEvents();
    }

    // Choose backend
    QString backEnd = Settings::SettingsData::instance()->backend();
    // Command line override for backend
    if ( args->isSet( "e" ) )
        backEnd = args->getOption( "e" );

    // Initialize correct back-end
    if ( backEnd == QString::fromLatin1("sql") ) {
#ifdef SQLDB_SUPPORT
        // SQL back-end needs some extra configuration first
        KConfig* config = kapp->config();
        config->setGroup(QString::fromLatin1("SQLDB"));
        try {
            SQLDB::DatabaseAddress address = SQLDB::readConnectionParameters(*config);

            // Initialize SQLDB with the paramaters
            DB::ImageDB::setupSQLDB(address);
            return true;
        }
        catch (SQLDB::Error& e){
            KMessageBox::error(this, i18n("SQL backend initialization failed, "
                                          "because following error occurred:\n%1").arg(e.message()));
        }
#else
        KMessageBox::error(this, i18n("SQL database support is not compiled in."));
#endif
    }
    else if ( backEnd == QString::fromLatin1("xml") );
    else {
        KMessageBox::error(this, i18n("Invalid database backend: %1").arg(backEnd));
    }

    if (backEnd != QString::fromLatin1("xml")) {
        int answer =
            KMessageBox::questionYesNo(this, i18n("Do you want to use XML backend instead?"));
        if (answer != KMessageBox::Yes)
            return false;
    }

    DB::ImageDB::setupXMLDB( configFile );

    return true;
}

void MainWindow::Window::contextMenuEvent( QContextMenuEvent* e )
{
    if ( _stack->visibleWidget() == _thumbnailView ) {
        QPopupMenu menu( this, "context popup menu");
        _configOneAtATime->plug( &menu );
        _configAllSimultaniously->plug( &menu );
        _runSlideShow->plug( &menu );
        _runRandomSlideShow->plug( &menu );
#ifdef HASEXIV2
        _showExifDialog->plug( &menu );
#endif

        menu.insertSeparator();
        _rotLeft->plug( &menu );
        _rotRight->plug( &menu );
        _recreateThumbnails->plug( &menu );
        menu.insertSeparator();

        _view->plug( &menu );
        _viewInNewWindow->plug( &menu );

        ExternalPopup* externalCommands = new ExternalPopup( &menu );
        DB::ImageInfoPtr info = DB::ImageInfoPtr( 0 );
        QString fileName = _thumbnailView->fileNameUnderCursor();
        if ( !fileName.isNull() )
            info = DB::ImageDB::instance()->info( fileName );

        externalCommands->populate( info, selected() );
        int id = menu.insertItem( i18n( "Invoke External Program" ), externalCommands );
        if ( info == 0 && selected().count() == 0 )
            menu.setItemEnabled( id, false );

        menu.exec( QCursor::pos() );

        delete externalCommands;
    }
    e->consume();
}

void MainWindow::Window::setDefaultScopePositive()
{
    Settings::SettingsData::instance()->setCurrentLock( _browser->currentContext(), false );
}

void MainWindow::Window::setDefaultScopeNegative()
{
    Settings::SettingsData::instance()->setCurrentLock( _browser->currentContext(), true );
}

void MainWindow::Window::lockToDefaultScope()
{
    int i = KMessageBox::warningContinueCancel( this,
                                                i18n( "<p>The password protection is only a means of allowing your little sister "
                                                      "to look in your images, without getting to those embarrassing images from "
                                                      "your last party.</p>"
                                                      "<p>In other words, anyone with access to the index.xml file can easily "
                                                      "circumvent this password.</b></p>"),
                                                i18n("Password Protection"),
                                                KStdGuiItem::cont(),
                                                QString::fromLatin1( "lockPassWordIsNotEncruption" ) );
    if ( i == KMessageBox::Cancel )
        return;

    setLocked( true, false );

}

void MainWindow::Window::unlockFromDefaultScope()
{
    QCString passwd;
    bool OK = ( Settings::SettingsData::instance()->password().isEmpty() );
    while ( !OK ) {
        int code = KPasswordDialog::getPassword( passwd, i18n("Type in Password to Unlock"));
        if ( code == QDialog::Rejected )
            return;
        OK = (Settings::SettingsData::instance()->password() == QString(passwd));

        if ( !OK )
            KMessageBox::sorry( this, i18n("Invalid password.") );
    }
    setLocked( false, false );
}

void MainWindow::Window::setLocked( bool locked, bool force )
{
    static QPixmap* lockedPix = new QPixmap( SmallIcon( QString::fromLatin1( "key" ) ) );
    _lockedIndicator->setFixedWidth( lockedPix->width() );

    if ( locked )
        _lockedIndicator->setPixmap( *lockedPix );
    else
        _lockedIndicator->setPixmap( QPixmap() );

    Settings::SettingsData::instance()->setLocked( locked, force );

    _lock->setEnabled( !locked );
    _unlock->setEnabled( locked );
    _setDefaultPos->setEnabled( !locked );
    _setDefaultNeg->setEnabled( !locked );
    _browser->reload();
}

void MainWindow::Window::changePassword()
{
    QCString passwd;
    bool OK = ( Settings::SettingsData::instance()->password().isEmpty() );

    while ( !OK ) {
        int code = KPasswordDialog::getPassword( passwd, i18n("Type in Old Password"));
        if ( code == QDialog::Rejected )
            return;
        OK = (Settings::SettingsData::instance()->password() == QString(passwd));

        if ( !OK )
            KMessageBox::sorry( this, i18n("Invalid password.") );
    }

    int code = KPasswordDialog::getNewPassword( passwd, i18n("Type in New Password"));
    if ( code == QDialog::Accepted )
        Settings::SettingsData::instance()->setPassword( passwd );
}

void MainWindow::Window::slotConfigureKeyBindings()
{
    Viewer::ViewerWidget* viewer = new Viewer::ViewerWidget( "viewer" ); // Do not show, this is only used to get a key configuration
    KKeyDialog* dialog = new KKeyDialog();
    dialog->insert( actionCollection(), i18n( "General" ) );
    dialog->insert( viewer->actions(), i18n("Viewer") );

#ifdef HASKIPI
    loadPlugins();
    KIPI::PluginLoader::PluginList list = _pluginLoader->pluginList();
    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin(); it != list.end(); ++it ) {
        KIPI::Plugin* plugin = (*it)->plugin();
        if ( plugin )
            dialog->insert( plugin->actionCollection(), (*it)->comment() );
    }
#endif

    createAnnotationDialog();
    dialog->insert( _annotationDialog->actions(), i18n("Annotation Dialog" ) );

    dialog->configure();

    delete dialog;
    delete viewer;
}

void MainWindow::Window::slotSetFileName( const QString& fileName )
{
    statusBar()->message( fileName, 4000 );
}

void MainWindow::Window::slotThumbNailSelectionChanged()
{
    QStringList selection = _thumbnailView->selection();

    _configAllSimultaniously->setEnabled(selection.count() > 1 );
    _configOneAtATime->setEnabled(selection.count() >= 1 );
    _sortByDateAndTime->setEnabled(selection.count() > 1 );
    _recreateThumbnails->setEnabled( selection.count() >= 1 );
    _rotLeft->setEnabled( selection.count() >= 1 );
    _rotRight->setEnabled( selection.count() >= 1 );
}

void MainWindow::Window::rotateSelected( int angle )
{
    QStringList list = selected();
    if ( list.count() == 0 )  {
        QMessageBox::warning( this,  i18n("No Selection"),  i18n("No item is selected.") );
    } else {
        for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
            DB::ImageDB::instance()->info( *it )->rotate( angle );
        }
        _dirtyIndicator->markDirty();
        reloadThumbnailsAndFlushCache();
    }
}

void MainWindow::Window::slotPluginThatResetsOrientationActivated()
{
    QStringList list = selected();
    if ( list.count() == 0 )  {
        QMessageBox::warning( this,  i18n("No Selection"),  i18n("Weird, no item is selected... who knows how the plugin action ended...") );
    } else {
        for ( QStringList::const_iterator it = list.begin(); it != list.end(); ++it ) {
            DB::ImageDB::instance()->info( *it )->setAngle( 0 );
        }
        _dirtyIndicator->markDirty();
        reloadThumbnailsAndFlushCache();
        QMessageBox::information( this, i18n("Orientation changed"), i18n("Rotation information for %1 images has been reset in the database.").arg( list.count() ) );
    }
}

void MainWindow::Window::slotRotateSelectedLeft()
{
    rotateSelected( -90 );
}

void MainWindow::Window::slotRotateSelectedRight()
{
    rotateSelected( 90 );
}

void MainWindow::Window::reloadThumbnails(bool flushCache)
{
    _thumbnailView->reload( flushCache );
    slotThumbNailSelectionChanged();
}

void MainWindow::Window::reloadThumbnailsAndFlushCache()
{
    reloadThumbnails(true);
}

void MainWindow::Window::slotUpdateViewMenu( DB::Category::ViewType type )
{
    if ( type == DB::Category::ListView )
        _smallListView->setChecked( true );
    else if ( type == DB::Category::ThumbedListView )
        _largeListView->setChecked( true );
    else if ( type == DB::Category::IconView )
        _smallIconView->setChecked( true );
    else if ( type == DB::Category::ThumbedIconView )
        _largeIconView->setChecked( true );
}

void MainWindow::Window::slotShowNotOnDisk()
{
    QStringList allImages = DB::ImageDB::instance()->images();
    QStringList notOnDisk;
    for( QStringList::ConstIterator it = allImages.begin(); it != allImages.end(); ++it ) {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info(*it);
        QFileInfo fi( info->fileName() );
        if ( !fi.exists() )
            notOnDisk.append(*it);
    }

    showThumbNails( notOnDisk );
}


void MainWindow::Window::slotShowImagesWithChangedMD5Sum()
{
    Utilities::ShowBusyCursor dummy;
    StringSet changed = DB::ImageDB::instance()->imagesWithMD5Changed();
    showThumbNails( changed.toList() );
}


void MainWindow::Window::donateMoney()
{
    DonateDialog donate( this, "Donate Money" );
    donate.exec();
}

void MainWindow::Window::updateStates( bool thumbNailView )
{
#ifdef CODE_FOR_OLD_CUT_AND_PASTE_IN_THUMBNAIL_VIEW
    _cut->setEnabled( thumbNailView );
    _paste->setEnabled( thumbNailView );
#endif
    _selectAll->setEnabled( thumbNailView );
    _deleteSelected->setEnabled( thumbNailView );
    _limitToMarked->setEnabled( thumbNailView );
}

void MainWindow::Window::slotRemoveAllThumbnails()
{
    DeleteThumbnailsDialog dialog( this );
    dialog.exec();
}

void MainWindow::Window::slotRunSlideShow()
{
    slotView( true, true );
}

void MainWindow::Window::slotRunRandomizedSlideShow()
{
    slotView( true, true, true );
}

MainWindow::Window* MainWindow::Window::theMainWindow()
{
    Q_ASSERT( _instance );
    return _instance;
}

void MainWindow::Window::slotConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config(), QString::fromLatin1("MainWindow"));
    KEditToolbar dlg(actionCollection());
    connect(&dlg, SIGNAL( newToolbarConfig() ),
                  SLOT( slotNewToolbarConfig() ));
    dlg.exec();

}

void MainWindow::Window::slotNewToolbarConfig()
{
    createGUI();
    applyMainWindowSettings(KGlobal::config(), QString::fromLatin1("MainWindow"));
}

void MainWindow::Window::slotImport()
{
    ImportExport::Import::imageImport();
}

void MainWindow::Window::slotExport()
{
    ImportExport::Export::imageExport( selectedOnDisk() );
}

void MainWindow::Window::slotReenableMessages()
{
    int ret = KMessageBox::questionYesNo( this, i18n("<p>Really enable all messageboxes where you previously "
                                                     "checked the do-not-show-again check box?</p>" ) );
    if ( ret == KMessageBox::Yes )
        KMessageBox::enableAllMessages();

}

void MainWindow::Window::setupPluginMenu()
{
    QObjectList *l = queryList( "QPopupMenu", "plugins" );
    QObject *obj;
    QPopupMenu* menu = NULL;
    for ( QObjectListIt it( *l ); (obj = it.current()) != 0; ) {
        ++it;
        menu = static_cast<QPopupMenu*>( obj );
        break;
    }
    delete l; // delete the list, not the objects

#ifdef HASKIPI
    connect( menu, SIGNAL( aboutToShow() ), this, SLOT( loadPlugins() ) );
    _hasLoadedPlugins = false;
#else
    delete menu;
    _hasLoadedPlugins = true;
#endif
}

void MainWindow::Window::loadPlugins()
{
#ifdef HASKIPI
    Utilities::ShowBusyCursor dummy;
    if ( _hasLoadedPlugins )
        return;

    _pluginInterface = new Plugins::Interface( this, "demo interface" );
    connect( _pluginInterface, SIGNAL( imagesChanged( const KURL::List& ) ), this, SLOT( slotImagesChanged( const KURL::List& ) ) );

    QStringList ignores;
    ignores << QString::fromLatin1( "CommentsEditor" )
            << QString::fromLatin1( "HelloWorld" );

    _pluginLoader = new KIPI::PluginLoader( ignores, _pluginInterface );
    connect( _pluginLoader, SIGNAL( replug() ), this, SLOT( plug() ) );
    _pluginLoader->loadPlugins();

    // Setup signals
    connect( _thumbnailView, SIGNAL( selectionChanged() ), this, SLOT( slotSelectionChanged() ) );
    _hasLoadedPlugins = true;

    // Make sure selection is updated also when plugin loading is
    // delayed. This is needed, because selection might already be
    // non-empty when loading the plugins.
    slotSelectionChanged();
#endif // HASKIPI
}


void MainWindow::Window::plug()
{
#ifdef HASKIPI
    unplugActionList( QString::fromLatin1("import_actions") );
    unplugActionList( QString::fromLatin1("export_actions") );
    unplugActionList( QString::fromLatin1("image_actions") );
    unplugActionList( QString::fromLatin1("tool_actions") );
    unplugActionList( QString::fromLatin1("batch_actions") );

    QPtrList<KAction> importActions;
    QPtrList<KAction> exportActions;
    QPtrList<KAction> imageActions;
    QPtrList<KAction> toolsActions;
    QPtrList<KAction> batchActions;

    KIPI::PluginLoader::PluginList list = _pluginLoader->pluginList();
    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin(); it != list.end(); ++it ) {
        KIPI::Plugin* plugin = (*it)->plugin();
        if ( !plugin || !(*it)->shouldLoad() )
            continue;

        plugin->setup( this );

        KActionPtrList actions = plugin->actions();
        for( KActionPtrList::Iterator it = actions.begin(); it != actions.end(); ++it ) {
            KIPI::Category category = plugin->category( *it );

            /**
             * The following attempt is highly unportable result of three hours
             * of RTTI fighting. Enjoy :)
             * 
             * We can't use
             * dynamic_cast<KIPIJPEGLossLessPlugin::Plugin_JPEGLossless> because
             * the header file doesn't have to be available. If we try to use a
             * lazy 'namespace ... {class ...}' stuff, we get:
             *
             * error: cannot dynamic_cast `plugin' (of type `class
             * KIPI::Plugin*') to type `struct
             * KIPIJPEGLossLessPlugin::Plugin_JPEGLossless*' (target is not
             * pointer or reference to complete type).
             *
             * So let's just live with this and hope compiler doesn't mangle class
             * name too much...
             **/
            if ( ( QCString( typeid( *plugin ).name() ).find( "Plugin_JPEGLossless" ) != -1 ) && 
                    ( qstrcmp( (*it)->name(), "rotate_exif" ) == 0 ) ) {
                connect( *it, SIGNAL( activated() ), this, SLOT( slotPluginThatResetsOrientationActivated() ) );
            }

            if (  category == KIPI::IMAGESPLUGIN ||  category == KIPI::COLLECTIONSPLUGIN )
                imageActions.append( *it );

            else if ( category == KIPI::IMPORTPLUGIN )
                importActions.append( *it );

            else if ( category == KIPI::EXPORTPLUGIN )
                exportActions.append( *it );

            else if ( category == KIPI::TOOLSPLUGIN )
                toolsActions.append( *it );

            else if ( category == KIPI::BATCHPLUGIN )
                batchActions.append( *it );

            else {
                kdDebug() << "Unknow category\n";
            }
        }
        plugin->actionCollection()->readShortcutSettings();
    }

    // For this to work I need to pass false as second arg for createGUI
    plugActionList( QString::fromLatin1("import_actions"), importActions );
    plugActionList( QString::fromLatin1("export_actions"), exportActions );
    plugActionList( QString::fromLatin1("image_actions"), imageActions );
    plugActionList( QString::fromLatin1("tool_actions"), toolsActions );
    plugActionList( QString::fromLatin1("batch_actions"), batchActions );
#endif
}


void MainWindow::Window::slotImagesChanged( const KURL::List& urls )
{
    for( KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it ) {
        ImageManager::ImageLoader::removeThumbnail( (*it).path() );
    }
    _dirtyIndicator->markDirty();
    reloadThumbnails(true);
}

DB::ImageSearchInfo MainWindow::Window::currentContext()
{
    return _browser->currentContext();
}

QString MainWindow::Window::currentBrowseCategory() const
{
    return _browser->currentCategory();
}

void MainWindow::Window::slotSelectionChanged()
{
#ifdef HASKIPI
    _pluginInterface->slotSelectionChanged( selected().count() != 0);
#endif
}

void MainWindow::Window::resizeEvent( QResizeEvent* )
{
    if ( Settings::SettingsData::ready() && isVisible() )
        Settings::SettingsData::instance()->setWindowGeometry( Settings::MainWindow, geometry() );
}

void MainWindow::Window::moveEvent( QMoveEvent * )
{
    if ( Settings::SettingsData::ready() && isVisible() )
        Settings::SettingsData::instance()->setWindowGeometry( Settings::MainWindow, geometry() );
}


void MainWindow::Window::slotRemoveTokens()
{
    if ( !_tokenEditor )
        _tokenEditor = new TokenEditor( this, "token editor" );
    _tokenEditor->show();
    connect( _tokenEditor, SIGNAL( finished() ), _browser, SLOT( go() ) );
}

void MainWindow::Window::updateDateBar( const QString& path )
{
    static QString lastPath = QString::fromLatin1("ThisStringShouldNeverBeSeenSoWeUseItAsInitialContent");
    if ( path != lastPath )
        updateDateBar();
    lastPath = path;
}

void MainWindow::Window::updateDateBar()
{
    _dateBar->setImageDateCollection( DB::ImageDB::instance()->rangeCollection() );
}


void MainWindow::Window::slotShowImagesWithInvalidDate()
{
    InvalidDateFinder finder( this, "invaliddatefinder" );
    if ( finder.exec() == QDialog::Accepted )
        showThumbNails();
}

void MainWindow::Window::showDateBarTip( const QString& msg )
{
    statusBar()->message( msg, 3000 );
}

void MainWindow::Window::slotJumpToContext()
{
    QString fileName =_thumbnailView->currentItem();
    if ( !fileName.isNull() ) {
        _browser->addImageView( fileName );
   }
}

void MainWindow::Window::setDateRange( const DB::ImageDate& range )
{
    DB::ImageDB::instance()->setDateRange( range, _dateBar->includeFuzzyCounts() );
    _browser->reload();
    reloadThumbnails(false);
}

void MainWindow::Window::clearDateRange()
{
    DB::ImageDB::instance()->clearDateRange();
    _browser->reload();
    reloadThumbnails(false);
}

void MainWindow::Window::runSurvey()
{
    Survey::MySurvey survey(this);
    survey.exec();
}

void MainWindow::Window::possibleRunSuvey()
{
    Survey::MySurvey survey(this);
    survey.possibleExecSurvey();
}



void MainWindow::Window::showThumbNails( const QStringList& list )
{
    _thumbnailView->setImageList( list );
    _partial->setMatchCount( list.count() );
    showThumbNails();
}

void MainWindow::Window::convertBackend()
{
#ifdef SQLDB_SUPPORT
    // Converting from SQLDB to the same SQLDB will not work and there
    // is currently no way to check if two SQL back-ends use the same
    // database. So this is my current workaround for it.
    if (dynamic_cast<SQLDB::Database*>(DB::ImageDB::instance())) {
        KMessageBox::sorry(this, i18n("Database conversion from SQL database is not yet supported."));
        return;
    }

    KConfig* config = kapp->config();
    if (!config->hasGroup(QString::fromLatin1("SQLDB"))) {
        int ret =
            KMessageBox::questionYesNo(this, i18n("You should set SQL database settings before the conversion. "
                                                  "Do you want to do this now?"));
        if (ret != KMessageBox::Yes)
            return;
        if (!_optionsDialog)
            _optionsDialog = new Settings::SettingsDialog(this);
        _optionsDialog->showBackendPage();
        ret = _optionsDialog->exec();
        if (ret != Settings::SettingsDialog::Accepted)
            return;
    }
    config->setGroup(QString::fromLatin1("SQLDB"));
    try {
        SQLDB::DatabaseAddress address = SQLDB::readConnectionParameters(*config);

        SQLDB::Database sqlBackend(address);

        // TODO: ask if old database should be flushed first

        KProgressDialog dialog(this);
        dialog.setModal(true);
        dialog.setCaption(i18n("Converting database"));
        dialog.setLabel(QString::fromLatin1("<p><b><nobr>%1</nobr></b></p><p>%2</p>")
                        .arg(i18n("Converting database to SQL."))
                        .arg(i18n("Please wait.")));
        dialog.setAllowCancel(false);
        dialog.setAutoClose(true);
        dialog.setFixedSize(dialog.sizeHint());
        dialog.setMinimumDuration(0);
        qApp->processEvents();

        DB::ImageDB::instance()->convertBackend(&sqlBackend, dialog.progressBar());

        KMessageBox::information(this, i18n("Database conversion is ready."));
    }
    catch (SQLDB::Error& e) {
        KMessageBox::error(this, i18n("Database conversion failed, because following error occurred:\n%1").arg(e.message()));
    }
#endif
}

void MainWindow::Window::slotRecalcCheckSums()
{
    DB::ImageDB::instance()->slotRecalcCheckSums( selected() );
}

void MainWindow::Window::slotShowExifInfo()
{
#ifdef HASEXIV2
    QStringList items = selectedOnDisk();
    if ( !items.empty() ) {
        Exif::InfoDialog* exifDialog = new Exif::InfoDialog( items[0], this );
        exifDialog->show();
    }
#endif
}

void MainWindow::Window::showFeatures()
{
    FeatureDialog dialog(this);
    dialog.exec();
}

void MainWindow::Window::showImage( const QString& fileName )
{
    launchViewer( QStringList() << fileName, true, false, false );
}

void MainWindow::Window::slotBuildThumbnails()
{
    try {
        new ThumbnailView::ThumbnailBuilder( this ); // It will delete itself
    }
    catch (std::out_of_range&) {
        // This is thrown if there are no images to show, see ThumbnailBuilder's
        // documentation.We can safely ignore it here.
    }
}

void MainWindow::Window::slotOrderIncr()
{
    _thumbnailView->setSortDirection( ThumbnailView::OldestFirst );
}

void MainWindow::Window::slotOrderDecr()
{
    _thumbnailView->setSortDirection( ThumbnailView::NewestFirst );
}

void MainWindow::Window::slotRecreateThumbnail()
{
    QStringList selected = selectedOnDisk();
    for( QStringList::ConstIterator imageIt = selected.begin(); imageIt != selected.end(); ++imageIt ) {
        ImageManager::ImageLoader::removeThumbnail( *imageIt );

        int size = Settings::SettingsData::instance()->thumbSize();
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info( *imageIt );
        ImageManager::ImageRequest* request = new ImageManager::ImageRequest( *imageIt, QSize(size,size), info->angle(), _thumbnailView );
        ImageManager::Manager::instance()->load( request );
    }

}

void MainWindow::Window::tellPeopleAboutTheVideos()
{
    QString id = QString::fromLatin1( "KPhotoAlbumQuickStart" );
    KMessageBox::ButtonCode dummy;
    if ( !KMessageBox::shouldBeShownYesNo( id, dummy ) )
        return;

    int ret = KMessageBox::questionYesNo(this, i18n("<p>To get a quick start with KPhotoAlbum, "
                                                    "it might be worthwhile to spent 10 minutes "
                                                    "watching a few introduction videos.</p>" ),
                                         i18n( "KPhotoAlbum quick start" ),
                                         i18n( "Show Videos" ), i18n("Don't Show Videos"),
                                         id );
    if ( ret == KMessageBox::Yes )
        KRun::runURL(KURL(QString::fromLatin1("http://www.kphotoalbum.org/videos/")), QString::fromLatin1( "text/html" ) );
}

void MainWindow::Window::checkIfAllFeaturesAreInstalled()
{
    if ( !FeatureDialog::hasAllFeaturesAvailable() ) {
        const QString msg =
            i18n("<p>KPhotoAlbum does not seem to be build with support for all its features. The following is a list "
                 "indicating to you what you may miss:<ul>%1</ul></p>"
                 "<p>For details on how to solve this problem, please choose <b>Help</b>|<b>KPhotoAlbum Feature Status</b> "
                 "from the menus.</p>" )
            .arg( FeatureDialog::featureString() );
        KMessageBox::information( this, msg, i18n("Feature Check"), QString::fromLatin1( "InitialFeatureCheck" ) );
    }
}

#include "Window.moc"
