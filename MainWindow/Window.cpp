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

#include "Window.h"
#include "StatisticsDialog.h"
#include "Settings/SettingsDialog.h"
#include <qapplication.h>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QLabel>
#include <QPixmap>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <Q3Frame>
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

#include <q3widgetstack.h>
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
#include <KProcess>
#include "DeleteDialog.h"
#include <ksimpleconfig.h>
#include <kcmdlineargs.h>
#include <q3popupmenu.h>
#include <kiconloader.h>
#include <kpassworddialog.h>
#include <KShortcutsDialog>
#include <kdebug.h>
#include "ExternalPopup.h"
#include <kstandardaction.h>
#include <kedittoolbar.h>
#include "ImportExport/Export.h"
#include "ImportExport/Import.h"
#include <config-kpa-kipi.h>
#ifdef HASKIPI
#  include "Plugins/Interface.h"
#  include <libkipi/pluginloader.h>
#  include <libkipi/plugin.h>
#endif
#include <config-kpa-exiv2.h>
#ifdef HAVE_EXIV2
#  include "Exif/ReReadDialog.h"
#endif
#include "ImageManager/ImageLoader.h"
#include "SplashScreen.h"
#include <qobject.h>
#include "SearchBar.h"
#include "TokenEditor.h"
#include "DB/CategoryCollection.h"
#include <qlayout.h>
#include "DateBar/DateBarWidget.h"
#include "DB/ImageDateCollection.h"
#include "InvalidDateFinder.h"
#include "DB/ImageInfo.h"
#include "DB/ResultId.h"
#include "Survey/MySurvey.h"
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_EXIV2
#  include "Exif/Info.h"
#  include "Exif/InfoDialog.h"
#  include "Exif/Database.h"
#endif

#include "FeatureDialog.h"
#include "ImageManager/ImageRequest.h"
#include "ImageManager/Manager.h"

#include <config-kpa-sqldb.h>
#ifdef SQLDB_SUPPORT
#  include "SQLDB/Database.h"
#  include "SQLDB/ConfigFileHandler.h"
#  include "SQLDB/QueryErrors.h"
#  include <kprogressdialog.h>
#endif
#include <krun.h>
#include <kglobal.h>
#include <kvbox.h>
#include "DirtyIndicator.h"
#include "Utilities/ShowBusyCursor.h"
#include <KToggleAction>
#include <KActionMenu>
#include <KHBox>
#include <K3URLDrag>
#include <qclipboard.h>
#include <stdexcept>
#include <KInputDialog>
#include "DB/Result.h"

MainWindow::Window* MainWindow::Window::_instance = 0;

MainWindow::Window::Window( QWidget* parent )
    :KXmlGuiWindow( parent ),
    _annotationDialog(0),
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

    QWidget* top = new QWidget( this );
    QVBoxLayout* lay = new QVBoxLayout( top );
    setCentralWidget( top );

    _stack = new Q3WidgetStack( top, "_stack" );
    lay->addWidget( _stack, 1 );

    _dateBar = new DateBar::DateBarWidget( top );
    lay->addWidget( _dateBar );

    Q3Frame* line = new Q3Frame( top );
    line->setFrameStyle( Q3Frame::HLine | Q3Frame::Plain );
    line->setLineWidth(1);
    lay->addWidget( line );

    _browser = new Browser::BrowserWidget( _stack );
    connect( _browser, SIGNAL( showingOverview() ), this, SLOT( showBrowser() ) );
    connect( _browser, SIGNAL( pathChanged( const QString& ) ), this, SLOT( pathChanged( const QString& ) ) );
    connect( _browser, SIGNAL( pathChanged( const QString& ) ), this, SLOT( updateDateBar( const QString& ) ) );
    _thumbnailView = new ThumbnailView::ThumbnailWidget( _stack );
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
    connect( _browser, SIGNAL( showsContentView( bool ) ), bar, SLOT( setLineEditEnabled( bool ) ) );

    setupStatusBar();

    // Misc
    _autoSaveTimer = new QTimer( this );
    connect( _autoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
    startAutoSaveTimer();

    connect( DB::ImageDB::instance(), SIGNAL( totalChanged( uint ) ), this, SLOT( updateDateBar() ) );
    connect( DB::ImageDB::instance(), SIGNAL( totalChanged( uint ) ), _browser, SLOT( home() ) );
    connect( _browser, SIGNAL( showingOverview() ), _partial, SLOT( showingOverview() ) );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL( categoryCollectionChanged() ), this, SLOT( slotOptionGroupChanged() ) );
    connect( _thumbnailView, SIGNAL( selectionChanged() ), this, SLOT( slotThumbNailSelectionChanged() ) );

    connect( _dirtyIndicator, SIGNAL( dirty() ), _thumbnailView, SLOT(repaintScreen() ) );

    statusBar()->showMessage(i18n("Welcome to KPhotoAlbum"), 5000 );

    QTimer::singleShot( 0, this, SLOT( delayedInit() ) );
    slotThumbNailSelectionChanged();
}

MainWindow::Window::~Window()
{
    DB::ImageDB::deleteInstance();
#ifdef HAVE_EXIV2
    Exif::Database::deleteInstance();
#endif
}

void MainWindow::Window::delayedInit()
{
    SplashScreen* splash = SplashScreen::instance();
    setupPluginMenu();

    if ( Settings::SettingsData::instance()->searchForImagesOnStartup() ) {
        splash->message( i18n("Searching for New Files") );
        qApp->processEvents();
        QTimer* timer = new QTimer( this );
        connect( timer, SIGNAL( timeout() ), DB::ImageDB::instance(), SLOT( slotRescan() ) );
        timer->setSingleShot( true );
        timer->start( 0 );
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
        ImportExport::Import::imageImport( KCmdLineArgs::makeURL( args->getOption("import").toLocal8Bit() ) );
    }
    else {
        // I need to postpone this otherwise the tip dialog will not get focus on start up
        KTipDialog::showTip( this );

        possibleRunSuvey();
    }

#ifdef HAVE_EXIV2
    Exif::Database* exifDB = Exif::Database::instance(); // Load the database
    if ( exifDB->isAvailable() && !exifDB->isOpen() ) {
        KMessageBox::sorry( this, i18n("EXIF database cannot be opened. Check that the image root directory is writable.") );
    }
#endif
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
                                                    KStandardGuiItem::yes(), KStandardGuiItem::no(), KStandardGuiItem::cancel(),
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

void MainWindow::Window::slotCreateImageStack()
{
    QStringList list = selected();
    if ( list.size() < 2 ) {
        // it doesn't make sense to make a stack from one image, does it?
        return;
    }

    bool ok = DB::ImageDB::instance()->stack( list );
    if ( !ok ) {
        if ( KMessageBox::questionYesNo( this,
                    i18n("Some of the selected images already belong to some stack. "
                        "Do you want to remove them from their stacks and create a "
                        "completely new one?"), i18n("Stacking Error")) == KMessageBox::Yes ) {
            DB::ImageDB::instance()->unstack( list );
            if ( ! DB::ImageDB::instance()->stack( list ) ) {
                KMessageBox::sorry( this,
                        i18n("Something fishy happened, stack creation failed, sorry."),
                        i18n("Stacking Error"));
                return;
            }
        } else {
            return;
        }
    }

    // FIXME: here we should invoke a fancy dialog for user's pleasure

    _thumbnailView->updateDisplayModel();
}

void MainWindow::Window::slotUnStackImages()
{
    QStringList list = selected();
    if ( list.isEmpty() )
        return;

    DB::ImageDB::instance()->unstack( list );
    _thumbnailView->updateDisplayModel();
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
        KMessageBox::sorry( this, i18n("No item is selected."), i18n("No Selection") );
    }
    else {
        DB::ImageInfoList images;
        for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            images.append( DB::ImageDB::instance()->info( *it, DB::AbsolutePath ) );
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

    _annotationDialog = new AnnotationDialog::Dialog( 0 );
}

void MainWindow::Window::slotSave()
{
    Utilities::ShowBusyCursor dummy;
    statusBar()->showMessage(i18n("Saving..."), 5000 );
    DB::ImageDB::instance()->save( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("index.xml"), false );
    _dirtyIndicator->saved();
    QDir().remove( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1(".#index.xml") );
    statusBar()->showMessage(i18n("Saving... Done"), 5000 );
}

void MainWindow::Window::slotDeleteSelected()
{
#ifdef KDAB_TEMPORARILY_REMOVED //QWERTY
    if ( ! _deleteDialog )
        _deleteDialog = new DeleteDialog( this );
    if ( _deleteDialog->exec( selected() ) != QDialog::Accepted )
        return;

    Utilities::ShowBusyCursor dummy;
    DirtyIndicator::markDirty();

    QStringList images = _thumbnailView->imageList( ThumbnailView::ThumbnailWidget::SortedOrder );
    StringSet allImages( DB::ImageDB::instance()->CONVERT( DB::ImageDB::instance()->images()) );
    QStringList newSet;
    for( QStringList::Iterator it = images.begin(); it != images.end(); ++it ) {
        if ( allImages.contains( *it ) )
            newSet.append(*it);
    }
    showThumbNails( newSet );
#else // KDAB_TEMPORARILY_REMOVED
    qFatal("Code commented out in MainWindow::Window::slotDeleteSelected");
#endif //KDAB_TEMPORARILY_REMOVED
}

void MainWindow::Window::slotCopySelectedURLs()
{
    const QStringList& sel = selectedOnDisk();
    KUrl::List urls;

    for (QStringList::const_iterator it = sel.begin(); it != sel.end(); ++it) {
        urls.append( KUrl( *it ) );
    }

    QApplication::clipboard()->setData( new K3URLDrag( urls ) );
}

void MainWindow::Window::slotReReadExifInfo()
{
#ifdef HAVE_EXIV2
    QStringList files = selectedOnDisk();
    static Exif::ReReadDialog* dialog = 0;
    if ( ! dialog )
        dialog = new Exif::ReReadDialog( this );
    if ( dialog->exec( files ) == QDialog::Accepted )
            DirtyIndicator::markDirty();
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
        return DB::ImageDB::instance()->CONVERT( DB::ImageDB::instance()->currentScope( true ) );

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
        seek = files.indexOf(fileName);
    }

    if ( !files.count() )
        files = DB::ImageDB::instance()->CONVERT( DB::ImageDB::instance()->currentScope( false ));

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
        viewer->activateWindow();
    }
    else
        viewer = new Viewer::ViewerWidget;

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
        e->setAccepted(true);
}


void MainWindow::Window::slotLimitToSelected()
{
#ifdef KDAB_TEMPORARILY_REMOVED //QWERTY
    Utilities::ShowBusyCursor dummy;
    showThumbNails( selected() );
#else // KDAB_TEMPORARILY_REMOVED
    qFatal("Code commented out in MainWindow::Window::slotLimitToSelected");
#endif //KDAB_TEMPORARILY_REMOVED
}

void MainWindow::Window::setupMenuBar()
{
// File menu
    KStandardAction::save( this, SLOT( slotSave() ), actionCollection() );
    KStandardAction::quit( this, SLOT( slotExit() ), actionCollection() );
    _generateHtml = actionCollection()->addAction( QString::fromLatin1("exportHTML") );
    _generateHtml->setText( i18n("Generate HTML...") );
    connect( _generateHtml, SIGNAL(triggered()), this, SLOT( slotExportToHTML() ) );

    KAction* a = actionCollection()->addAction( QString::fromLatin1("import"), this, SLOT( slotImport() ) );
    a->setText( i18n( "Import...") );

    a = actionCollection()->addAction( QString::fromLatin1("export"), this, SLOT( slotExport() ) );
    a->setText( i18n( "Export/Copy Images...") );


    // Go menu
    a = KStandardAction::back( _browser, SLOT( back() ), actionCollection() );
    connect( _browser, SIGNAL( canGoBack( bool ) ), a, SLOT( setEnabled( bool ) ) );
    a->setEnabled( false );

    a = KStandardAction::forward( _browser, SLOT( forward() ), actionCollection() );
    connect( _browser, SIGNAL( canGoForward( bool ) ), a, SLOT( setEnabled( bool ) ) );
    a->setEnabled( false );

    a = KStandardAction::home( _browser, SLOT( home() ), actionCollection() );

    a = KStandardAction::redisplay( _browser, SLOT( go() ), actionCollection() );

    // The Edit menu
#ifdef CODE_FOR_OLD_CUT_AND_PASTE_IN_THUMBNAIL_VIEW
    _cut = KStandardAction::cut( _thumbNailViewOLD, SLOT( slotCut() ), actionCollection() );
    _paste = KStandardAction::paste( _thumbNailViewOLD, SLOT( slotPaste() ), actionCollection() );
#endif
    KStandardAction::copy( this, SLOT( slotCopySelectedURLs() ), actionCollection() );
    _selectAll = KStandardAction::selectAll( _thumbnailView, SLOT( selectAll() ), actionCollection() );
    KStandardAction::find( this, SLOT( slotSearch() ), actionCollection() );

    _deleteSelected = actionCollection()->addAction(QString::fromLatin1("deleteSelected"));
    _deleteSelected->setText( i18n( "Delete Selected" ) );
    _deleteSelected->setIcon( KIcon( QString::fromLatin1("edit-delete") ) );
    _deleteSelected->setShortcut( Qt::Key_Delete );
    connect( _deleteSelected, SIGNAL( triggered() ), this, SLOT( slotDeleteSelected() ) );

    a = actionCollection()->addAction(QString::fromLatin1("removeTokens"), this, SLOT( slotRemoveTokens() ));
    a->setText( i18n("Remove Tokens") );

    a = actionCollection()->addAction(QString::fromLatin1("showListOfFiles"), this, SLOT( slotShowListOfFiles() ));
    a->setText( i18n("Open List of Files...")) ;


    _configOneAtATime = actionCollection()->addAction( QString::fromLatin1("oneProp"), this, SLOT( slotConfigureImagesOneAtATime() ) );
    _configOneAtATime->setText( i18n( "Annotate Individual Items" ) );
    _configOneAtATime->setShortcut(  Qt::CTRL+Qt::Key_1 );

    _configAllSimultaniously = actionCollection()->addAction( QString::fromLatin1("allProp"), this, SLOT( slotConfigureAllImages() ) );
    _configAllSimultaniously->setText( i18n( "Annotate Multiple Items at a Time" ) );
    _configAllSimultaniously->setShortcut(  Qt::CTRL+Qt::Key_2 );

    _createImageStack = actionCollection()->addAction( QString::fromLatin1("createImageStack"), this, SLOT( slotCreateImageStack() ) );
    _createImageStack->setText( i18n("Merge Images into a Stack") );
    _createImageStack->setShortcut( Qt::CTRL + Qt::Key_3 );

    _unStackImages = actionCollection()->addAction( QString::fromLatin1("unStackImages"), this, SLOT( slotUnStackImages() ) );
    _unStackImages->setText( i18n("Remove Images from Stack") );

    _rotLeft = actionCollection()->addAction( QString::fromLatin1("rotateLeft"), this, SLOT( slotRotateSelectedLeft() ) );
    _rotLeft->setText( i18n( "Rotate counterclockwise" ) );
    _rotLeft->setShortcut(  0 );


    _rotRight = actionCollection()->addAction( QString::fromLatin1("rotateRight"), this, SLOT( slotRotateSelectedRight() ) );
    _rotRight->setText( i18n( "Rotate clockwise" ) );


    // The Images menu
    _view = actionCollection()->addAction( QString::fromLatin1("viewImages"), this, SLOT( slotView() ) );
    _view->setText( i18n("View") );
    _view->setShortcut(  Qt::CTRL+Qt::Key_I );

    _viewInNewWindow = actionCollection()->addAction( QString::fromLatin1("viewImagesNewWindow"), this, SLOT( slotViewNewWindow() ) );
    _viewInNewWindow->setText( i18n("View (In New Window)") );

    _runSlideShow = actionCollection()->addAction( QString::fromLatin1("runSlideShow"), this, SLOT( slotRunSlideShow() ) );
    _runSlideShow->setText( i18n("Run Slide Show") );
    _runSlideShow->setIcon( KIcon( QString::fromLatin1("view-presentation") ) );
    _runSlideShow->setShortcut( Qt::CTRL+Qt::Key_R );

    _runRandomSlideShow = actionCollection()->addAction( QString::fromLatin1("runRandomizedSlideShow"), this, SLOT( slotRunRandomizedSlideShow() ) );
    _runRandomSlideShow->setText( i18n( "Run Randomized Slide Show" ) );

    a = actionCollection()->addAction( QString::fromLatin1("collapseAllStacks"),
                                       _thumbnailView, SLOT( collapseAllStacks() ) );
    connect(_thumbnailView, SIGNAL( collapseAllStacksEnabled(bool) ), a, SLOT( setEnabled(bool) ));
    a->setEnabled(false);
    a->setText( i18n("Collapse all stacks" ));

    QActionGroup* grp = new QActionGroup( this );

    a = actionCollection()->add<KToggleAction>( QString::fromLatin1("orderIncr"), this, SLOT( slotOrderIncr() ) );
    a->setText( i18n("Show &Oldest First") ) ;
    a->setActionGroup(grp);
    a->setChecked( !Settings::SettingsData::instance()->showNewestThumbnailFirst() );

    a = actionCollection()->add<KToggleAction>( QString::fromLatin1("orderDecr"), this, SLOT( slotOrderDecr() ) );
    a->setText( i18n("Show &Newest First") );
    a->setActionGroup(grp);
    a->setChecked( Settings::SettingsData::instance()->showNewestThumbnailFirst() );

    _sortByDateAndTime = actionCollection()->addAction( QString::fromLatin1("sortImages"), this, SLOT( slotSortByDateAndTime() ) );
    _sortByDateAndTime->setText( i18n("Sort Selected by Date && Time") );

    _limitToMarked = actionCollection()->addAction( QString::fromLatin1("limitToMarked"), this, SLOT( slotLimitToSelected() ) );
    _limitToMarked->setText( i18n("Limit View to Marked") );

    _jumpToContext = actionCollection()->addAction( QString::fromLatin1("jumpToContext"), this, SLOT( slotJumpToContext() ) );
    _jumpToContext->setText( i18n("Jump to Context") );
    _jumpToContext->setShortcut(  Qt::CTRL+Qt::Key_J );
    _jumpToContext->setIcon( KIcon( QString::fromLatin1( "kphotoalbum" ) ) ); // icon suggestion: go-jump (don't know the exact meaning though, so I didn't replace it right away

    _lock = actionCollection()->addAction( QString::fromLatin1("lockToDefaultScope"), this, SLOT( lockToDefaultScope() ) );
    _lock->setText( i18n("Lock Images") );

    _unlock = actionCollection()->addAction( QString::fromLatin1("unlockFromDefaultScope"), this, SLOT( unlockFromDefaultScope() ) );
    _unlock->setText( i18n("Unlock") );

    a = actionCollection()->addAction( QString::fromLatin1("changeScopePasswd"), this, SLOT( changePassword() ) );
    a->setText( i18n("Change Password...") );
    a->setShortcut( 0 );

    _setDefaultPos = actionCollection()->addAction( QString::fromLatin1("setDefaultScopePositive"), this, SLOT( setDefaultScopePositive() ) );
    _setDefaultPos->setText( i18n("Lock Away All Other Items") );

    _setDefaultNeg = actionCollection()->addAction( QString::fromLatin1("setDefaultScopeNegative"), this, SLOT( setDefaultScopeNegative() ) );
    _setDefaultNeg->setText( i18n("Lock Away Current Set of Items") );

    // Maintenance
    a = actionCollection()->addAction( QString::fromLatin1("findUnavailableImages"), this, SLOT( slotShowNotOnDisk() ) );
    a->setText( i18n("Display Images and Videos Not on Disk") );

    a = actionCollection()->addAction( QString::fromLatin1("findImagesWithInvalidDate"), this, SLOT( slotShowImagesWithInvalidDate() ) );
    a->setText( i18n("Display Images and Videos with Incomplete Dates...") );

    a = actionCollection()->addAction( QString::fromLatin1("findImagesWithChangedMD5Sum"), this, SLOT( slotShowImagesWithChangedMD5Sum() ) );
    a->setText( i18n("Display Images and Videos with Changed MD5 Sum") );

    a = actionCollection()->addAction( QString::fromLatin1("rebuildMD5s"), this, SLOT( slotRecalcCheckSums() ) );
    a->setText( i18n("Recalculate Checksum") );

    a = actionCollection()->addAction( QString::fromLatin1("rescan"), DB::ImageDB::instance(), SLOT( slotRescan() ) );
    a->setText( i18n("Rescan for Images and Videos") );

#ifdef HAVE_EXIV2
    a = actionCollection()->addAction( QString::fromLatin1( "recreateExifDB" ), this, SLOT( slotRecreateExifDB() ) );
    a->setText( i18n("Recreate Exif Search Database") );

    a = actionCollection()->addAction( QString::fromLatin1("reReadExifInfo"), this, SLOT( slotReReadExifInfo() ) );
    a->setText( i18n("Read EXIF Info From Files...") );
#endif

#ifdef SQLDB_SUPPORT
    a = actionCollection()->addAction( QString::fromLatin1("convertBackend"), this, SLOT( convertBackend() ) );
    a->setText( i18n("Convert Backend...(Experimental!)" ) );
#endif


    a = actionCollection()->addAction( QString::fromLatin1("buildThumbs"), this, SLOT( slotBuildThumbnails() ) );
    a->setText( i18n("Build Thumbnails") );

    a = actionCollection()->addAction( QString::fromLatin1("statistics"), this, SLOT( slotStatistics() ) );
    a->setText( i18n("Statistics") );


    // Settings
    KStandardAction::preferences( this, SLOT( slotOptions() ), actionCollection() );
    KStandardAction::keyBindings( this, SLOT( slotConfigureKeyBindings() ), actionCollection() );
    KStandardAction::configureToolbars( this, SLOT( slotConfigureToolbars() ), actionCollection() );

    a = actionCollection()->addAction( QString::fromLatin1("readdAllMessages"), this, SLOT( slotReenableMessages() ) );
    a->setText( i18n("Enable All Messages") );

    _viewMenu = actionCollection()->add<KActionMenu>( QString::fromLatin1("configureView") );
    _viewMenu->setText( i18n("Configure View") );

    _viewMenu->setIcon( KIcon( QString::fromLatin1( "view-list-details" ) ) );
    _viewMenu->setDelayed( false );
    connect( _browser, SIGNAL( showsContentView( bool ) ), _viewMenu, SLOT( setEnabled( bool ) ) );

    QActionGroup* viewGrp = new QActionGroup( this );
    viewGrp->setExclusive( true );

    _smallListView = actionCollection()->add<KToggleAction>( QString::fromLatin1("smallListView"), _browser, SLOT( slotSmallListView() ) );
    _smallListView->setText( i18n("List View") );
    _viewMenu->addAction( _smallListView );
    _smallListView->setActionGroup( viewGrp );

    _largeListView = actionCollection()->add<KToggleAction>( QString::fromLatin1("largelistview"), _browser, SLOT( slotLargeListView() ) );
    _largeListView->setText( i18n("List View with Custom Icons") );
    _viewMenu->addAction( _largeListView );
    _largeListView->setActionGroup( viewGrp );

    _smallIconView = actionCollection()->add<KToggleAction>( QString::fromLatin1("smalliconview"),  _browser, SLOT( slotSmallIconView() ) );
    _smallIconView->setText( i18n("Icon View") );
    _viewMenu->addAction( _smallIconView );
    _smallIconView->setActionGroup( viewGrp );

    _largeIconView = actionCollection()->add<KToggleAction>(  QString::fromLatin1("largeiconview"), _browser, SLOT( slotLargeIconView() ) );
    _largeIconView->setText( i18n("Icon View with Custom Icons") );
    _viewMenu->addAction( _largeIconView );
    _largeIconView->setActionGroup( viewGrp );

    connect( _browser, SIGNAL( currentViewTypeChanged( DB::Category::ViewType ) ),
             this, SLOT( slotUpdateViewMenu( DB::Category::ViewType ) ) );
    // The help menu
    KStandardAction::tipOfDay( this, SLOT(showTipOfDay()), actionCollection() );

    a = actionCollection()->add<KToggleAction>( QString::fromLatin1("showToolTipOnImages") );
    a->setText( i18n("Show Tooltips in Thumbnails Window") );
    a->setShortcut( Qt::CTRL+Qt::Key_T );
    connect( a, SIGNAL(toggled(bool)), _thumbnailView, SLOT( showToolTipsOnImages( bool ) ) );


    a = actionCollection()->addAction( QString::fromLatin1("runDemo"), this, SLOT( runDemo() ) );
    a->setText( i18n("Run KPhotoAlbum Demo") );

    a = actionCollection()->addAction( QString::fromLatin1("runSurvey"), this, SLOT( runSurvey() ) );
    a->setText( i18n("Answer KPhotoAlbum Survey...") );

    a = actionCollection()->addAction( QString::fromLatin1("features"), this, SLOT( showFeatures() ) );
    a->setText( i18n("KPhotoAlbum Feature Status") );

    a = actionCollection()->addAction( QString::fromLatin1("showVideo"), this, SLOT(showVideos()) );
    a->setText( i18n( "Show Demo Videos") );

    // Context menu actions
#ifdef HAVE_EXIV2
    _showExifDialog = actionCollection()->addAction( QString::fromLatin1("showExifInfo"), this, SLOT( slotShowExifInfo() ) );
    _showExifDialog->setText( i18n("Show Exif Info") );
#endif
    _recreateThumbnails = actionCollection()->addAction( QString::fromLatin1("recreateThumbnails"), this, SLOT( slotRecreateThumbnail() ) );
    _recreateThumbnails->setText( i18n("Recreate Selected Thumbnails") );

#ifdef CODE_FOR_OLD_CUT_AND_PASTE_IN_THUMBNAIL_VIEW
    connect( _thumbNailViewOLD, SIGNAL( changed() ), this, SLOT( slotChanges() ) );
#endif
    createGUI( QString::fromLatin1( "kphotoalbumui.rc" ) );
}

void MainWindow::Window::slotExportToHTML()
{
    if ( ! _htmlDialog )
        _htmlDialog = new HTMLGenerator::HTMLDialog( this );
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
        statusBar()->showMessage(i18n("Auto saving...."));
        DB::ImageDB::instance()->save( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1(".#index.xml"), true );
        statusBar()->showMessage(i18n("Auto saving.... Done"), 5000);
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
    // FIXME: What if annotation dialog is open? (if that's possible)
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
    QString text = path;

    if ( text.length() > 80 )
        text = text.left(80) + QString::fromLatin1( "..." );

    _pathIndicator->setText( text );
}

void MainWindow::Window::runDemo()
{
    KProcess* process = new KProcess;
    *process << "kphotoalbum" << "-demo";
    process->startDetached();
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
        KConfigGroup config = KGlobal::config()->group(QString());
        if ( config.hasKey( QString::fromLatin1("configfile") ) ) {
            configFile = config.readEntry<QString>( QString::fromLatin1("configfile"), QString() );
            if ( !QFileInfo( configFile ).exists() )
                showWelcome = true;
        }
        else
            showWelcome = true;

        if ( showWelcome ) {
            SplashScreen::instance()->hide();
            configFile = welcome();
        }
    }
    if ( configFile.isNull() )
        return false;

    if (configFile.startsWith( QString::fromLatin1( "~" ) ) )
        configFile = QDir::home().path() + QString::fromLatin1( "/" ) + configFile.mid(1);

    Settings::SettingsData::setup( QFileInfo( configFile ).absolutePath() );

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
        KConfigGroup config = KGlobal::config()->group(QString::fromLatin1("SQLDB"));
        try {
            SQLDB::DatabaseAddress address = SQLDB::readConnectionParameters(config);

            // Initialize SQLDB with the paramaters
            DB::ImageDB::setupSQLDB(address);
            return true;
        }
        catch (SQLDB::Error& e){
            KMessageBox::error(this, i18n("SQL backend initialization failed, "
                                          "because following error occurred:\n%1",e.whatAsQString()));
        }
#else
        KMessageBox::error(this, i18n("SQL database support is not compiled in."));
#endif
    }
    else if ( backEnd == QString::fromLatin1("xml") );
    else {
        KMessageBox::error(this, i18n("Invalid database backend: %1",backEnd));
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
        Q3PopupMenu menu( this, "context popup menu");
        menu.addAction( _configOneAtATime );
        menu.addAction( _configAllSimultaniously );
        menu.addAction( _createImageStack );
        menu.addAction( _unStackImages );
        menu.addSeparator();
        menu.addAction( _runSlideShow );
        menu.addAction(_runRandomSlideShow );
#ifdef HAVE_EXIV2
        menu.addAction( _showExifDialog);
#endif

        menu.addSeparator();
        menu.addAction(_rotLeft);
        menu.addAction(_rotRight);
        menu.addAction(_recreateThumbnails);
        menu.addSeparator();

        menu.addAction(_view);
        menu.addAction(_viewInNewWindow);

        ExternalPopup* externalCommands = new ExternalPopup( &menu );
        DB::ImageInfoPtr info = DB::ImageInfoPtr( 0 );
        QString fileName = _thumbnailView->fileNameUnderCursor();
        if ( !fileName.isNull() )
            info = DB::ImageDB::instance()->info( fileName, DB::AbsolutePath );

        externalCommands->populate( info, selected() );
        QAction* action = menu.addMenu( externalCommands );
        if ( info.isNull() && selected().count() == 0 )
            action->setEnabled( false );

        menu.exec( QCursor::pos() );

        delete externalCommands;
    }
    e->setAccepted(true);
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
                                                KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                QString::fromLatin1( "lockPassWordIsNotEncruption" ) );
    if ( i == KMessageBox::Cancel )
        return;

    setLocked( true, false );
}

void MainWindow::Window::unlockFromDefaultScope()
{
    bool OK = ( Settings::SettingsData::instance()->password().isEmpty() );
    while ( !OK ) {
        KPasswordDialog dialog( this );
        dialog.setPrompt( i18n("Type in Password to Unlock") );
        const int code = dialog.exec();
        if ( code == QDialog::Rejected )
            return;
        const QString passwd = dialog.password();

        OK = (Settings::SettingsData::instance()->password() == passwd);

        if ( !OK )
            KMessageBox::sorry( this, i18n("Invalid password.") );
    }
    setLocked( false, false );
}

void MainWindow::Window::setLocked( bool locked, bool force )
{
    static QPixmap* lockedPix = new QPixmap( SmallIcon( QString::fromLatin1( "object-locked" ) ) );
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
    bool OK = ( Settings::SettingsData::instance()->password().isEmpty() );

    KPasswordDialog dialog;

    while ( !OK ) {
        dialog.setPrompt( i18n("Type in Old Password") );
        const int code = dialog.exec();
        if ( code == QDialog::Rejected )
            return;
        const QString passwd = dialog.password();

        OK = (Settings::SettingsData::instance()->password() == QString(passwd));

        if ( !OK )
            KMessageBox::sorry( this, i18n("Invalid password.") );
    }

    dialog.setPrompt( i18n("Type in New Password") );
    const int code = dialog.exec();
    if ( code == QDialog::Accepted )
        Settings::SettingsData::instance()->setPassword( dialog.password() );
}

void MainWindow::Window::slotConfigureKeyBindings()
{
    Viewer::ViewerWidget* viewer = new Viewer::ViewerWidget; // Do not show, this is only used to get a key configuration
    KShortcutsDialog* dialog = new KShortcutsDialog();
    dialog->addCollection( actionCollection(), i18n( "General" ) );
    dialog->addCollection( viewer->actions(), i18n("Viewer") );

#ifdef HASKIPI
    loadPlugins();
    KIPI::PluginLoader::PluginList list = _pluginLoader->pluginList();
    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin(); it != list.end(); ++it ) {
        KIPI::Plugin* plugin = (*it)->plugin();
        if ( plugin )
            dialog->addCollection( plugin->actionCollection(), (*it)->comment() );
    }
#endif

    createAnnotationDialog();
    dialog->addCollection( _annotationDialog->actions(), i18n("Annotation Dialog" ) );

    dialog->configure();

    delete dialog;
    delete viewer;
}

void MainWindow::Window::slotSetFileName( const QString& fileName )
{
    statusBar()->showMessage( fileName, 4000 );
}

void MainWindow::Window::slotThumbNailSelectionChanged()
{
    QStringList selection = _thumbnailView->selection();

    _configAllSimultaniously->setEnabled(selection.count() > 1 );
    _configOneAtATime->setEnabled(selection.count() >= 1 );
    _createImageStack->setEnabled( selection.count() > 1 );
    _unStackImages->setEnabled( selection.count() >= 1 );
    _sortByDateAndTime->setEnabled(selection.count() > 1 );
    _recreateThumbnails->setEnabled( selection.count() >= 1 );
    _rotLeft->setEnabled( selection.count() >= 1 );
    _rotRight->setEnabled( selection.count() >= 1 );
}

void MainWindow::Window::rotateSelected( int angle )
{
    QStringList list = selected();
    if ( list.count() == 0 )  {
        KMessageBox::sorry( this, i18n("No item is selected."), i18n("No Selection") );
    } else {
        for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
            DB::ImageDB::instance()->info( *it, DB::AbsolutePath )->rotate( angle );
        }
        _dirtyIndicator->markDirty();
        reloadThumbnailsAndFlushCache();
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
    DB::ResultPtr allImages = DB::ImageDB::instance()->images();
    DB::Result* notOnDisk = new DB::Result;
    for( DB::Result::ConstIterator it = allImages->begin(); it != allImages->end(); ++it ) {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info(*it);
        QFileInfo fi( info->fileName() );
        if ( !fi.exists() )
            notOnDisk->append(*it);
    }

    showThumbNails( DB::ResultPtr(notOnDisk) );
}


void MainWindow::Window::slotShowImagesWithChangedMD5Sum()
{
#ifdef KDAB_TEMPORARILY_REMOVED//QWERTY
    Utilities::ShowBusyCursor dummy;
    StringSet changed = DB::ImageDB::instance()->imagesWithMD5Changed();
    showThumbNails( changed.toList() );
#else // KDAB_TEMPORARILY_REMOVED
    qFatal("Code commented out in MainWindow::Window::slotShowImagesWithChangedMD5Sum");
#endif //KDAB_TEMPORARILY_REMOVED
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
    KConfigGroup group = KGlobal::config()->group( QString::fromLatin1("MainWindow") );
    saveMainWindowSettings( group );
    KEditToolBar dlg(actionCollection());
    connect(&dlg, SIGNAL( newToolbarConfig() ),
                  SLOT( slotNewToolbarConfig() ));
    dlg.exec();

}

void MainWindow::Window::slotNewToolbarConfig()
{
    createGUI();
    KConfigGroup group = KGlobal::config()->group( QString::fromLatin1("MainWindow") );
    applyMainWindowSettings(group);
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
    QMenu* menu = findChild<QMenu*>( QString::fromLatin1("plugins") );
    Q_ASSERT( menu );

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
    connect( _pluginInterface, SIGNAL( imagesChanged( const KUrl::List& ) ), this, SLOT( slotImagesChanged( const KUrl::List& ) ) );

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

    QList<QAction*> importActions;
    QList<QAction*> exportActions;
    QList<QAction*> imageActions;
    QList<QAction*> toolsActions;
    QList<QAction*> batchActions;

    KIPI::PluginLoader::PluginList list = _pluginLoader->pluginList();
    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin(); it != list.end(); ++it ) {
        KIPI::Plugin* plugin = (*it)->plugin();
        if ( !plugin || !(*it)->shouldLoad() )
            continue;

        plugin->setup( this );

        QList<KAction*> actions = plugin->actions();
        for( QList<KAction*>::Iterator it = actions.begin(); it != actions.end(); ++it ) {
            KIPI::Category category = plugin->category( *it );
            if (  category == KIPI::ImagesPlugin ||  category == KIPI::CollectionsPlugin )
                imageActions.append( *it );

            else if ( category == KIPI::ImportPlugin )
                importActions.append( *it );

            else if ( category == KIPI::ExportPlugin )
                exportActions.append( *it );

            else if ( category == KIPI::ToolsPlugin )
                toolsActions.append( *it );

            else if ( category == KIPI::BatchPlugin )
                batchActions.append( *it );

            else {
                kDebug() << "Unknow category\n";
            }
        }
#ifdef TEMPORARILY_REMOVED
        plugin->actionCollection()->readShortcutSettings();
#else
        kDebug() << "TEMPORILY REMOVED " << k_funcinfo;
#endif // TEMPORARILY_REMOVED
    }

    // For this to work I need to pass false as second arg for createGUI
    plugActionList( QString::fromLatin1("import_actions"), importActions );
    plugActionList( QString::fromLatin1("export_actions"), exportActions );
    plugActionList( QString::fromLatin1("image_actions"), imageActions );
    plugActionList( QString::fromLatin1("tool_actions"), toolsActions );
    plugActionList( QString::fromLatin1("batch_actions"), batchActions );
#endif
}


void MainWindow::Window::slotImagesChanged( const KUrl::List& urls )
{
    for( KUrl::List::ConstIterator it = urls.begin(); it != urls.end(); ++it ) {
        ImageManager::Manager::instance()->removeThumbnail( (*it).path() );
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
        _tokenEditor = new TokenEditor( this );
    _tokenEditor->show();
    connect( _tokenEditor, SIGNAL( finished() ), _browser, SLOT( go() ) );
}

void MainWindow::Window::slotShowListOfFiles()
{
#ifdef KDAB_TEMPORARILY_REMOVED//QWERTY
    QStringList list = KInputDialog::getMultiLineText( i18n("Open List of Files"), i18n("Enter file names") )
                       .split( QChar::fromLatin1('\n'), QString::SkipEmptyParts );
    if ( list.isEmpty() )
        return;

    QStringList out;
    for ( QStringList::const_iterator it = list.begin(); it != list.end(); ++it ) {
        QString fileName = Utilities::imageFileNameToAbsolute( *it );
        if ( !DB::ImageDB::instance()->info( fileName, DB::AbsolutePath).isNull() )
            out.append( fileName );
    }

    if ( out.isEmpty() )
        KMessageBox::sorry( this, i18n("No images matching your input were found."), i18n("No Matches") );
    else
        showThumbNails( out );
#else // KDAB_TEMPORARILY_REMOVED
    qFatal("Code commented out in MainWindow::Window::slotShowListOfFiles");
#endif //KDAB_TEMPORARILY_REMOVED
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
    InvalidDateFinder finder( this );
    if ( finder.exec() == QDialog::Accepted )
        showThumbNails();
}

void MainWindow::Window::showDateBarTip( const QString& msg )
{
    statusBar()->showMessage( msg, 3000 );
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

void MainWindow::Window::showThumbNails( const DB::ResultPtr& items )
{
    _thumbnailView->setImageList( items );
    _partial->setMatchCount( items->count() );
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

    KConfigGroup config = KGlobal::config()->group( QString::fromLatin1("SQLDB") );
    if (!config.exists()) {
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
    try {
        SQLDB::DatabaseAddress address = SQLDB::readConnectionParameters(config);

        SQLDB::Database sqlBackend(address);

        // TODO: ask if old database should be flushed first

        KProgressDialog dialog(this);
        dialog.setModal(true);
        dialog.setCaption(i18n("Converting database"));
        dialog.setLabelText
            (QString::fromLatin1("<p><b><nobr>%1</nobr></b></p><p>%2</p>")
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
        KMessageBox::error(this, i18n("Database conversion failed, because following error occurred:\n%1",e.whatAsQString()));
    }
#endif
}

void MainWindow::Window::slotRecalcCheckSums()
{
#ifdef KDAB_TEMPORARILY_REMOVED//QWERTY
    DB::ImageDB::instance()->slotRecalcCheckSums( selected() );
#else // KDAB_TEMPORARILY_REMOVED
    qFatal("Code commented out in MainWindow::Window::slotRecalcCheckSums");
#endif //KDAB_TEMPORARILY_REMOVED
}

void MainWindow::Window::slotShowExifInfo()
{
#ifdef HAVE_EXIV2
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
    new ThumbnailView::ThumbnailBuilder( this );
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
        ImageManager::Manager::instance()->removeThumbnail( *imageIt );

        int size = Settings::SettingsData::instance()->thumbSize();
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info( *imageIt, DB::AbsolutePath );
        ImageManager::ImageRequest* request = new ImageManager::ImageRequest( *imageIt, QSize(size,size), info->angle(), _thumbnailView );
        request->setPriority( ImageManager::BatchTask );
        ImageManager::Manager::instance()->load( request );
    }

}


void MainWindow::Window::showVideos()
{
    KRun::runUrl(KUrl(QString::fromLatin1("http://www.kphotoalbum.org/videos.html")), QString::fromLatin1( "text/html" ), this );
}

void MainWindow::Window::slotStatistics()
{
    static StatisticsDialog* dialog = 0;
    if ( !dialog )
        dialog = new StatisticsDialog(this);
    dialog->show();
}

void MainWindow::Window::setupStatusBar()
{
    // Avoid flicker in the statusbar when moving over dates from the datebar
    QFont f( statusBar()->font() );
    f.setStyleHint( QFont::TypeWriter );
    f.setFamily( QString::fromLatin1( "courier" ) );
    f.setBold( true );
    statusBar()->setFont( f );

    KHBox* indicators = new KHBox( statusBar());
    _dirtyIndicator = new DirtyIndicator( indicators );

    _lockedIndicator = new QLabel( indicators );
    setLocked( Settings::SettingsData::instance()->isLocked(), true );

    statusBar()->addPermanentWidget( indicators, 0 );

    _partial = new ImageCounter( statusBar() );
    statusBar()->addPermanentWidget( _partial, 0 );

    ImageCounter* total = new ImageCounter( statusBar() );
    statusBar()->addPermanentWidget( total, 0 );
    total->setTotal( DB::ImageDB::instance()->totalCount() );
    connect( DB::ImageDB::instance(), SIGNAL( totalChanged( uint ) ), total, SLOT( setTotal( uint ) ) );

    _pathIndicator = new QLabel;
    statusBar()->addWidget( _pathIndicator );
}

void MainWindow::Window::slotRecreateExifDB()
{
#ifdef HAVE_EXIV2
    Exif::Database::instance()->recreate();
#endif
}

#include "Window.moc"
