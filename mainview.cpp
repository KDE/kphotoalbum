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

#include "mainview.h"
#include "Settings/SettingsDialog.h"
#include <qapplication.h>
#include "ThumbnailView/ThumbnailView.h"
#include "ThumbnailView/ThumbnailBuilder.h"
#include "AnnotationDialog/AnnotationDialog.h"
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qdict.h>
#include "Viewer/Viewer.h"
#include "Dialogs/WelcomeDialog.h"
#include <qcursor.h>
#include "Utilities/ShowBusyCursor.h"
#include <klocale.h>
#include <qhbox.h>
#include <qwidgetstack.h>
#include <kstandarddirs.h>
#include "Dialogs/HtmlExportDialog.h"
#include <kstatusbar.h>
#include "imagecounter.h"
#include <qtimer.h>
#include <kmessagebox.h>
#include "Settings/Settings.h"
#include "Browser/Browser.h"
#include "imagedb.h"
#include "Utilities/Util.h"
#include <kapplication.h>
#include <ktip.h>
#include <kprocess.h>
#include "Dialogs/DeleteDialog.h"
#include <ksimpleconfig.h>
#include <kcmdlineargs.h>
#include <qregexp.h>
#include <qpopupmenu.h>
#include <kiconloader.h>
#include <kpassdlg.h>
#include <kkeydialog.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include "externalpopup.h"
#include "Dialogs/DonateDialog.h"
#include <kstdaction.h>
#include "Dialogs/DeleteThumbnailsDialog.h"
#include <kedittoolbar.h>
#include "ImportExport/Export.h"
#include "ImportExport/Import.h"
#ifdef HASKIPI
#  include "Plugins/Interface.h"
#  include <libkipi/pluginloader.h>
#  include <libkipi/plugin.h>
#endif
#ifdef HASEXIV2
#  include "Exif/ReReadDialog.h"
#endif
#include "ImageManager/ImageLoader.h"
#include "mysplashscreen.h"
#include <qobjectlist.h>
#include <qmenubar.h>
#include <kmenubar.h>
#include <searchbar.h>
#include "Dialogs/TokenEditor.h"
#include "categorycollection.h"
#include <qlayout.h>
#include "DateBar/DateBar.h"
#include "imagedatecollection.h"
#include "Dialogs/InvalidDateFinder.h"
#include "imageinfo.h"
#include "Survey/MySurvey.h"
#include <config.h>
#ifdef HASEXIV2
#  include "Exif/Info.h"
#  include "Exif/InfoDialog.h"
#  include "Exif/Database.h"
#endif

#include "Dialogs/FeatureDialog.h"

MainView* MainView::_instance = 0;

MainView::MainView( QWidget* parent, const char* name )
    :KMainWindow( parent,  name ), _annotationDialog(0), _dirty( false ), _autoSaveDirty( false ),
     _deleteDialog( 0 ), _dirtyIndicator(0),
     _htmlDialog(0), _tokenEditor( 0 )
{
    MySplashScreen::instance()->message( i18n("Loading Database") );
    _instance = this;

    bool gotConfigFile = load();
    if ( !gotConfigFile )
        exit(0);
    MySplashScreen::instance()->message( i18n("Loading Main Window") );

    // To avoid a race conditions where both the image loader thread creates an instance of
    // Options, and where the main thread crates an instance, we better get it created now.
    Settings::Settings::instance();

    QWidget* top = new QWidget( this, "top" );
    QVBoxLayout* lay = new QVBoxLayout( top, 6 );
    setCentralWidget( top );

    _stack = new QWidgetStack( top, "_stack" );
    lay->addWidget( _stack, 1 );

    _dateBar = new DateBar::DateBar( top, "datebar" );
    lay->addWidget( _dateBar );

    QFrame* line = new QFrame( top );
    line->setFrameStyle( QFrame::HLine | QFrame::Plain );
    line->setLineWidth(1);
    lay->addWidget( line );

    _browser = new Browser::Browser( _stack, "browser" );
    connect( _browser, SIGNAL( showingOverview() ), this, SLOT( showBrowser() ) );
    connect( _browser, SIGNAL( pathChanged( const QString& ) ), this, SLOT( pathChanged( const QString& ) ) );
    connect( _browser, SIGNAL( pathChanged( const QString& ) ), this, SLOT( updateDateBar( const QString& ) ) );
    _thumbnailView = new ThumbnailView::ThumbnailView( _stack, "_thumbnailView" );
    connect( _dateBar, SIGNAL( dateSelected( const ImageDate&, bool ) ), _thumbnailView, SLOT( gotoDate( const ImageDate&, bool ) ) );
    connect( _dateBar, SIGNAL( toolTipInfo( const QString& ) ), this, SLOT( showDateBarTip( const QString& ) ) );
    connect( Settings::Settings::instance(), SIGNAL( histogramSizeChanged( const QSize& ) ), _dateBar, SLOT( setHistogramBarSize( const QSize& ) ) );


    connect( _dateBar, SIGNAL( dateRangeChange( const ImageDate& ) ),
             this, SLOT( setDateRange( const ImageDate& ) ) );
    connect( _dateBar, SIGNAL( dateRangeCleared() ), this, SLOT( clearDateRange() ) );

    connect( _thumbnailView, SIGNAL( showImage( const QString& ) ), this, SLOT( showImage( const QString& ) ) );
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
    connect( _browser, SIGNAL( viewChanged() ), bar, SLOT( reset() ) );
    connect( _browser, SIGNAL( showsContentView( bool ) ), bar, SLOT( setEnabled( bool ) ) );

    // Setting up status bar
    QFont f( statusBar()->font() ); // Avoid flicker in the statusbar when moving over dates from the datebar
    f.setStyleHint( QFont::TypeWriter );
    f.setFamily( QString::fromLatin1( "courier" ) );
    f.setBold( true );
    statusBar()->setFont( f );

    QHBox* indicators = new QHBox( statusBar(), "indicator" );
    _dirtyIndicator = new QLabel( indicators, "_dirtyIndicator" );
    setDirty( _dirty ); // Might already have been made dirty by load above

    _lockedIndicator = new QLabel( indicators, "_lockedIndicator" );
    setLocked( Settings::Settings::instance()->isLocked() );

    statusBar()->addWidget( indicators, 0, true );

    _partial = new ImageCounter( statusBar(), "partial image counter" );
    statusBar()->addWidget( _partial, 0, true );

    ImageCounter* total = new ImageCounter( statusBar(), "total image counter" );
    statusBar()->addWidget( total, 0, true );

    // Misc
    _autoSaveTimer = new QTimer( this );
    connect( _autoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
    startAutoSaveTimer();

    connect( ImageDB::instance(), SIGNAL( totalChanged( int ) ), total, SLOT( setTotal( int ) ) );
    connect( ImageDB::instance(), SIGNAL( totalChanged( int ) ), this, SLOT( updateDateBar() ) );
    connect( ImageDB::instance(), SIGNAL( totalChanged( int ) ), _browser, SLOT( home() ) );
    connect( _browser, SIGNAL( showingOverview() ), _partial, SLOT( showingOverview() ) );
    connect( ImageDB::instance()->categoryCollection(), SIGNAL( categoryCollectionChanged() ), this, SLOT( slotOptionGroupChanged() ) );
    connect( _thumbnailView, SIGNAL( selectionChanged() ), this, SLOT( slotThumbNailSelectionChanged() ) );

    connect( ImageDB::instance(), SIGNAL( dirty() ), this, SLOT( markDirty() ) );

    total->setTotal( ImageDB::instance()->totalCount() );
    statusBar()->message(i18n("Welcome to KPhotoAlbum"), 5000 );

    QTimer::singleShot( 0, this, SLOT( delayedInit() ) );
    slotThumbNailSelectionChanged();
}

void MainView::delayedInit()
{
    MySplashScreen* splash = MySplashScreen::instance();
    setupPluginMenu();

    if ( Settings::Settings::instance()->searchForImagesOnStartup() ) {
        splash->message( i18n("Searching for New Images") );
        qApp->processEvents();
        ImageDB::instance()->slotRescan();
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

    if ( !Settings::Settings::instance()->delayLoadingPlugins() )
        loadPlugins();

#ifdef HASEXIV2
    Exif::Database* exifDB = Exif::Database::instance(); // Load the database
    if ( !exifDB || !exifDB->isOpen() ) {
        KMessageBox::sorry( this, i18n("EXIF database cannot be opened. Check that the image root directory is writable.") );
        qApp->exit(1);
    }
#endif
}


bool MainView::slotExit()
{
    if ( Utilities::runningDemo() ) {
        QString txt = i18n("<qt><p><b>Delete Your Temporary Demo Database</b></p>"
                           "<p>I hope you enjoyed the KPhotoAlbum demo. The demo database was copied to "
                           "/tmp, should it be deleted now? If you do not delete it, it will waste disk space; "
                           "on the other hand, if you want to come back and try the demo again, you "
                           "might want to keep it around with the changes you made through this session.</p></qt>" );
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

    if ( _dirty || !ImageDB::instance()->isClipboardEmpty() ) {
        int ret = KMessageBox::warningYesNoCancel( this, i18n("Do you want to save the changes?"),
                                                   i18n("Save Changes?") );
        if ( ret == KMessageBox::Cancel )
            return false;
        if ( ret == KMessageBox::Yes ) {
            slotSave();
        }
        if ( ret == KMessageBox::No ) {
            QDir().remove( Settings::Settings::instance()->imageDirectory() + QString::fromLatin1(".#index.xml") );
        }
    }

 doQuit:
    qApp->quit();
    return true;
}

void MainView::slotOptions()
{
    if ( ! _optionsDialog ) {
        _optionsDialog = new Settings::SettingsDialog( this );
        connect( _optionsDialog, SIGNAL( changed() ), this, SLOT( reloadThumbnailsAndFlushCache() ) );
        connect( _optionsDialog, SIGNAL( changed() ), this, SLOT( startAutoSaveTimer() ) );
    }
    _optionsDialog->show();
}


void MainView::slotConfigureAllImages()
{
    configureImages( false );
}


void MainView::slotConfigureImagesOneAtATime()
{
    configureImages( true );
}



void MainView::configureImages( bool oneAtATime )
{
    QStringList list = selected();
    if ( list.count() == 0 )  {
        QMessageBox::warning( this,  i18n("No Selection"),  i18n("No item is selected.") );
    }
    else {
        ImageInfoList images;
        for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            images.append( ImageDB::instance()->info( *it ) );
        }
        configureImages( images, oneAtATime );
    }
}

void MainView::configureImages( const ImageInfoList& list, bool oneAtATime )
{
    _instance->configImages( list, oneAtATime );
}


void MainView::configImages( const ImageInfoList& list, bool oneAtATime )
{
    createAnnotationDialog();
    _annotationDialog->configure( list,  oneAtATime );
    if ( _annotationDialog->thumbnailShouldReload() )
        reloadThumbnails(true);
}


void MainView::slotSearch()
{
    createAnnotationDialog();
    ImageSearchInfo searchInfo = _annotationDialog->search();
    if ( !searchInfo.isNull() )
        _browser->addSearch( searchInfo );
}

void MainView::createAnnotationDialog()
{
    Utilities::ShowBusyCursor dummy;
    if ( !_annotationDialog.isNull() )
        return;

    _annotationDialog = new AnnotationDialog::AnnotationDialog( this,  "_annotationDialog" );
    connect( _annotationDialog, SIGNAL( changed() ), this, SLOT( slotChanges() ) );
}

void MainView::deleteAnnotationDialog()
{
    _annotationDialog->deleteLater();
    _annotationDialog = 0;
}

void MainView::slotSave()
{
    Utilities::ShowBusyCursor dummy;
    statusBar()->message(i18n("Saving..."), 5000 );
    ImageDB::instance()->save( Settings::Settings::instance()->imageDirectory() + QString::fromLatin1("index.xml"), false );
    setDirty( false );
    QDir().remove( Settings::Settings::instance()->imageDirectory() + QString::fromLatin1(".#index.xml") );
    statusBar()->message(i18n("Saving... Done"), 5000 );
}

void MainView::slotDeleteSelected()
{
    if ( ! _deleteDialog )
        _deleteDialog = new Dialogs::DeleteDialog( this );
    if ( _deleteDialog->exec( selected() ) != QDialog::Accepted )
        return;

    Utilities::ShowBusyCursor dummy;
    setDirty( true );

    QStringList images = _thumbnailView->imageList( ThumbnailView::ThumbnailView::SortedOrder );
    Set<QString> allImages( ImageDB::instance()->images() );
    QStringList newSet;
    for( QStringList::Iterator it = images.begin(); it != images.end(); ++it ) {
        if ( allImages.contains( *it ) )
            newSet.append(*it);
    }
    showThumbNails( newSet );
}


void MainView::slotReReadExifInfo()
{
#ifdef HASEXIV2
    QStringList files = selectedOnDisk();
    static Exif::ReReadDialog* dialog = 0;
    if ( ! dialog )
        dialog = new Exif::ReReadDialog( this );
    if ( dialog->exec( files ) == QDialog::Accepted )
        setDirty( true );
#endif
}


QStringList MainView::selected( bool keepSortOrderOfDatabase )
{
    if ( _thumbnailView == _stack->visibleWidget() )
        return _thumbnailView->selection( keepSortOrderOfDatabase );
    else
        return QStringList();
}

void MainView::slotViewNewWindow()
{
    slotView( false, false );
}

QStringList MainView::selectedOnDisk()
{
    QStringList listOnDisk;
    QStringList list = selected();
    if ( list.count() == 0 )
        list = ImageDB::instance()->currentScope(  true );

    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( ImageInfo::imageOnDisk( *it ) )
            listOnDisk.append( *it );
    }

    return listOnDisk;
}

void MainView::slotView( bool reuse, bool slideShow, bool random )
{
    QStringList listOnDisk = selectedOnDisk();

    if ( listOnDisk.count() == 0 ) {
        QMessageBox::warning( this, i18n("No Images to Display"),
                              i18n("None of the selected images were available on the disk.") );
    }

    if (random)
        listOnDisk = Utilities::shuffle( listOnDisk );

    if ( listOnDisk.count() != 0 ) {

        Viewer::Viewer* viewer;
        if ( reuse && Viewer::Viewer::latest() ) {
            viewer = Viewer::Viewer::latest();
            topLevelWidget()->raise();
            setActiveWindow();
        }
        else {
            viewer = new Viewer::Viewer( "viewer" );
            connect( viewer, SIGNAL( dirty() ), this, SLOT( markDirty() ) );
        }
        viewer->show( slideShow );

        viewer->load( listOnDisk );
        viewer->raise();
    }
}

void MainView::slotSortByDateAndTime()
{
    ImageDB::instance()->sortAndMergeBackIn( selected( true /* sort with oldest first */ ) );
    showThumbNails( ImageDB::instance()->search( Browser::Browser::instance()->currentContext() ) );
    markDirty();
}


QString MainView::welcome()
{
    Dialogs::WelComeDialog dialog( this );
    dialog.exec();
    return dialog.configFileName();
}

void MainView::slotChanges()
{
    setDirty( true );
}

void MainView::closeEvent( QCloseEvent* e )
{
    bool quit = true;
    quit = slotExit();
    // If I made it here, then the user canceled
    if ( !quit )
        e->ignore();
    else
        e->accept();
}


void MainView::slotLimitToSelected()
{
    Utilities::ShowBusyCursor dummy;
    showThumbNails( selected() );
}

void MainView::setupMenuBar()
{
    // File menu
    KStdAction::save( this, SLOT( slotSave() ), actionCollection() );
    KStdAction::quit( this, SLOT( slotExit() ), actionCollection() );
    _generateHtml = new KAction( i18n("Generate HTML..."), 0, this, SLOT( slotExportToHTML() ), actionCollection(), "exportHTML" );

    new KAction( i18n( "Import..."), 0, this, SLOT( slotImport() ), actionCollection(), "import" );
    new KAction( i18n( "Export..."), 0, this, SLOT( slotExport() ), actionCollection(), "export" );


    // Go menu
    KAction* a = KStdAction::back( _browser, SLOT( back() ), actionCollection() );
    connect( _browser, SIGNAL( canGoBack( bool ) ), a, SLOT( setEnabled( bool ) ) );
    a->setEnabled( false );

    a = KStdAction::forward( _browser, SLOT( forward() ), actionCollection() );
    connect( _browser, SIGNAL( canGoForward( bool ) ), a, SLOT( setEnabled( bool ) ) );
    a->setEnabled( false );

    a = KStdAction::home( _browser, SLOT( home() ), actionCollection() );

    // The Edit menu
#ifdef CODE_FOR_OLD_CUT_AND_PASTE_IN_THUMBNAIL_VIEW
    _cut = KStdAction::cut( _thumbNailViewOLD, SLOT( slotCut() ), actionCollection() );
    _paste = KStdAction::paste( _thumbNailViewOLD, SLOT( slotPaste() ), actionCollection() );
#endif
    _selectAll = KStdAction::selectAll( _thumbnailView, SLOT( selectAll() ), actionCollection() );
    KStdAction::find( this, SLOT( slotSearch() ), actionCollection() );
    _deleteSelected = new KAction( i18n( "Delete Selected" ), QString::fromLatin1("editdelete"), Key_Delete, this, SLOT( slotDeleteSelected() ),
                                   actionCollection(), "deleteSelected" );
    new KAction( i18n("Remove Tokens"), 0, this, SLOT( slotRemoveTokens() ), actionCollection(), "removeTokens" );
    _configOneAtATime = new KAction( i18n( "Set Properties for Individual Images" ), CTRL+Key_1, this, SLOT( slotConfigureImagesOneAtATime() ),
                                     actionCollection(), "oneProp" );
    _configAllSimultaniously = new KAction( i18n( "Set Properties for Multiple Images at a Time" ), CTRL+Key_2, this, SLOT( slotConfigureAllImages() ),
                                            actionCollection(), "allProp" );

    // The Images menu
    _view = new KAction( i18n("View"), CTRL+Key_I, this, SLOT( slotView() ),
                                 actionCollection(), "viewImages" );

    _viewInNewWindow = new KAction( i18n("View (In New Window)"), 0, this, SLOT( slotViewNewWindow() ),
                                           actionCollection(), "viewImagesNewWindow" );
    _runSlideShow = new KAction( i18n("Run Slide Show"), QString::fromLatin1("video"), CTRL+Key_R, this, SLOT( slotRunSlideShow() ),
                                 actionCollection(), "runSlideShow" );
    _runRandomSlideShow = new KAction( i18n( "Run Randomized Slide Show" ), 0, this, SLOT( slotRunRandomizedSlideShow() ),
                                       actionCollection(), "runRandomizedSlideShow" );
    KToggleAction* incr = new KToggleAction( i18n("Show &Oldest Image First"), 0, this,
                                             SLOT( slotOrderIncr() ), actionCollection(), "orderIncr" );
    KToggleAction* decr = new KToggleAction( i18n("Show &Newest Image First"), 0, this,
                                             SLOT( slotOrderDecr() ), actionCollection(), "orderDecr" );
    incr->setExclusiveGroup( QString::fromLatin1( "Sort Direction") );
    decr->setExclusiveGroup(QString::fromLatin1( "Sort Direction") );
    incr->setChecked( !Settings::Settings::instance()->showNewestThumbnailFirst() );
    decr->setChecked( Settings::Settings::instance()->showNewestThumbnailFirst() );

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

    _setDefaultPos = new KAction( i18n("Lock Away All Other Images"), 0, this, SLOT( setDefaultScopePositive() ),
                                  actionCollection(), "setDefaultScopePositive" );
    _setDefaultNeg = new KAction( i18n("Lock Away Current Set of Images"), 0, this, SLOT( setDefaultScopeNegative() ),
                                  actionCollection(), "setDefaultScopeNegative" );

    // Maintenance
    new KAction( i18n("Display Images Not on Disk"), 0, this, SLOT( slotShowNotOnDisk() ), actionCollection(), "findUnavailableImages" );
    new KAction( i18n("Display Images with Incomplete Dates..."), 0, this, SLOT( slotShowImagesWithInvalidDate() ), actionCollection(), "findImagesWithInvalidDate" );
    new KAction( i18n("Recalculate Checksum"), 0, this, SLOT( slotRecalcCheckSums() ), actionCollection(), "rebuildMD5s" );
    new KAction( i18n("Rescan for Images"), 0, ImageDB::instance(), SLOT( slotRescan() ), actionCollection(), "rescan" );
#ifdef HASEXIV2
    new KAction( i18n("Read EXIF Info From Files..."), 0, this, SLOT( slotReReadExifInfo() ), actionCollection(), "reReadExifInfo" );
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
    _smallListView = new KRadioAction( i18n("Small List View"), KShortcut(), _browser, SLOT( slotSmallListView() ),
                                                    _viewMenu );
    _viewMenu->insert( _smallListView );
    _smallListView->setExclusiveGroup( QString::fromLatin1("configureview") );

    _largeListView = new KRadioAction( i18n("Large List View"), KShortcut(), _browser, SLOT( slotLargeListView() ),
                                                    _viewMenu );
    _viewMenu->insert( _largeListView );
    _largeListView->setExclusiveGroup( QString::fromLatin1("configureview") );

    _smallIconView = new KRadioAction( i18n("Small Icon View"), KShortcut(), _browser, SLOT( slotSmallIconView() ),
                                                    _viewMenu );
    _viewMenu->insert( _smallIconView );
    _smallIconView->setExclusiveGroup( QString::fromLatin1("configureview") );

    _largeIconView = new KRadioAction( i18n("Large Icon View"), KShortcut(), _browser, SLOT( slotLargeIconView() ),
                                                    _viewMenu );
    _viewMenu->insert( _largeIconView );
    _largeIconView->setExclusiveGroup( QString::fromLatin1("configureview") );


    connect( _browser, SIGNAL( currentSizeAndTypeChanged( Category::ViewSize, Category::ViewType ) ),
             this, SLOT( slotUpdateViewMenu( Category::ViewSize, Category::ViewType ) ) );
    // The help menu
    KStdAction::tipOfDay( this, SLOT(showTipOfDay()), actionCollection() );
    KToggleAction* taction = new KToggleAction( i18n("Show Tooltips on Images"), CTRL+Key_T, actionCollection(), "showToolTipOnImages" );
    connect( taction, SIGNAL( toggled( bool ) ), _thumbnailView, SLOT( showToolTipsOnImages( bool ) ) );
    new KAction( i18n("Run KPhotoAlbum Demo"), 0, this, SLOT( runDemo() ), actionCollection(), "runDemo" );
    new KAction( i18n("Answer KPhotoAlbum Survey..."), 0, this, SLOT( runSurvey() ), actionCollection(), "runSurvey" );
    new KAction( i18n("Donate Money..."), 0, this, SLOT( donateMoney() ), actionCollection(), "donate" );
    new KAction( i18n("KPhotoAlbum Feature Status"), 0, this, SLOT( showFeatures() ), actionCollection(), "features" );

    // Context menu actions
#ifdef HASEXIV2
    _showExifDialog = new KAction( i18n("Show Exif Info"), 0, this, SLOT( slotShowExifInfo() ), actionCollection(), "showExifInfo" );
#endif

#ifdef CODE_FOR_OLD_CUT_AND_PASTE_IN_THUMBNAIL_VIEW
    connect( _thumbNailViewOLD, SIGNAL( changed() ), this, SLOT( slotChanges() ) );
#endif
    createGUI( QString::fromLatin1( "kphotoalbumui.rc" ), false );
}

void MainView::slotExportToHTML()
{
    QStringList list = selectedOnDisk();
    if ( list.count() == 0 )
        list = ImageDB::instance()->currentScope( true );

    if ( ! _htmlDialog )
        _htmlDialog = new Dialogs::HTMLExportDialog( this, "htmlExportDialog" );
    _htmlDialog->exec( list );
}

void MainView::startAutoSaveTimer()
{
    int i = Settings::Settings::instance()->autoSave();
    _autoSaveTimer->stop();
    if ( i != 0 ) {
        _autoSaveTimer->start( i * 1000 * 60  );
    }
}

void MainView::slotAutoSave()
{
    if ( _autoSaveDirty ) {
        Utilities::ShowBusyCursor dummy;
        statusBar()->message(i18n("Auto saving...."));
        ImageDB::instance()->save( Settings::Settings::instance()->imageDirectory() + QString::fromLatin1(".#index.xml"), true );
        statusBar()->message(i18n("Auto saving.... Done"), 5000);
        _autoSaveDirty = false;
    }
}


void MainView::showThumbNails()
{
    reloadThumbnails(false);
    _stack->raiseWidget( _thumbnailView );
    _thumbnailView->setFocus();
    updateStates( true );
}

void MainView::showBrowser()
{
    _stack->raiseWidget( _browser );
    _browser->setFocus();
    updateStates( false );
}


void MainView::slotOptionGroupChanged()
{
    Q_ASSERT( !_annotationDialog || !_annotationDialog->isShown() );
    delete _annotationDialog;
    _annotationDialog = 0;
    setDirty( true );
}

void MainView::showTipOfDay()
{
    KTipDialog::showTip( this, QString::null, true );
}

void MainView::pathChanged( const QString& path )
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

void MainView::runDemo()
{
    KProcess* process = new KProcess;
    *process << "kphotoalbum" << "-demo";
    process->start();
}

bool MainView::load()
{
    // Let first try to find a config file.
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString configFile = QString::null;
    QString backEnd = QString::null;

    if ( args->isSet( "c" ) ) {
        configFile = args->getOption( "c" );
        if ( configFile.find( QString::fromLatin1("sql:") ) != -1 ) {
            backEnd = QString::fromLatin1( "sql" );
            configFile = configFile.mid(4);
        }
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
            MySplashScreen::instance()->hide();
            configFile = welcome();
            if ( !configFile )
                return false;
        }
    }

    if (configFile.startsWith( QString::fromLatin1( "~" ) ) )
        configFile = QDir::home().path() + QString::fromLatin1( "/" ) + configFile.mid(1);

    Settings::Settings::setup( QFileInfo( configFile ).dirPath( true ) );
    ImageDB::setup( backEnd, configFile );
    return true;
}

void MainView::contextMenuEvent( QContextMenuEvent* e )
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

        _view->plug( &menu );
        _viewInNewWindow->plug( &menu );

        ExternalPopup* externalCommands = new ExternalPopup( &menu );
        ImageInfoPtr info = ImageInfoPtr( 0 );
        QString fileName = _thumbnailView->fileNameUnderCursor();
        if ( !fileName.isNull() )
            info = ImageDB::instance()->info( fileName );

        externalCommands->populate( info, selected() );
        int id = menu.insertItem( i18n( "Invoke External Program" ), externalCommands );
        if ( info == 0 && selected().count() == 0 )
            menu.setItemEnabled( id, false );

        menu.exec( QCursor::pos() );

        delete externalCommands;
    }
    e->consume();
}

void MainView::markDirty()
{
    setDirty( true );
}

void MainView::setDirty( bool dirty )
{
    static QPixmap* dirtyPix = new QPixmap( SmallIcon( QString::fromLatin1( "3floppy_unmount" ) ) );

    if ( _dirtyIndicator ) {
        // Might not yet have been created.

        _dirtyIndicator->setFixedWidth( dirtyPix->width() );
        if ( dirty )
            _dirtyIndicator->setPixmap( *dirtyPix );
        else
            _dirtyIndicator->setPixmap( QPixmap() );
    }

    _dirty = dirty;
    _autoSaveDirty = dirty;
}

void MainView::setDefaultScopePositive()
{
    Settings::Settings::instance()->setCurrentLock( _browser->currentContext(), false );
}

void MainView::setDefaultScopeNegative()
{
    Settings::Settings::instance()->setCurrentLock( _browser->currentContext(), true );
}

void MainView::lockToDefaultScope()
{
    int i = KMessageBox::warningContinueCancel( this,
                                                i18n( "<qt><p>The password protection is only a means of allowing your little sister "
                                                      "to look in your images, without getting to those embarrassing images from "
                                                      "your last party.</p>"
                                                      "<p>In other words, anyone with access to the index.xml file can easily circumvent "
                                                      "this password.</b></p>"),
                                                i18n("Password Protection"),
                                                KStdGuiItem::cont(),
                                                QString::fromLatin1( "lockPassWordIsNotEncruption" ) );
    if ( i == KMessageBox::Cancel )
        return;

    setLocked( true );

}

void MainView::unlockFromDefaultScope()
{
    QCString passwd;
    bool OK = ( Settings::Settings::instance()->password().isEmpty() );
    while ( !OK ) {
        int code = KPasswordDialog::getPassword( passwd, i18n("Type in Password to Unlock"));
        if ( code == QDialog::Rejected )
            return;
        OK = (Settings::Settings::instance()->password() == QString(passwd));

        if ( !OK )
            KMessageBox::sorry( this, i18n("Invalid password.") );
    }
    setLocked( false );
}

void MainView::setLocked( bool locked )
{
    static QPixmap* lockedPix = new QPixmap( SmallIcon( QString::fromLatin1( "key" ) ) );
    _lockedIndicator->setFixedWidth( lockedPix->width() );

    if ( locked )
        _lockedIndicator->setPixmap( *lockedPix );
    else
        _lockedIndicator->setPixmap( QPixmap() );

    Settings::Settings::instance()->setLocked( locked );

    _lock->setEnabled( !locked );
    _unlock->setEnabled( locked );
    _setDefaultPos->setEnabled( !locked );
    _setDefaultNeg->setEnabled( !locked );
    _browser->reload();
}

void MainView::changePassword()
{
    QCString passwd;
    bool OK = ( Settings::Settings::instance()->password().isEmpty() );

    while ( !OK ) {
        int code = KPasswordDialog::getPassword( passwd, i18n("Type in Old Password"));
        if ( code == QDialog::Rejected )
            return;
        OK = (Settings::Settings::instance()->password() == QString(passwd));

        if ( !OK )
            KMessageBox::sorry( this, i18n("Invalid password.") );
    }

    int code = KPasswordDialog::getNewPassword( passwd, i18n("Type in New Password"));
    if ( code == QDialog::Accepted )
        Settings::Settings::instance()->setPassword( passwd );
}

void MainView::slotConfigureKeyBindings()
{
    Viewer::Viewer* viewer = new Viewer::Viewer( "viewer" ); // Do not show, this is only used to get a key configuration
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

    dialog->configure();

    delete dialog;
    delete viewer;
}

void MainView::slotSetFileName( const QString& fileName )
{
    statusBar()->message( fileName, 4000 );
}

void MainView::slotThumbNailSelectionChanged()
{
    QStringList selection = _thumbnailView->selection();

    _configAllSimultaniously->setEnabled(selection.count() > 1 );
    _configOneAtATime->setEnabled(selection.count() >= 1 );
    _sortByDateAndTime->setEnabled(selection.count() > 1 );
}

void MainView::reloadThumbnails(bool flushCache)
{
    _thumbnailView->reload( flushCache );
    slotThumbNailSelectionChanged();
}

void MainView::reloadThumbnailsAndFlushCache()
{
    reloadThumbnails(true);
}

void MainView::slotUpdateViewMenu( Category::ViewSize size, Category::ViewType type )
{
    if ( size == Category::Small && type == Category::ListView )
        _smallListView->setChecked( true );
    else if ( size == Category::Large && type == Category::ListView )
        _largeListView->setChecked( true );
    else if ( size == Category::Small && type == Category::IconView )
        _smallIconView->setChecked( true );
    else if ( size == Category::Large && type == Category::IconView )
        _largeIconView->setChecked( true );
}

void MainView::slotShowNotOnDisk()
{
    QStringList allImages = ImageDB::instance()->images();
    QStringList notOnDisk;
    for( QStringList::ConstIterator it = allImages.begin(); it != allImages.end(); ++it ) {
        ImageInfoPtr info = ImageDB::instance()->info(*it);
        QFileInfo fi( info->fileName() );
        if ( !fi.exists() )
            notOnDisk.append(*it);
    }

    showThumbNails( notOnDisk );
}


void MainView::donateMoney()
{
    Dialogs::DonateDialog donate( this, "Donate Money" );
    donate.exec();
}

void MainView::updateStates( bool thumbNailView )
{
#ifdef CODE_FOR_OLD_CUT_AND_PASTE_IN_THUMBNAIL_VIEW
    _cut->setEnabled( thumbNailView );
    _paste->setEnabled( thumbNailView );
#endif
    _selectAll->setEnabled( thumbNailView );
    _deleteSelected->setEnabled( thumbNailView );
    _limitToMarked->setEnabled( thumbNailView );
}

void MainView::slotRemoveAllThumbnails()
{
    Dialogs::DeleteThumbnailsDialog dialog( this );
    dialog.exec();
}

void MainView::slotRunSlideShow()
{
    slotView( true, true );
}

void MainView::slotRunRandomizedSlideShow()
{
    slotView( true, true, true );
}

MainView* MainView::theMainView()
{
    Q_ASSERT( _instance );
    return _instance;
}

void MainView::slotConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config(), QString::fromLatin1("MainWindow"));
    KEditToolbar dlg(actionCollection());
    connect(&dlg, SIGNAL( newToolbarConfig() ),
                  SLOT( slotNewToolbarConfig() ));
    dlg.exec();

}

void MainView::slotNewToolbarConfig()
{
    createGUI();
    applyMainWindowSettings(KGlobal::config(), QString::fromLatin1("MainWindow"));
}

void MainView::slotImport()
{
    ImportExport::Import::imageImport();
}

void MainView::slotExport()
{
    QStringList list = selectedOnDisk();
    if ( list.count() == 0 ) {
        KMessageBox::sorry( this, i18n("No images to export.") );
    }
    else
        ImportExport::Export::imageExport( list );
}

void MainView::slotReenableMessages()
{
    int ret = KMessageBox::questionYesNo( this, i18n("<qt><p>Really enable all messageboxes where you previously "
                                                     "checked the do-not-show-again check box?</p></qt>" ) );
    if ( ret == KMessageBox::Yes )
        KMessageBox::enableAllMessages();

}

void MainView::setupPluginMenu()
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

void MainView::loadPlugins()
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
#endif // HASKIPI
}


void MainView::plug()
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


void MainView::slotImagesChanged( const KURL::List& urls )
{
    for( KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it ) {
        ImageManager::ImageLoader::removeThumbnail( (*it).path() );
    }
    reloadThumbnails(true);
}

ImageSearchInfo MainView::currentContext()
{
    return _browser->currentContext();
}

QString MainView::currentBrowseCategory() const
{
    return _browser->currentCategory();
}

void MainView::slotSelectionChanged()
{
#ifdef HASKIPI
    _pluginInterface->slotSelectionChanged( selected().count() != 0);
#endif
}

void MainView::resizeEvent( QResizeEvent* )
{
    if ( Settings::Settings::ready() )
        Settings::Settings::instance()->setWindowGeometry( Settings::MainWindow, geometry() );
}

void MainView::moveEvent( QMoveEvent * )
{
    if ( Settings::Settings::ready() )
        Settings::Settings::instance()->setWindowGeometry( Settings::MainWindow, geometry() );
}


void MainView::slotRemoveTokens()
{
    if ( !_tokenEditor )
        _tokenEditor = new Dialogs::TokenEditor( this, "token editor" );
    _tokenEditor->show();
}

void MainView::updateDateBar( const QString& path )
{
    static QString lastPath = QString::fromLatin1("ThisStringShouldNeverBeSeenSoWeUseItAsInitialContent");
    if ( path != lastPath )
        updateDateBar();
    lastPath = path;
}

void MainView::updateDateBar()
{
    _dateBar->setImageDateCollection( ImageDB::instance()->rangeCollection() );
}


void MainView::slotShowImagesWithInvalidDate()
{
    Dialogs::InvalidDateFinder finder( this, "invaliddatefinder" );
    if ( finder.exec() == QDialog::Accepted )
        showThumbNails();
}

void MainView::showDateBarTip( const QString& msg )
{
    statusBar()->message( msg, 3000 );
}

void MainView::slotJumpToContext()
{
    QString fileName =_thumbnailView->currentItem();
    if ( !fileName.isNull() ) {
        _browser->addImageView( fileName );
   }
}

void MainView::setDateRange( const ImageDate& range )
{
    ImageDB::instance()->setDateRange( range, _dateBar->includeFuzzyCounts() );
    _browser->reload();
    reloadThumbnails(false);
}

void MainView::clearDateRange()
{
    ImageDB::instance()->clearDateRange();
    _browser->reload();
    reloadThumbnails(false);
}

void MainView::runSurvey()
{
    Survey::MySurvey survey(this);
    survey.exec();
}

void MainView::possibleRunSuvey()
{
    Survey::MySurvey survey(this);
    survey.possibleExecSurvey();
}



void MainView::showThumbNails( const QStringList& list )
{
    _thumbnailView->setImageList( list );
    _partial->setMatchCount( list.count() );
    showThumbNails();
}

void MainView::convertBackend()
{
    ImageDB::instance()->convertBackend();
}

void MainView::slotRecalcCheckSums()
{
    ImageDB::instance()->slotRecalcCheckSums( selected() );
}

void MainView::slotShowExifInfo()
{
#ifdef HASEXIV2
    QStringList items = selectedOnDisk();
    if ( !items.empty() ) {
        Exif::InfoDialog* exifDialog = new Exif::InfoDialog( items[0], this );
        exifDialog->show();
    }
#endif
}

void MainView::showFeatures()
{
    Dialogs::FeatureDialog dialog(this);
    dialog.exec();
}

void MainView::showImage( const QString& fileName )
{
    // PENDING(blackie) This code most be duplicated for Ctrl+I
    if ( !ImageInfo::imageOnDisk(fileName) ) {
        QMessageBox::warning( this, i18n("No Images to Display"),
                              i18n("The selected image was not available on disk.") );
    }
    else {
        QStringList list;
        list.append( fileName );
        Viewer::Viewer* viewer;
        if ( !Utilities::ctrlKeyDown() && Viewer::Viewer::latest() ) {
            viewer = Viewer::Viewer::latest();
            viewer->setActiveWindow();
            viewer->raise();
        }
        else {
            viewer = new Viewer::Viewer( "viewer" );
            viewer->show( false );
        }
        viewer->load( list );
    }

}

void MainView::slotBuildThumbnails()
{
    new ThumbnailView::ThumbnailBuilder( this ); // It will delete itself
}

void MainView::slotOrderIncr()
{
    _thumbnailView->setSortDirection( ThumbnailView::OldestFirst );
}

void MainView::slotOrderDecr()
{
    _thumbnailView->setSortDirection( ThumbnailView::NewestFirst );
}

#include "mainview.moc"
