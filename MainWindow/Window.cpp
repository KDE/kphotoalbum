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

#include <config-kpa-kipi.h>
#include "Window.h"

#include <stdexcept>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QContextMenuEvent>
#include <QCursor>
#include <QElapsedTimer>
#include <QDir>
#include <QFrame>
#include <QInputDialog>
#include <QLayout>
#include <QLoggingCategory>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMoveEvent>
#include <QObject>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KActionMenu>
#include <KEditToolBar>
#include <kio_version.h> // for #if KIO_VERSION...
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPasswordDialog>
#include <KProcess>
#include <KRun>
#include <KSharedConfig>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <ktip.h>
#include <KToggleAction>
#include <KConfigGroup>

#ifdef HASKIPI
#  include <KIPI/PluginLoader>
#  include <KIPI/Plugin>
#endif

#include <AnnotationDialog/Dialog.h>
#include <BackgroundJobs/SearchForVideosWithoutLengthInfo.h>
#include <BackgroundJobs/SearchForVideosWithoutVideoThumbnailsJob.h>
#include <BackgroundTaskManager/JobManager.h>
#include <Browser/BrowserWidget.h>
#include <DateBar/DateBarWidget.h>
#include <DB/CategoryCollection.h>
#include <DB/ImageDateCollection.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <DB/MD5.h>
#include <DB/MD5Map.h>
#include <Exif/Database.h>
#include <Exif/InfoDialog.h>
#include <Exif/Info.h>
#include <Exif/ReReadDialog.h>
#include <HTMLGenerator/HTMLDialog.h>
#include <ImageManager/AsyncLoader.h>
#include <ImageManager/ThumbnailBuilder.h>
#include <ImageManager/ThumbnailCache.h>
#include <ImportExport/Export.h>
#include <ImportExport/Import.h>
#ifdef HASKIPI
#  include <Plugins/Interface.h>
#endif
#include <RemoteControl/RemoteInterface.h>
#include <Settings/SettingsData.h>
#include <Settings/SettingsDialog.h>
#include <ThumbnailView/enums.h>
#include <ThumbnailView/ThumbnailFacade.h>
#include <Utilities/DemoUtil.h>
#include <Utilities/List.h>
#include <Utilities/ShowBusyCursor.h>
#include <Utilities/Util.h>
#include <Viewer/ViewerWidget.h>

#include "AutoStackImages.h"
#include "BreadcrumbViewer.h"
#include "CopyPopup.h"
#include "DeleteDialog.h"
#include "DirtyIndicator.h"
#include "DuplicateMerger/DuplicateMerger.h"
#include "ExternalPopup.h"
#include "FeatureDialog.h"
#include "ImageCounter.h"
#include "InvalidDateFinder.h"
#include "Logging.h"
#include "Options.h"
#include "SearchBar.h"
#include "SplashScreen.h"
#include "StatisticsDialog.h"
#include "StatusBar.h"
#include "TokenEditor.h"
#include "UpdateVideoThumbnail.h"
#include "WelcomeDialog.h"

using namespace DB;

MainWindow::Window* MainWindow::Window::s_instance = nullptr;

MainWindow::Window::Window( QWidget* parent )
    :KXmlGuiWindow( parent ),
      m_annotationDialog(nullptr),
      m_deleteDialog( nullptr ), m_htmlDialog(nullptr), m_tokenEditor( nullptr )
{
#ifdef HAVE_KGEOMAP
    m_positionBrowser = 0;
#endif

    qCDebug(MainWindowLog) << "Using icon theme: " << QIcon::themeName();
    qCDebug(MainWindowLog) << "Icon search paths: " << QIcon::themeSearchPaths();
    QElapsedTimer timer;
    timer.start();
    SplashScreen::instance()->message( i18n("Loading Database") );
    s_instance = this;

    bool gotConfigFile = load();
    if ( !gotConfigFile )
        throw 0;
    qCInfo(TimingLog) << "MainWindow: Loading Database: " << timer.restart() << "ms.";
    SplashScreen::instance()->message( i18n("Loading Main Window") );

    QWidget* top = new QWidget( this );
    QVBoxLayout* lay = new QVBoxLayout( top );
    lay->setSpacing(2);
    lay->setContentsMargins(2,2,2,2);
    setCentralWidget( top );

    m_stack = new QStackedWidget( top );
    lay->addWidget( m_stack, 1 );

    m_dateBar = new DateBar::DateBarWidget( top );
    lay->addWidget( m_dateBar );

    m_dateBarLine = new QFrame( top );
    m_dateBarLine->setFrameStyle( QFrame::HLine | QFrame::Plain );
    m_dateBarLine->setLineWidth(0); m_dateBarLine->setMidLineWidth(0);

    QPalette pal = m_dateBarLine->palette();
    pal.setColor( QPalette::WindowText, QColor("#c4c1bd") );
    m_dateBarLine->setPalette( pal );

    lay->addWidget( m_dateBarLine );

    setHistogramVisibilty(Settings::SettingsData::instance()->showHistogram());

    m_browser = new Browser::BrowserWidget( m_stack );
    m_thumbnailView = new ThumbnailView::ThumbnailFacade();

    m_stack->addWidget( m_browser );
    m_stack->addWidget( m_thumbnailView->gui() );
    m_stack->setCurrentWidget( m_browser );

    m_settingsDialog = nullptr;
    qCInfo(TimingLog) << "MainWindow: Loading MainWindow: " << timer.restart() << "ms.";
    setupMenuBar();
    qCInfo(TimingLog) << "MainWindow: setupMenuBar: " << timer.restart() << "ms.";
    createSarchBar();
    qCInfo(TimingLog) << "MainWindow: createSearchBar: " << timer.restart() << "ms.";
    setupStatusBar();
    qCInfo(TimingLog) << "MainWindow: setupStatusBar: " << timer.restart() << "ms.";

    // Misc
    m_autoSaveTimer = new QTimer( this );
    connect(m_autoSaveTimer, &QTimer::timeout, this, &Window::slotAutoSave);
    startAutoSaveTimer();

    connect(m_browser, &Browser::BrowserWidget::showingOverview, this, &Window::showBrowser);
    connect( m_browser, SIGNAL(pathChanged(Browser::BreadcrumbList)), m_statusBar->mp_pathIndicator, SLOT(setBreadcrumbs(Browser::BreadcrumbList)) );
    connect( m_statusBar->mp_pathIndicator, SIGNAL(widenToBreadcrumb(Browser::Breadcrumb)), m_browser, SLOT(widenToBreadcrumb(Browser::Breadcrumb)) );
    connect( m_browser, SIGNAL(pathChanged(Browser::BreadcrumbList)), this, SLOT(updateDateBar(Browser::BreadcrumbList)) );

    connect(m_dateBar, &DateBar::DateBarWidget::dateSelected, m_thumbnailView, &ThumbnailView::ThumbnailFacade::gotoDate);
    connect(m_dateBar, &DateBar::DateBarWidget::toolTipInfo, this, &Window::showDateBarTip);
    connect( Settings::SettingsData::instance(), SIGNAL(histogramSizeChanged(QSize)), m_dateBar, SLOT(setHistogramBarSize(QSize)) );
    connect( Settings::SettingsData::instance(), SIGNAL(actualThumbnailSizeChanged(int)), this, SLOT(slotThumbnailSizeChanged()) );

    connect(m_dateBar, &DateBar::DateBarWidget::dateRangeChange, this, &Window::setDateRange);
    connect(m_dateBar, &DateBar::DateBarWidget::dateRangeCleared, this, &Window::clearDateRange);
    connect(m_thumbnailView, &ThumbnailView::ThumbnailFacade::currentDateChanged, m_dateBar, &DateBar::DateBarWidget::setDate);

    connect(m_thumbnailView, &ThumbnailView::ThumbnailFacade::showImage, this, &Window::showImage);
    connect( m_thumbnailView, SIGNAL(showSelection()), this, SLOT(slotView()) );

    connect(m_thumbnailView, &ThumbnailView::ThumbnailFacade::fileIdUnderCursorChanged, this, &Window::slotSetFileName);
    connect( DB::ImageDB::instance(), SIGNAL(totalChanged(uint)), this, SLOT(updateDateBar()) );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL(categoryCollectionChanged()), this, SLOT(slotOptionGroupChanged()) );
    connect( m_browser, SIGNAL(imageCount(uint)), m_statusBar->mp_partial, SLOT(showBrowserMatches(uint)) );
    connect(m_thumbnailView, &ThumbnailView::ThumbnailFacade::selectionChanged, this, &Window::updateContextMenuFromSelectionSize);

    checkIfMplayerIsInstalled();
    executeStartupActions();

    qCInfo(TimingLog) << "MainWindow: executeStartupActions " << timer.restart() << "ms.";
    QTimer::singleShot( 0, this, SLOT(delayedInit()) );
    updateContextMenuFromSelectionSize(0);

    // Automatically save toolbar settings
    setAutoSaveSettings();

    qCInfo(TimingLog) << "MainWindow: misc setup time: " << timer.restart() << "ms.";
}

MainWindow::Window::~Window()
{
    DB::ImageDB::deleteInstance();
    ImageManager::ThumbnailCache::deleteInstance();
    Exif::Database::deleteInstance();
}

void MainWindow::Window::delayedInit()
{
    QElapsedTimer timer;
    timer.start();
    SplashScreen* splash = SplashScreen::instance();
    setupPluginMenu();
    qCInfo(TimingLog) << "MainWindow: setupPluginMenu: " << timer.restart() << "ms.";


    if ( Settings::SettingsData::instance()->searchForImagesOnStart() ||
         Options::the()->searchForImagesOnStart() ) {
        splash->message( i18n("Searching for New Files") );
        qApp->processEvents();
        DB::ImageDB::instance()->slotRescan();
        qCInfo(TimingLog) << "MainWindow: Search for New Files: " << timer.restart() << "ms.";
    }

    if ( !Settings::SettingsData::instance()->delayLoadingPlugins() ) {
        splash->message( i18n( "Loading Plug-ins" ) );
        loadPlugins();
        qCInfo(TimingLog) << "MainWindow: Loading Plug-ins: " << timer.restart() << "ms.";
    }

    splash->done();
    show();
    updateDateBar();
    qCInfo(TimingLog) << "MainWindow: MainWindow.show():" << timer.restart() << "ms.";

    QUrl importUrl = Options::the()->importFile();
    if ( importUrl.isValid() )
    {
        // I need to do this in delayed init to get the import window on top of the normal window
        ImportExport::Import::imageImport( importUrl );
        qCInfo(TimingLog) << "MainWindow: imageImport:" << timer.restart() << "ms.";
    } else {
        // I need to postpone this otherwise the tip dialog will not get focus on start up
        KTipDialog::showTip( this );
    }

    Exif::Database::instance(); // Load the database
    qCInfo(TimingLog) << "MainWindow: Loading Exif DB:" << timer.restart() << "ms.";

    if (!Options::the()->listen().isNull())
        RemoteControl::RemoteInterface::instance().listen(Options::the()->listen());
    else if ( Settings::SettingsData::instance()->listenForAndroidDevicesOnStartup())
        RemoteControl::RemoteInterface::instance().listen();

    announceAndroidVersion();
}


bool MainWindow::Window::slotExit()
{
    if ( Options::the()->demoMode() ) {
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

    if ( m_statusBar->mp_dirtyIndicator->isSaveDirty() ) {
        int ret = KMessageBox::warningYesNoCancel( this, i18n("Do you want to save the changes?"),
                                                   i18n("Save Changes?") );
        if (ret == KMessageBox::Cancel) {
            return false;
        }
        if ( ret == KMessageBox::Yes ) {
            slotSave();
        }
        if ( ret == KMessageBox::No ) {
            QDir().remove( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1(".#index.xml") );
        }
    }
    // Flush any remaining thumbnails
    ImageManager::ThumbnailCache::instance()->save();

doQuit:
    ImageManager::AsyncLoader::instance()->requestExit();
    qApp->quit();
    return true;
}

void MainWindow::Window::slotOptions()
{
    if ( ! m_settingsDialog ) {
        m_settingsDialog = new Settings::SettingsDialog( this );
        connect( m_settingsDialog, SIGNAL(changed()), this, SLOT(reloadThumbnails()) );
        connect(m_settingsDialog, &Settings::SettingsDialog::changed, this, &Window::startAutoSaveTimer);
        connect(m_settingsDialog, &Settings::SettingsDialog::changed, m_browser, &Browser::BrowserWidget::reload);
    }
    m_settingsDialog->show();
}

void MainWindow::Window::slotCreateImageStack()
{
    const DB::FileNameList list = selected();
    if (list.size() < 2) {
        // it doesn't make sense to make a stack from one image, does it?
        return;
    }

    bool ok = DB::ImageDB::instance()->stack( list );
    if ( !ok ) {
        if ( KMessageBox::questionYesNo( this,
                                         i18n("Some of the selected images already belong to a stack. "
                                              "Do you want to remove them from their stacks and create a "
                                              "completely new one?"), i18n("Stacking Error")) == KMessageBox::Yes ) {
            DB::ImageDB::instance()->unstack(list);
            if ( ! DB::ImageDB::instance()->stack(list)) {
                KMessageBox::sorry( this,
                                    i18n("Unknown error, stack creation failed."),
                                    i18n("Stacking Error"));
                return;
            }
        } else {
            return;
        }
    }

    DirtyIndicator::markDirty();
    // The current item might have just became invisible
    m_thumbnailView->setCurrentItem(list.at(0));
    m_thumbnailView->updateDisplayModel();
}

/** @short Make the selected image the head of a stack
 *
 * The whole point of image stacking is to group images together and then select
 * one of them as the "most important". This function is (maybe just a
 * temporary) way of promoting a selected image to the "head" of a stack it
 * belongs to. In future, it might get replaced by a Ligtroom-like interface.
 * */
void MainWindow::Window::slotSetStackHead()
{
    const DB::FileNameList list = selected();
    if ( list.size() != 1 ) {
        // this should be checked by enabling/disabling of QActions
        return;
    }

    setStackHead( *list.begin() );
}

void MainWindow::Window::setStackHead( const DB::FileName& image )
{
    if ( ! image.info()->isStacked() )
        return;

    unsigned int oldOrder = image.info()->stackOrder();

    DB::FileNameList others = DB::ImageDB::instance()->getStackFor(image);
    Q_FOREACH( const DB::FileName& current, others ) {
        if (current == image) {
            current.info()->setStackOrder( 1 );
        } else if ( current.info()->stackOrder() < oldOrder ) {
            current.info()->setStackOrder( current.info()->stackOrder() + 1 );
        }
    }

    DirtyIndicator::markDirty();
    m_thumbnailView->updateDisplayModel();
}

void MainWindow::Window::slotUnStackImages()
{
    const DB::FileNameList& list = selected();
    if (list.isEmpty())
        return;

    DB::ImageDB::instance()->unstack(list);
    DirtyIndicator::markDirty();
    m_thumbnailView->updateDisplayModel();
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
    const DB::FileNameList& list = selected();
    if (list.isEmpty()) {
        KMessageBox::sorry( this, i18n("No item is selected."), i18n("No Selection") );
    }
    else {
        DB::ImageInfoList images;
        Q_FOREACH( const DB::FileName& fileName, list) {
            images.append(fileName.info());
        }
        configureImages( images, oneAtATime );
    }
}

void MainWindow::Window::configureImages( const DB::ImageInfoList& list, bool oneAtATime )
{
    s_instance->configImages( list, oneAtATime );
}


void MainWindow::Window::configImages( const DB::ImageInfoList& list, bool oneAtATime )
{
    createAnnotationDialog();
    if ( m_annotationDialog->configure( list,  oneAtATime ) == QDialog::Rejected )
        return;

    reloadThumbnails(  ThumbnailView::MaintainSelection );
}


void MainWindow::Window::slotSearch()
{
    createAnnotationDialog();
    DB::ImageSearchInfo searchInfo = m_annotationDialog->search();
    if ( !searchInfo.isNull() )
        m_browser->addSearch( searchInfo );
}

void MainWindow::Window::createAnnotationDialog()
{
    Utilities::ShowBusyCursor dummy;
    if ( !m_annotationDialog.isNull() )
        return;

    m_annotationDialog = new AnnotationDialog::Dialog( nullptr );
    connect(m_annotationDialog.data(), &AnnotationDialog::Dialog::imageRotated, this, &Window::slotImageRotated);
}

void MainWindow::Window::slotSave()
{
    Utilities::ShowBusyCursor dummy;
    m_statusBar->showMessage(i18n("Saving..."), 5000 );
    DB::ImageDB::instance()->save( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("index.xml"), false );
    ImageManager::ThumbnailCache::instance()->save();
    m_statusBar->mp_dirtyIndicator->saved();
    QDir().remove( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1(".#index.xml") );
    m_statusBar->showMessage(i18n("Saving... Done"), 5000 );
}

void MainWindow::Window::slotDeleteSelected()
{
    if ( ! m_deleteDialog )
        m_deleteDialog = new DeleteDialog( this );
    if ( m_deleteDialog->exec( selected() ) != QDialog::Accepted )
        return;

    DirtyIndicator::markDirty();
}

void MainWindow::Window::slotCopySelectedURLs()
{
    QList<QUrl> urls; int urlcount = 0;
    Q_FOREACH(const DB::FileName &fileName, selected()) {
        urls.append( QUrl::fromLocalFile(fileName.absolute()) );
        urlcount++;
    }
    if (urlcount == 1) m_paste->setEnabled (true); else m_paste->setEnabled(false);
    QMimeData* mimeData = new QMimeData;
    mimeData->setUrls(urls);

    QApplication::clipboard()->setMimeData( mimeData );
}

void MainWindow::Window::slotPasteInformation()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();

    // Idealy this would look like
    // QList<QUrl> urls;
    // urls.fromMimeData(mimeData);
    // if ( urls.count() != 1 ) return;
    // const QString string = urls.first().path();

    QString string = mimeData->text();
    // fail silent if more than one image is in clipboard.
    if (string.count(QString::fromLatin1("\n")) != 0) return;

    const QString urlHead = QLatin1String("file://");
    if (string.startsWith(urlHead)) {
        string = string.right(string.size()-urlHead.size());
    }

    const DB::FileName fileName = DB::FileName::fromAbsolutePath(string);
    // fail silent if there is no file.
    if (fileName.isNull()) return;

    MD5 originalSum = MD5Sum( fileName );
    ImageInfoPtr originalInfo;
    if ( DB::ImageDB::instance()->md5Map()->contains( originalSum ) ) {
        originalInfo = DB::ImageDB::instance()->info( fileName );
    } else {
        originalInfo = fileName.info();
    }
    // fail silent if there is no info for the file.
    if (!originalInfo) return;

    Q_FOREACH(const DB::FileName& newFile, selected()) {
        newFile.info()->copyExtraData(*originalInfo, false);
    }
    DirtyIndicator::markDirty();
}

void MainWindow::Window::slotReReadExifInfo()
{
    DB::FileNameList files = selectedOnDisk();
    static Exif::ReReadDialog* dialog = nullptr;
    if ( ! dialog )
        dialog = new Exif::ReReadDialog( this );
    if ( dialog->exec( files ) == QDialog::Accepted )
        DirtyIndicator::markDirty();
}

void MainWindow::Window::slotAutoStackImages()
{
    const DB::FileNameList list = selected();
    if (list.isEmpty()) {
        KMessageBox::sorry( this, i18n("No item is selected."), i18n("No Selection") );
        return;
    }
    QPointer<MainWindow::AutoStackImages> stacker = new AutoStackImages( this, list );
    if ( stacker->exec() == QDialog::Accepted )
        showThumbNails();
    delete stacker;
}

/**
 * In thumbnail mode, return a list of files that are selected.
 * Otherwise, return all images in the current scope/context.
 */
DB::FileNameList MainWindow::Window::selected( ThumbnailView::SelectionMode mode) const
{
    if ( m_thumbnailView->gui() == m_stack->currentWidget() )
        return m_thumbnailView->selection(mode);
    else
        // return all images in the current scope (parameter false: include images not on disk)
        return DB::ImageDB::instance()->currentScope(false);
}

void MainWindow::Window::slotViewNewWindow()
{
    slotView( false, false );
}

/*
 * Returns a list of files that are both selected and on disk. If there are no
 * selected files, returns all files form current context that are on disk.
 * Note: On some setups (NFS), this can be a very time-consuming method!
 * */
DB::FileNameList MainWindow::Window::selectedOnDisk()
{
    const DB::FileNameList list = selected(ThumbnailView::NoExpandCollapsedStacks);

    DB::FileNameList listOnDisk;
    Q_FOREACH(const DB::FileName& fileName, list) {
        if (DB::ImageInfo::imageOnDisk(fileName))
            listOnDisk.append(fileName);
    }

    return listOnDisk;
}

void MainWindow::Window::slotView( bool reuse, bool slideShow, bool random )
{
    launchViewer(selected(ThumbnailView::NoExpandCollapsedStacks), reuse, slideShow, random );
}

void MainWindow::Window::launchViewer(const DB::FileNameList& inputMediaList, bool reuse, bool slideShow, bool random)
{
    DB::FileNameList mediaList = inputMediaList;
    int seek = -1;
    if (mediaList.isEmpty()) {
        mediaList = m_thumbnailView->imageList( ThumbnailView::ViewOrder );
    } else if (mediaList.size() == 1) {
        // we fake it so it appears the user has selected all images
        // and magically scrolls to the originally selected one
        const DB::FileName first = mediaList.at(0);
        mediaList = m_thumbnailView->imageList( ThumbnailView::ViewOrder );
        seek = mediaList.indexOf(first);
    }

    if (mediaList.isEmpty())
        mediaList = DB::ImageDB::instance()->currentScope( false );

    if (mediaList.isEmpty()) {
        KMessageBox::sorry( this, i18n("There are no images to be shown.") );
        return;
    }

    if (random) {
        mediaList = DB::FileNameList(Utilities::shuffleList(mediaList));
    }

    Viewer::ViewerWidget* viewer;
    if ( reuse && Viewer::ViewerWidget::latest() ) {
        viewer = Viewer::ViewerWidget::latest();
        viewer->raise();
        viewer->activateWindow();
    }
    else
        viewer = new Viewer::ViewerWidget(Viewer::ViewerWidget::ViewerWindow,
                                          &m_viewerInputMacros);
    connect(viewer, &Viewer::ViewerWidget::soughtTo, m_thumbnailView, &ThumbnailView::ThumbnailFacade::changeSingleSelection);
    connect(viewer, &Viewer::ViewerWidget::imageRotated, this, &Window::slotImageRotated);

    viewer->show( slideShow );
    viewer->load( mediaList, seek < 0 ? 0 : seek );
    viewer->raise();
}

void MainWindow::Window::slotSortByDateAndTime()
{
    DB::ImageDB::instance()->sortAndMergeBackIn(selected());
    showThumbNails( DB::ImageDB::instance()->search( Browser::BrowserWidget::instance()->currentContext()));
    DirtyIndicator::markDirty();
}

void MainWindow::Window::slotSortAllByDateAndTime()
{
    DB::ImageDB::instance()->sortAndMergeBackIn(DB::ImageDB::instance()->images());
    if ( m_thumbnailView->gui() == m_stack->currentWidget() )
        showThumbNails( DB::ImageDB::instance()->search( Browser::BrowserWidget::instance()->currentContext()));
    DirtyIndicator::markDirty();
}


QString MainWindow::Window::welcome()
{
    QString configFileName;
    QPointer<MainWindow::WelcomeDialog> dialog = new WelcomeDialog( this );
    // exit if the user dismissed the welcome dialog
    if (!dialog->exec())
    {
        qApp->quit();
    }
    configFileName = dialog->configFileName();
    delete dialog;
    return configFileName;
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
    Utilities::ShowBusyCursor dummy;
    showThumbNails( selected() );
}

void MainWindow::Window::setupMenuBar()
{
    // File menu
    KStandardAction::save( this, SLOT(slotSave()), actionCollection() );
    KStandardAction::quit( this, SLOT(slotExit()), actionCollection() );
    m_generateHtml = actionCollection()->addAction( QString::fromLatin1("exportHTML") );
    m_generateHtml->setText( i18n("Generate HTML...") );
    connect(m_generateHtml, &QAction::triggered, this, &Window::slotExportToHTML);

    QAction* a = actionCollection()->addAction( QString::fromLatin1("import"), this, SLOT(slotImport()) );
    a->setText( i18n( "Import...") );

    a = actionCollection()->addAction( QString::fromLatin1("export"), this, SLOT(slotExport()) );
    a->setText( i18n( "Export/Copy Images...") );


    // Go menu
    a = KStandardAction::back( m_browser, SLOT(back()), actionCollection() );
    connect(m_browser, &Browser::BrowserWidget::canGoBack, a, &QAction::setEnabled);
    a->setEnabled( false );

    a = KStandardAction::forward( m_browser, SLOT(forward()), actionCollection() );
    connect(m_browser, &Browser::BrowserWidget::canGoForward, a, &QAction::setEnabled);
    a->setEnabled( false );

    a = KStandardAction::home( m_browser, SLOT(home()), actionCollection() );
    actionCollection()->setDefaultShortcut(a, Qt::CTRL + Qt::Key_Home);
    connect(a, &QAction::triggered, m_dateBar, &DateBar::DateBarWidget::clearSelection);

    KStandardAction::redisplay( m_browser, SLOT(go()), actionCollection() );

    // The Edit menu
    m_copy = KStandardAction::copy( this, SLOT(slotCopySelectedURLs()), actionCollection() );
    m_paste = KStandardAction::paste( this, SLOT(slotPasteInformation()), actionCollection() );
    m_paste->setEnabled(false);
    m_selectAll = KStandardAction::selectAll( m_thumbnailView, SLOT(selectAll()), actionCollection() );
    KStandardAction::find( this, SLOT(slotSearch()), actionCollection() );

    m_deleteSelected = actionCollection()->addAction(QString::fromLatin1("deleteSelected"));
    m_deleteSelected->setText( i18nc("Delete selected images", "Delete Selected" ) );
    m_deleteSelected->setIcon( QIcon::fromTheme( QString::fromLatin1("edit-delete") ) );
    actionCollection()->setDefaultShortcut(m_deleteSelected, Qt::Key_Delete);
    connect(m_deleteSelected, &QAction::triggered, this, &Window::slotDeleteSelected);

    a = actionCollection()->addAction(QString::fromLatin1("removeTokens"), this, SLOT(slotRemoveTokens()));
    a->setText( i18n("Remove Tokens...") );

    a = actionCollection()->addAction(QString::fromLatin1("showListOfFiles"), this, SLOT(slotShowListOfFiles()));
    a->setText( i18n("Open List of Files...")) ;


    m_configOneAtATime = actionCollection()->addAction( QString::fromLatin1("oneProp"), this, SLOT(slotConfigureImagesOneAtATime()) );
    m_configOneAtATime->setText( i18n( "Annotate Individual Items" ) );
    actionCollection()->setDefaultShortcut(m_configOneAtATime, Qt::CTRL + Qt::Key_1);

    m_configAllSimultaniously = actionCollection()->addAction( QString::fromLatin1("allProp"), this, SLOT(slotConfigureAllImages()) );
    m_configAllSimultaniously->setText( i18n( "Annotate Multiple Items at a Time" ) );
    actionCollection()->setDefaultShortcut(m_configAllSimultaniously, Qt::CTRL + Qt::Key_2);

    m_createImageStack = actionCollection()->addAction( QString::fromLatin1("createImageStack"), this, SLOT(slotCreateImageStack()) );
    m_createImageStack->setText( i18n("Merge Images into a Stack") );
    actionCollection()->setDefaultShortcut(m_createImageStack, Qt::CTRL + Qt::Key_3);

    m_unStackImages = actionCollection()->addAction( QString::fromLatin1("unStackImages"), this, SLOT(slotUnStackImages()) );
    m_unStackImages->setText( i18n("Remove Images from Stack") );

    m_setStackHead = actionCollection()->addAction( QString::fromLatin1("setStackHead"), this, SLOT(slotSetStackHead()) );
    m_setStackHead->setText( i18n("Set as First Image in Stack") );
    actionCollection()->setDefaultShortcut(m_setStackHead, Qt::CTRL + Qt::Key_4);

    m_rotLeft = actionCollection()->addAction( QString::fromLatin1("rotateLeft"), this, SLOT(slotRotateSelectedLeft()) );
    m_rotLeft->setText( i18n( "Rotate counterclockwise" ) );
    actionCollection()->setDefaultShortcut(m_rotLeft, Qt::Key_7);


    m_rotRight = actionCollection()->addAction( QString::fromLatin1("rotateRight"), this, SLOT(slotRotateSelectedRight()) );
    m_rotRight->setText( i18n( "Rotate clockwise" ) );
    actionCollection()->setDefaultShortcut(m_rotRight, Qt::Key_9);


    // The Images menu
    m_view = actionCollection()->addAction( QString::fromLatin1("viewImages"), this, SLOT(slotView()) );
    m_view->setText( i18n("View") );
    actionCollection()->setDefaultShortcut(m_view, Qt::CTRL + Qt::Key_I);

    m_viewInNewWindow = actionCollection()->addAction( QString::fromLatin1("viewImagesNewWindow"), this, SLOT(slotViewNewWindow()) );
    m_viewInNewWindow->setText( i18n("View (In New Window)") );

    m_runSlideShow = actionCollection()->addAction( QString::fromLatin1("runSlideShow"), this, SLOT(slotRunSlideShow()) );
    m_runSlideShow->setText( i18n("Run Slide Show") );
    m_runSlideShow->setIcon( QIcon::fromTheme( QString::fromLatin1("view-presentation") ) );
    actionCollection()->setDefaultShortcut(m_runSlideShow, Qt::CTRL + Qt::Key_R);

    m_runRandomSlideShow = actionCollection()->addAction( QString::fromLatin1("runRandomizedSlideShow"), this, SLOT(slotRunRandomizedSlideShow()) );
    m_runRandomSlideShow->setText( i18n( "Run Randomized Slide Show" ) );

    a = actionCollection()->addAction( QString::fromLatin1("collapseAllStacks"),
                                       m_thumbnailView, SLOT(collapseAllStacks()) );
    connect(m_thumbnailView, &ThumbnailView::ThumbnailFacade::collapseAllStacksEnabled, a, &QAction::setEnabled);
    a->setEnabled(false);
    a->setText( i18n("Collapse all stacks" ));

    a = actionCollection()->addAction( QString::fromLatin1("expandAllStacks"),
                                       m_thumbnailView, SLOT(expandAllStacks()) );
    connect(m_thumbnailView, &ThumbnailView::ThumbnailFacade::expandAllStacksEnabled, a, &QAction::setEnabled);
    a->setEnabled(false);
    a->setText( i18n("Expand all stacks" ));

    QActionGroup* grp = new QActionGroup( this );

    a = actionCollection()->add<KToggleAction>( QString::fromLatin1("orderIncr"), this, SLOT(slotOrderIncr()) );
    a->setText( i18n("Show &Oldest First") ) ;
    a->setActionGroup(grp);
    a->setChecked( !Settings::SettingsData::instance()->showNewestThumbnailFirst() );

    a = actionCollection()->add<KToggleAction>( QString::fromLatin1("orderDecr"), this, SLOT(slotOrderDecr()) );
    a->setText( i18n("Show &Newest First") );
    a->setActionGroup(grp);
    a->setChecked( Settings::SettingsData::instance()->showNewestThumbnailFirst() );

    m_sortByDateAndTime = actionCollection()->addAction( QString::fromLatin1("sortImages"), this, SLOT(slotSortByDateAndTime()) );
    m_sortByDateAndTime->setText( i18n("Sort Selected by Date && Time") );

    m_limitToMarked = actionCollection()->addAction( QString::fromLatin1("limitToMarked"), this, SLOT(slotLimitToSelected()) );
    m_limitToMarked->setText( i18n("Limit View to Selection") );

    m_jumpToContext = actionCollection()->addAction( QString::fromLatin1("jumpToContext"), this, SLOT(slotJumpToContext()) );
    m_jumpToContext->setText( i18n("Jump to Context") );
    actionCollection()->setDefaultShortcut(m_jumpToContext, Qt::CTRL + Qt::Key_J);
    m_jumpToContext->setIcon( QIcon::fromTheme( QString::fromLatin1( "kphotoalbum" ) ) ); // icon suggestion: go-jump (don't know the exact meaning though, so I didn't replace it right away

    m_lock = actionCollection()->addAction( QString::fromLatin1("lockToDefaultScope"), this, SLOT(lockToDefaultScope()) );
    m_lock->setText( i18n("Lock Images") );

    m_unlock = actionCollection()->addAction( QString::fromLatin1("unlockFromDefaultScope"), this, SLOT(unlockFromDefaultScope()) );
    m_unlock->setText( i18n("Unlock") );

    a = actionCollection()->addAction( QString::fromLatin1("changeScopePasswd"), this, SLOT(changePassword()) );
    a->setText( i18n("Change Password...") );
    actionCollection()->setDefaultShortcut(a, 0);

    m_setDefaultPos = actionCollection()->addAction( QString::fromLatin1("setDefaultScopePositive"), this, SLOT(setDefaultScopePositive()) );
    m_setDefaultPos->setText( i18n("Lock Away All Other Items") );

    m_setDefaultNeg = actionCollection()->addAction( QString::fromLatin1("setDefaultScopeNegative"), this, SLOT(setDefaultScopeNegative()) );
    m_setDefaultNeg->setText( i18n("Lock Away Current Set of Items") );

    // Maintenance
    a = actionCollection()->addAction( QString::fromLatin1("findUnavailableImages"), this, SLOT(slotShowNotOnDisk()) );
    a->setText( i18n("Display Images and Videos Not on Disk") );

    a = actionCollection()->addAction( QString::fromLatin1("findImagesWithInvalidDate"), this, SLOT(slotShowImagesWithInvalidDate()) );
    a->setText( i18n("Display Images and Videos with Incomplete Dates...") );

#ifdef DOES_STILL_NOT_WORK_IN_KPA4
    a = actionCollection()->addAction( QString::fromLatin1("findImagesWithChangedMD5Sum"), this, SLOT(slotShowImagesWithChangedMD5Sum()) );
    a->setText( i18n("Display Images and Videos with Changed MD5 Sum") );
#endif //DOES_STILL_NOT_WORK_IN_KPA4

    a = actionCollection()->addAction( QLatin1String("mergeDuplicates"), this, SLOT(mergeDuplicates()));
    a->setText(i18n("Merge duplicates"));
    a = actionCollection()->addAction( QString::fromLatin1("rebuildMD5s"), this, SLOT(slotRecalcCheckSums()) );
    a->setText( i18n("Recalculate Checksum") );

    a = actionCollection()->addAction( QString::fromLatin1("rescan"), DB::ImageDB::instance(), SLOT(slotRescan()) );
    a->setText( i18n("Rescan for Images and Videos") );

    QAction* recreateExif = actionCollection()->addAction( QString::fromLatin1( "recreateExifDB" ), this, SLOT(slotRecreateExifDB()) );
    recreateExif->setText( i18n("Recreate Exif Search Database") );

    QAction* rereadExif = actionCollection()->addAction( QString::fromLatin1("reReadExifInfo"), this, SLOT(slotReReadExifInfo()) );
    rereadExif->setText( i18n("Read Exif Info from Files...") );

    m_sortAllByDateAndTime = actionCollection()->addAction( QString::fromLatin1("sortAllImages"), this, SLOT(slotSortAllByDateAndTime()) );
    m_sortAllByDateAndTime->setText( i18n("Sort All by Date && Time") );
    m_sortAllByDateAndTime->setEnabled(true);

    m_AutoStackImages = actionCollection()->addAction( QString::fromLatin1( "autoStack" ), this, SLOT (slotAutoStackImages()) );
    m_AutoStackImages->setText( i18n("Automatically Stack Selected Images...") );

    a = actionCollection()->addAction( QString::fromLatin1("buildThumbs"), this, SLOT(slotBuildThumbnails()) );
    a->setText( i18n("Build Thumbnails") );

    a->setText( i18n("Statistics...") );

    m_markUntagged = actionCollection()->addAction(QString::fromUtf8("markUntagged"),
                                                   this, SLOT(slotMarkUntagged()));
    m_markUntagged->setText(i18n("Mark As Untagged"));


    // Settings
    KStandardAction::preferences( this, SLOT(slotOptions()), actionCollection() );
    KStandardAction::keyBindings( this, SLOT(slotConfigureKeyBindings()), actionCollection() );
    KStandardAction::configureToolbars( this, SLOT(slotConfigureToolbars()), actionCollection() );

    a = actionCollection()->addAction( QString::fromLatin1("readdAllMessages"), this, SLOT(slotReenableMessages()) );
    a->setText( i18n("Enable All Messages") );

    m_viewMenu = actionCollection()->add<KActionMenu>( QString::fromLatin1("configureView") );
    m_viewMenu->setText( i18n("Configure Current View") );

    m_viewMenu->setIcon( QIcon::fromTheme( QString::fromLatin1( "view-list-details" ) ) );
    m_viewMenu->setDelayed( false );

    QActionGroup* viewGrp = new QActionGroup( this );
    viewGrp->setExclusive( true );

    m_smallListView = actionCollection()->add<KToggleAction>( QString::fromLatin1("smallListView"), m_browser, SLOT(slotSmallListView()) );
    m_smallListView->setText( i18n("Tree") );
    m_viewMenu->addAction( m_smallListView );
    m_smallListView->setActionGroup( viewGrp );

    m_largeListView = actionCollection()->add<KToggleAction>( QString::fromLatin1("largelistview"), m_browser, SLOT(slotLargeListView()) );
    m_largeListView->setText( i18n("Tree with User Icons") );
    m_viewMenu->addAction( m_largeListView );
    m_largeListView->setActionGroup( viewGrp );

    m_largeIconView = actionCollection()->add<KToggleAction>(  QString::fromLatin1("largeiconview"), m_browser, SLOT(slotLargeIconView()) );
    m_largeIconView->setText( i18n("Icons") );
    m_viewMenu->addAction( m_largeIconView );
    m_largeIconView->setActionGroup( viewGrp );

    connect(m_browser, &Browser::BrowserWidget::isViewChangeable, viewGrp, &QActionGroup::setEnabled);

    connect(m_browser, &Browser::BrowserWidget::currentViewTypeChanged, this, &Window::slotUpdateViewMenu);
    // The help menu
    KStandardAction::tipOfDay( this, SLOT(showTipOfDay()), actionCollection() );

    a = actionCollection()->add<KToggleAction>( QString::fromLatin1("showToolTipOnImages") );
    a->setText( i18n("Show Tooltips in Thumbnails Window") );
    actionCollection()->setDefaultShortcut(a, Qt::CTRL + Qt::Key_T);
    connect(a, &QAction::toggled, m_thumbnailView, &ThumbnailView::ThumbnailFacade::showToolTipsOnImages);


    a = actionCollection()->addAction( QString::fromLatin1("runDemo"), this, SLOT(runDemo()) );
    a->setText( i18n("Run KPhotoAlbum Demo") );

    a = actionCollection()->addAction( QString::fromLatin1("features"), this, SLOT(showFeatures()) );
    a->setText( i18n("KPhotoAlbum Feature Status") );

    a = actionCollection()->addAction( QString::fromLatin1("showVideo"), this, SLOT(showVideos()) );
    a->setText( i18n( "Show Demo Videos") );

    // Context menu actions
    m_showExifDialog = actionCollection()->addAction( QString::fromLatin1("showExifInfo"), this, SLOT(slotShowExifInfo()) );
    m_showExifDialog->setText( i18n("Show Exif Info") );

    m_recreateThumbnails = actionCollection()->addAction( QString::fromLatin1("recreateThumbnails"), m_thumbnailView, SLOT(slotRecreateThumbnail()) );
    m_recreateThumbnails->setText( i18n("Recreate Selected Thumbnails") );

    m_useNextVideoThumbnail = actionCollection()->addAction( QString::fromLatin1("useNextVideoThumbnail"), this, SLOT(useNextVideoThumbnail()));
    m_useNextVideoThumbnail->setText(i18n("Use next video thumbnail"));
    actionCollection()->setDefaultShortcut(m_useNextVideoThumbnail, Qt::CTRL + Qt::Key_Plus);

    m_usePreviousVideoThumbnail = actionCollection()->addAction( QString::fromLatin1("usePreviousVideoThumbnail"), this, SLOT(usePreviousVideoThumbnail()));
    m_usePreviousVideoThumbnail->setText(i18n("Use previous video thumbnail"));
    actionCollection()->setDefaultShortcut(m_usePreviousVideoThumbnail, Qt::CTRL + Qt::Key_Minus);

    createGUI( QString::fromLatin1( "kphotoalbumui.rc" ) );
}

void MainWindow::Window::slotExportToHTML()
{
    if ( ! m_htmlDialog )
        m_htmlDialog = new HTMLGenerator::HTMLDialog( this );
    m_htmlDialog->exec(selectedOnDisk());
}

void MainWindow::Window::startAutoSaveTimer()
{
    int i = Settings::SettingsData::instance()->autoSave();
    m_autoSaveTimer->stop();
    if ( i != 0 ) {
        m_autoSaveTimer->start( i * 1000 * 60  );
    }
}

void MainWindow::Window::slotAutoSave()
{
    if ( m_statusBar->mp_dirtyIndicator->isAutoSaveDirty() ) {
        Utilities::ShowBusyCursor dummy;
        m_statusBar->showMessage(i18n("Auto saving...."));
        DB::ImageDB::instance()->save( Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1(".#index.xml"), true );
        ImageManager::ThumbnailCache::instance()->save();
        m_statusBar->showMessage(i18n("Auto saving.... Done"), 5000);
        m_statusBar->mp_dirtyIndicator->autoSaved();
    }
}


void MainWindow::Window::showThumbNails()
{
    m_statusBar->showThumbnailSlider();
    reloadThumbnails( ThumbnailView::ClearSelection );
    m_stack->setCurrentWidget( m_thumbnailView->gui() );
    m_thumbnailView->gui()->setFocus();
    updateStates( true );
}

void MainWindow::Window::showBrowser()
{
    m_statusBar->clearMessage();
    m_statusBar->hideThumbnailSlider();
    m_stack->setCurrentWidget( m_browser );
    m_browser->setFocus();
    updateContextMenuFromSelectionSize( 0 );
    updateStates( false );
}


void MainWindow::Window::slotOptionGroupChanged()
{
    // FIXME: What if annotation dialog is open? (if that's possible)
    delete m_annotationDialog;
    m_annotationDialog = nullptr;
    DirtyIndicator::markDirty();
}

void MainWindow::Window::showTipOfDay()
{
    KTipDialog::showTip( this, QString(), true );
}


void MainWindow::Window::runDemo()
{
    KProcess* process = new KProcess;
    *process << QLatin1String("kphotoalbum") << QLatin1String("--demo");
    process->startDetached();
}

bool MainWindow::Window::load()
{
    // Let first try to find a config file.
    QString configFile;

    QUrl dbFileUrl = Options::the()->dbFile();
    if ( !dbFileUrl.isEmpty() && dbFileUrl.isLocalFile() )
    {
        configFile = dbFileUrl.toLocalFile();
    }
    else if ( Options::the()->demoMode() )
    {
        configFile = Utilities::setupDemo();
    }
    else {
        bool showWelcome = false;
        KConfigGroup config = KSharedConfig::openConfig()->group(QString::fromUtf8("General"));
        if ( config.hasKey( QString::fromLatin1("imageDBFile") ) ) {
            configFile = config.readEntry<QString>( QString::fromLatin1("imageDBFile"), QString() );
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

    // To avoid a race conditions where both the image loader thread creates an instance of
    // Settings, and where the main thread crates an instance, we better get it created now.
    Settings::SettingsData::setup( QFileInfo( configFile ).absolutePath() );

    if ( Settings::SettingsData::instance()->showSplashScreen() ) {
        SplashScreen::instance()->show();
        qApp->processEvents();
    }

    // Doing some validation on user provided index file
    if ( Options::the()->dbFile().isValid() ) {
        QFileInfo fi( configFile );

        if ( !fi.dir().exists() ) {
            KMessageBox::error( this, i18n("<p>Could not open given index.xml as provided directory does not exist.<br />%1</p>",
                                           fi.absolutePath()) );
            return false;
        }

        // We use index.xml as the XML backend, thus we want to test for exactly it
        fi.setFile( QString::fromLatin1( "%1/index.xml" ).arg( fi.dir().absolutePath() ) );
        if ( !fi.exists() ) {
            int answer = KMessageBox::questionYesNo(this,i18n("<p>Given index file does not exist, do you want to create following?"
                                                              "<br />%1/index.xml</p>", fi.absolutePath() ) );
            if (answer != KMessageBox::Yes)
                return false;
        }
        configFile = fi.absoluteFilePath();
    }
    DB::ImageDB::setupXMLDB( configFile );

    // some sanity checks:
    if ( ! Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured()
         && ! (Settings::SettingsData::instance()->untaggedCategory().isEmpty()
               && Settings::SettingsData::instance()->untaggedTag().isEmpty() )
         && ! Options::the()->demoMode() )
    {
        KMessageBox::error( this, i18n(
                                "<p>You have configured a tag for untagged images, but either the tag itself "
                                "or its category does not exist in the database.</p>"
                                "<p>Please review your untagged tag setting under "
                                "<interface>Settings|Configure KPhotoAlbum...|Categories</interface></p>"));
    }

    return true;
}

void MainWindow::Window::contextMenuEvent( QContextMenuEvent* e )
{
    if ( m_stack->currentWidget() == m_thumbnailView->gui() ) {
        QMenu menu( this );
        menu.addAction( m_configOneAtATime );
        menu.addAction( m_configAllSimultaniously );
        menu.addSeparator();
        menu.addAction( m_createImageStack );
        menu.addAction( m_unStackImages );
        menu.addAction( m_setStackHead );
        menu.addSeparator();
        menu.addAction( m_runSlideShow );
        menu.addAction(m_runRandomSlideShow );
        menu.addAction( m_showExifDialog);

        menu.addSeparator();
        menu.addAction(m_rotLeft);
        menu.addAction(m_rotRight);
        menu.addAction(m_recreateThumbnails);
        menu.addAction(m_useNextVideoThumbnail);
        menu.addAction(m_usePreviousVideoThumbnail);
        m_useNextVideoThumbnail->setEnabled(anyVideosSelected());
        m_usePreviousVideoThumbnail->setEnabled(anyVideosSelected());
        menu.addSeparator();

        menu.addAction(m_view);
        menu.addAction(m_viewInNewWindow);

        // "Invoke external program"

        ExternalPopup externalCommands { &menu };
        DB::ImageInfoPtr info = m_thumbnailView->mediaIdUnderCursor().info();

        externalCommands.populate( info, selected());
        QAction* action = menu.addMenu( &externalCommands );
        if (!info && selected().isEmpty())
            action->setEnabled( false );

        QUrl selectedFile = QUrl::fromLocalFile(info->fileName().absolute());
        QList<QUrl> allSelectedFiles;
        for (const QString &selectedFile : selected().toStringList(DB::AbsolutePath)) {
            allSelectedFiles << QUrl::fromLocalFile(selectedFile);
        }

        // "Copy image(s) to ..."
        CopyPopup copyMenu (&menu, selectedFile, allSelectedFiles, m_lastTarget, CopyPopup::Copy);
        QAction *copyAction = menu.addMenu(&copyMenu);
        if (!info && selected().isEmpty()) {
            copyAction->setEnabled(false);
        }

        // "Link image(s) to ..."
        CopyPopup linkMenu (&menu, selectedFile, allSelectedFiles, m_lastTarget, CopyPopup::Link);
        QAction *linkAction = menu.addMenu(&linkMenu);
        if (!info && selected().isEmpty()) {
            linkAction->setEnabled(false);
        }

        menu.exec( QCursor::pos() );
    }
    e->setAccepted(true);
}

void MainWindow::Window::setDefaultScopePositive()
{
    Settings::SettingsData::instance()->setCurrentLock( m_browser->currentContext(), false );
}

void MainWindow::Window::setDefaultScopeNegative()
{
    Settings::SettingsData::instance()->setCurrentLock( m_browser->currentContext(), true );
}

void MainWindow::Window::lockToDefaultScope()
{
    int i = KMessageBox::warningContinueCancel( this,
                                                i18n( "<p>The password protection is only a means of allowing your little sister "
                                                      "to look in your images, without getting to those embarrassing images from "
                                                      "your last party.</p>"
                                                      "<p><b>In other words, anyone with access to the index.xml file can easily "
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
    QPointer <KPasswordDialog> dialog = new KPasswordDialog( this );
    while ( !OK ) {
        dialog->setPrompt( i18n("Type in Password to Unlock") );
        const int code = dialog->exec();
        if ( code == QDialog::Rejected )
            return;
        const QString passwd = dialog->password();

        OK = (Settings::SettingsData::instance()->password() == passwd);

        if ( !OK )
            KMessageBox::sorry( this, i18n("Invalid password.") );
    }
    setLocked( false, false );
    delete dialog;
}

void MainWindow::Window::setLocked( bool locked, bool force, bool recount )
{
    m_statusBar->setLocked( locked );
    Settings::SettingsData::instance()->setLocked( locked, force );

    m_lock->setEnabled( !locked );
    m_unlock->setEnabled( locked );
    m_setDefaultPos->setEnabled( !locked );
    m_setDefaultNeg->setEnabled( !locked );
    if (recount)
        m_browser->reload();
}

void MainWindow::Window::changePassword()
{
    bool OK = ( Settings::SettingsData::instance()->password().isEmpty() );

    QPointer<KPasswordDialog> dialog = new KPasswordDialog;

    while ( !OK ) {
        dialog->setPrompt( i18n("Type in Old Password") );
        const int code = dialog->exec();
        if ( code == QDialog::Rejected )
            return;
        const QString passwd = dialog->password();

        OK = (Settings::SettingsData::instance()->password() == QString(passwd));

        if ( !OK )
            KMessageBox::sorry( this, i18n("Invalid password.") );
    }

    dialog->setPrompt( i18n("Type in New Password") );
    const int code = dialog->exec();
    if ( code == QDialog::Accepted )
        Settings::SettingsData::instance()->setPassword( dialog->password() );
    delete dialog;
}

void MainWindow::Window::slotConfigureKeyBindings()
{
    Viewer::ViewerWidget* viewer = new Viewer::ViewerWidget; // Do not show, this is only used to get a key configuration
    KShortcutsDialog* dialog = new KShortcutsDialog();
    dialog->addCollection( actionCollection(), i18n( "General" ) );
    dialog->addCollection( viewer->actions(), i18n("Viewer") );

#ifdef HASKIPI
    loadPlugins();
    Q_FOREACH( const KIPI::PluginLoader::Info *pluginInfo, m_pluginLoader->pluginList() ) {
        KIPI::Plugin* plugin = pluginInfo->plugin();
        if ( plugin )
            dialog->addCollection( plugin->actionCollection(),
                                   i18nc("Add 'Plugin' prefix so that KIPI plugins are obvious in KShortcutsDialog…","Plugin: %1", pluginInfo->name()) );
    }
#endif

    createAnnotationDialog();
    dialog->addCollection( m_annotationDialog->actions(), i18n("Annotation Dialog" ) );

    dialog->configure();

    delete dialog;
    delete viewer;
}

void MainWindow::Window::slotSetFileName( const DB::FileName& fileName )
{
    ImageInfoPtr info;

    if ( fileName.isNull() )
        m_statusBar->clearMessage();
    else {
        info = fileName.info();
        if (info != ImageInfoPtr(nullptr) )
            m_statusBar->showMessage( fileName.absolute(), 4000 );
    }
}

void MainWindow::Window::updateContextMenuFromSelectionSize(int selectionSize)
{
    m_configAllSimultaniously->setEnabled(selectionSize > 1);
    m_configOneAtATime->setEnabled(selectionSize >= 1);
    m_createImageStack->setEnabled(selectionSize > 1);
    m_unStackImages->setEnabled(selectionSize >= 1);
    m_setStackHead->setEnabled(selectionSize == 1); // FIXME: do we want to check if it's stacked here?
    m_sortByDateAndTime->setEnabled(selectionSize > 1);
    m_recreateThumbnails->setEnabled(selectionSize >= 1);
    m_rotLeft->setEnabled(selectionSize >= 1);
    m_rotRight->setEnabled(selectionSize >= 1);
    m_AutoStackImages->setEnabled(selectionSize > 1);
    m_markUntagged->setEnabled(selectionSize >= 1);
    m_statusBar->mp_selected->setSelectionCount( selectionSize );
}

void MainWindow::Window::rotateSelected( int angle )
{
    const DB::FileNameList list = selected();
    if (list.isEmpty())  {
        KMessageBox::sorry( this, i18n("No item is selected."),
                            i18n("No Selection") );
    } else {
        Q_FOREACH(const DB::FileName& fileName, list) {
            fileName.info()->rotate(angle);
            ImageManager::ThumbnailCache::instance()->removeThumbnail(fileName);
        }
        m_statusBar->mp_dirtyIndicator->markDirty();
    }
}

void MainWindow::Window::slotRotateSelectedLeft()
{
    rotateSelected( -90 );
    reloadThumbnails();
}

void MainWindow::Window::slotRotateSelectedRight()
{
    rotateSelected( 90 );
    reloadThumbnails();
}

void MainWindow::Window::reloadThumbnails( ThumbnailView::SelectionUpdateMethod method )
{
    m_thumbnailView->reload( method );
    updateContextMenuFromSelectionSize( m_thumbnailView->selection().size() );
}

void MainWindow::Window::slotUpdateViewMenu( DB::Category::ViewType type )
{
    if ( type == DB::Category::TreeView )
        m_smallListView->setChecked( true );
    else if ( type == DB::Category::ThumbedTreeView )
        m_largeListView->setChecked( true );
    else if ( type == DB::Category::ThumbedIconView )
        m_largeIconView->setChecked( true );
}

void MainWindow::Window::slotShowNotOnDisk()
{
    DB::FileNameList notOnDisk;
    Q_FOREACH(const DB::FileName& fileName, DB::ImageDB::instance()->images()) {
        if ( !fileName.exists() )
            notOnDisk.append(fileName);
    }

    showThumbNails(notOnDisk);
}


void MainWindow::Window::slotShowImagesWithChangedMD5Sum()
{
#ifdef DOES_STILL_NOT_WORK_IN_KPA4
    Utilities::ShowBusyCursor dummy;
    StringSet changed = DB::ImageDB::instance()->imagesWithMD5Changed();
    showThumbNails( changed.toList() );
#else // DOES_STILL_NOT_WORK_IN_KPA4
    qFatal("Code commented out in MainWindow::Window::slotShowImagesWithChangedMD5Sum");
#endif // DOES_STILL_NOT_WORK_IN_KPA4
}


void MainWindow::Window::updateStates( bool thumbNailView )
{
    m_selectAll->setEnabled( thumbNailView );
    m_deleteSelected->setEnabled( thumbNailView );
    m_limitToMarked->setEnabled( thumbNailView );
    m_jumpToContext->setEnabled( thumbNailView );
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
    Q_ASSERT( s_instance );
    return s_instance;
}

void MainWindow::Window::slotConfigureToolbars()
{
    QPointer<KEditToolBar> dlg = new KEditToolBar(guiFactory());
    connect(dlg, SIGNAL(newToolbarConfig()),
            SLOT(slotNewToolbarConfig()));
    dlg->exec();
    delete dlg;
}

void MainWindow::Window::slotNewToolbarConfig()
{
    createGUI();
    createSarchBar();
}

void MainWindow::Window::slotImport()
{
    ImportExport::Import::imageImport();
}

void MainWindow::Window::slotExport()
{
    ImportExport::Export::imageExport(selectedOnDisk());
}

void MainWindow::Window::slotReenableMessages()
{
    int ret = KMessageBox::questionYesNo( this, i18n("<p>Really enable all message boxes where you previously "
                                                     "checked the do-not-show-again check box?</p>" ) );
    if ( ret == KMessageBox::Yes )
        KMessageBox::enableAllMessages();

}

void MainWindow::Window::setupPluginMenu()
{
    QMenu* menu = findChild<QMenu*>( QString::fromLatin1("plugins") );
    if ( !menu ) {
        KMessageBox::error( this, i18n("<p>KPhotoAlbum hit an internal error (missing plug-in menu in MainWindow::Window::setupPluginMenu). This indicate that you forgot to do a make install. If you did compile KPhotoAlbum yourself, then please run make install. If not, please report this as a bug.</p><p>KPhotoAlbum will continue execution, but it is not entirely unlikely that it will crash later on due to the missing make install.</p>" ), i18n("Internal Error") );
        m_hasLoadedPlugins = true;
        return; // This is no good, but lets try and continue.
    }


#ifdef HASKIPI
    connect(menu, &QMenu::aboutToShow, this, &Window::loadPlugins);
    m_hasLoadedPlugins = false;
#else
    menu->setEnabled(false);
    m_hasLoadedPlugins = true;
#endif
}

void MainWindow::Window::loadPlugins()
{
#ifdef HASKIPI
    Utilities::ShowBusyCursor dummy;
    if ( m_hasLoadedPlugins )
        return;

    m_pluginInterface = new Plugins::Interface( this, QString::fromLatin1("KPhotoAlbum kipi interface") );
    connect(m_pluginInterface, &Plugins::Interface::imagesChanged, this, &Window::slotImagesChanged);

    QStringList ignores;
    ignores << QString::fromLatin1( "CommentsEditor" )
            << QString::fromLatin1( "HelloWorld" );

    m_pluginLoader = new KIPI::PluginLoader();
    m_pluginLoader->setIgnoredPluginsList( ignores );
    m_pluginLoader->setInterface( m_pluginInterface );
    m_pluginLoader->init();
    connect(m_pluginLoader, &KIPI::PluginLoader::replug, this, &Window::plug);
    m_pluginLoader->loadPlugins();

    // Setup signals
    connect(m_thumbnailView, &ThumbnailView::ThumbnailFacade::selectionChanged, this, &Window::slotSelectionChanged);
    m_hasLoadedPlugins = true;

    // Make sure selection is updated also when plugin loading is
    // delayed. This is needed, because selection might already be
    // non-empty when loading the plugins.
    slotSelectionChanged(selected().size());
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

    KIPI::PluginLoader::PluginList list = m_pluginLoader->pluginList();
    Q_FOREACH( const KIPI::PluginLoader::Info *pluginInfo, list ) {
        KIPI::Plugin* plugin = pluginInfo->plugin();
        if ( !plugin || !pluginInfo->shouldLoad() )
            continue;

        plugin->setup( this );

        QList<QAction*> actions = plugin->actions();
        Q_FOREACH( QAction *action, actions ) {
            KIPI::Category category = plugin->category( action );
            if (  category == KIPI::ImagesPlugin ||  category == KIPI::CollectionsPlugin )
                imageActions.append( action );

            else if ( category == KIPI::ImportPlugin )
                importActions.append( action );

            else if ( category == KIPI::ExportPlugin )
                exportActions.append( action );

            else if ( category == KIPI::ToolsPlugin )
                toolsActions.append( action );

            else if ( category == KIPI::BatchPlugin )
                batchActions.append( action );

            else {
                qCWarning(MainWindowLog) << "Unknown category\n";
            }
        }
        KConfigGroup group = KSharedConfig::openConfig()->group( QString::fromLatin1("Shortcuts") );
        plugin->actionCollection()->importGlobalShortcuts( &group );
    }


    setPluginMenuState( "importplugin", importActions );
    setPluginMenuState( "exportplugin", exportActions );
    setPluginMenuState( "imagesplugins", imageActions );
    setPluginMenuState( "batch_plugins", batchActions );
    setPluginMenuState( "tool_plugins", toolsActions );

    // For this to work I need to pass false as second arg for createGUI
    plugActionList( QString::fromLatin1("import_actions"), importActions );
    plugActionList( QString::fromLatin1("export_actions"), exportActions );
    plugActionList( QString::fromLatin1("image_actions"), imageActions );
    plugActionList( QString::fromLatin1("tool_actions"), toolsActions );
    plugActionList( QString::fromLatin1("batch_actions"), batchActions );
#endif
}

void MainWindow::Window::setPluginMenuState( const char* name, const QList<QAction*>& actions )
{
    QMenu* menu = findChild<QMenu*>( QString::fromLatin1(name) );
    if ( menu )
        menu->setEnabled(actions.count() != 0);
}



void MainWindow::Window::slotImagesChanged( const QList<QUrl>& urls )
{
    for( QList<QUrl>::ConstIterator it = urls.begin(); it != urls.end(); ++it ) {
        DB::FileName fileName = DB::FileName::fromAbsolutePath((*it).path());
        if ( !fileName.isNull()) {
            // Plugins may report images outsite of the photodatabase
            // This seems to be the case with the border image plugin, which reports the destination image
            ImageManager::ThumbnailCache::instance()->removeThumbnail( fileName );
            // update MD5sum:
            MD5 md5sum = MD5Sum( fileName );
            fileName.info()->setMD5Sum( md5sum );
        }
    }
    m_statusBar->mp_dirtyIndicator->markDirty();
    reloadThumbnails( ThumbnailView::MaintainSelection );
}

DB::ImageSearchInfo MainWindow::Window::currentContext()
{
    return m_browser->currentContext();
}

QString MainWindow::Window::currentBrowseCategory() const
{
    return m_browser->currentCategory();
}

void MainWindow::Window::slotSelectionChanged( int count )
{
#ifdef HASKIPI
    m_pluginInterface->slotSelectionChanged( count != 0 );
#else
    Q_UNUSED( count );
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
    if ( !m_tokenEditor )
        m_tokenEditor = new TokenEditor( this );
    m_tokenEditor->show();
    connect(m_tokenEditor, &TokenEditor::finished, m_browser, &Browser::BrowserWidget::go);
}

void MainWindow::Window::slotShowListOfFiles()
{
    QStringList list = QInputDialog::getMultiLineText( this,
                                                       i18n("Open List of Files"),
                                                       i18n("You can open a set of files from KPhotoAlbum's image root by listing the files here.")
                                                       )
            .split( QChar::fromLatin1('\n'), QString::SkipEmptyParts );
    if ( list.isEmpty() )
        return;

    DB::FileNameList out;
    for ( QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it ) {
        QString fileNameStr = Utilities::imageFileNameToAbsolute( *it );
        if ( fileNameStr.isNull() )
            continue;
        const DB::FileName fileName = DB::FileName::fromAbsolutePath(fileNameStr);
        if ( !fileName.isNull() )
            out.append(fileName);
    }

    if (out.isEmpty())
        KMessageBox::sorry( this, i18n("No images matching your input were found."), i18n("No Matches") );
    else
        showThumbNails(out);
}

void MainWindow::Window::updateDateBar( const Browser::BreadcrumbList& path )
{
    static QString lastPath = QString::fromLatin1("ThisStringShouldNeverBeSeenSoWeUseItAsInitialContent");
    if ( path.toString() != lastPath )
        updateDateBar();
    lastPath = path.toString();
}

void MainWindow::Window::updateDateBar()
{
    m_dateBar->setImageDateCollection( DB::ImageDB::instance()->rangeCollection() );
}


void MainWindow::Window::slotShowImagesWithInvalidDate()
{
    QPointer<InvalidDateFinder> finder = new InvalidDateFinder( this );
    if ( finder->exec() == QDialog::Accepted )
        showThumbNails();
    delete finder;
}

void MainWindow::Window::showDateBarTip( const QString& msg )
{
    m_statusBar->showMessage( msg, 3000 );
}

void MainWindow::Window::slotJumpToContext()
{
    const DB::FileName fileName =m_thumbnailView->currentItem();
    if ( !fileName.isNull() ) {
        m_browser->addImageView(fileName);
    }
}

void MainWindow::Window::setDateRange( const DB::ImageDate& range )
{
    DB::ImageDB::instance()->setDateRange( range, m_dateBar->includeFuzzyCounts() );
    m_statusBar->mp_partial->showBrowserMatches( this->selected().size() );
    m_browser->reload();
    reloadThumbnails( ThumbnailView::MaintainSelection );
}

void MainWindow::Window::clearDateRange()
{
    DB::ImageDB::instance()->clearDateRange();
    m_browser->reload();
    reloadThumbnails( ThumbnailView::MaintainSelection );
}

void MainWindow::Window::showThumbNails(const DB::FileNameList& items)
{
    m_thumbnailView->setImageList(items);
    m_statusBar->mp_partial->setMatchCount(items.size());
    showThumbNails();
}

void MainWindow::Window::slotRecalcCheckSums()
{
    DB::ImageDB::instance()->slotRecalcCheckSums( selected() );
}

void MainWindow::Window::slotShowExifInfo()
{
    DB::FileNameList items = selectedOnDisk();
    if (!items.isEmpty()) {
        Exif::InfoDialog* exifDialog = new Exif::InfoDialog(items.at(0), this);
        exifDialog->show();
    }
}

void MainWindow::Window::showFeatures()
{
    FeatureDialog dialog(this);
    dialog.exec();
}

void MainWindow::Window::showImage( const DB::FileName& fileName )
{
    launchViewer(DB::FileNameList() << fileName, true, false, false);
}

void MainWindow::Window::slotBuildThumbnails()
{
    ImageManager::ThumbnailBuilder::instance()->buildAll( ImageManager::StartNow );
}

void MainWindow::Window::slotBuildThumbnailsIfWanted()
{
    ImageManager::ThumbnailCache::instance()->flush();
    if ( ! Settings::SettingsData::instance()->incrementalThumbnails())
        ImageManager::ThumbnailBuilder::instance()->buildAll( ImageManager::StartDelayed );
}

void MainWindow::Window::slotOrderIncr()
{
    m_thumbnailView->setSortDirection( ThumbnailView::OldestFirst );
}

void MainWindow::Window::slotOrderDecr()
{
    m_thumbnailView->setSortDirection( ThumbnailView::NewestFirst );
}

void MainWindow::Window::showVideos()
{
#if (KIO_VERSION >= ((5<<16)|(31<<8)|(0)))
    KRun::runUrl(QUrl(QString::fromLatin1("http://www.kphotoalbum.org/index.php?page=videos"))
                 , QString::fromLatin1( "text/html" )
                 , this
                 , KRun::RunFlags()
                 );
#else
    // this signature is deprecated in newer kio versions
    // TODO: remove this when we don't support Ubuntu 16.04 LTS anymore
    KRun::runUrl(QUrl(QString::fromLatin1("http://www.kphotoalbum.org/index.php?page=videos"))
                 , QString::fromLatin1( "text/html" )
                 , this
                 );
#endif
}

void MainWindow::Window::slotStatistics()
{
    static StatisticsDialog* dialog = new StatisticsDialog(this);
    dialog->show();
}

void MainWindow::Window::slotMarkUntagged()
{
    if (Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured()) {
        for (const DB::FileName& newFile : selected()) {
            newFile.info()->addCategoryInfo(Settings::SettingsData::instance()->untaggedCategory(),
                                            Settings::SettingsData::instance()->untaggedTag());
        }

        DirtyIndicator::markDirty();
    } else {
        // Note: the same dialog text is used in
        // Browser::OverviewPage::activateUntaggedImagesAction(),
        // so if it is changed, be sure to also change it there!
        KMessageBox::information(this,
                                 i18n("<p>You have not yet configured which tag to use for indicating untagged images."
                                      "</p>"
                                      "<p>Please follow these steps to do so:"
                                      "<ul><li>In the menu bar choose <b>Settings</b></li>"
                                      "<li>From there choose <b>Configure KPhotoAlbum</b></li>"
                                      "<li>Now choose the <b>Categories</b> icon</li>"
                                      "<li>Now configure section <b>Untagged Images</b></li></ul></p>"),
                                 i18n("Feature has not been configured")
                                 );
    }
}

void MainWindow::Window::setupStatusBar()
{
    m_statusBar = new MainWindow::StatusBar;
    setStatusBar( m_statusBar );
    setLocked( Settings::SettingsData::instance()->locked(), true, false );
}

void MainWindow::Window::slotRecreateExifDB()
{
    Exif::Database::instance()->recreate();
}

void MainWindow::Window::useNextVideoThumbnail()
{
    UpdateVideoThumbnail::useNext(selected());
}

void MainWindow::Window::usePreviousVideoThumbnail()
{
    UpdateVideoThumbnail::usePrevious(selected());
}

void MainWindow::Window::mergeDuplicates()
{
    DuplicateMerger* merger = new DuplicateMerger;
    merger->show();
}

void MainWindow::Window::slotThumbnailSizeChanged()
{
    QString thumbnailSizeMsg = i18nc( "@info:status",
                                      //xgettext:no-c-format
                                      "Thumbnail width: %1px (storage size: %2px)",
                                      Settings::SettingsData::instance()->actualThumbnailSize(),
                                      Settings::SettingsData::instance()->thumbnailSize()
                                      );
    m_statusBar->showMessage( thumbnailSizeMsg, 4000);
}

void MainWindow::Window::createSarchBar()
{
    // Set up the search tool bar
    SearchBar* bar = new SearchBar( this );
    bar->setLineEditEnabled(false);
    bar->setObjectName(QString::fromUtf8("searchBar"));

    connect(bar, &SearchBar::textChanged, m_browser, &Browser::BrowserWidget::slotLimitToMatch);
    connect(bar, &SearchBar::returnPressed, m_browser, &Browser::BrowserWidget::slotInvokeSeleted);
    connect(bar, &SearchBar::keyPressed, m_browser, &Browser::BrowserWidget::scrollKeyPressed);
    connect(m_browser, &Browser::BrowserWidget::viewChanged, bar, &SearchBar::reset);
    connect(m_browser, &Browser::BrowserWidget::isSearchable, bar, &SearchBar::setLineEditEnabled);
}

void MainWindow::Window::executeStartupActions()
{
    new ImageManager::ThumbnailBuilder( m_statusBar, this );
    if ( ! Settings::SettingsData::instance()->incrementalThumbnails())
        ImageManager::ThumbnailBuilder::instance()->buildMissing();
    connect( Settings::SettingsData::instance(), SIGNAL(thumbnailSizeChanged(int)), this, SLOT(slotBuildThumbnailsIfWanted()) );

    if ( ! FeatureDialog::hasVideoThumbnailer() ) {
        BackgroundTaskManager::JobManager::instance()->addJob(
                    new BackgroundJobs::SearchForVideosWithoutLengthInfo );

        BackgroundTaskManager::JobManager::instance()->addJob(
                    new BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob );
    }
}

void MainWindow::Window::checkIfMplayerIsInstalled()
{
    if (Options::the()->demoMode())
        return;

    if ( !FeatureDialog::hasVideoThumbnailer() ) {
        KMessageBox::information( this,
                                  i18n("<p>Unable to find ffmpeg or MPlayer on the system.</p>"
                                       "<p>Without either of these, KPhotoAlbum will not be able to display video thumbnails and video lengths. "
                                       "Please install the ffmpeg or MPlayer package</p>"),
                                  i18n("Video thumbnails are not available"), QString::fromLatin1("mplayerNotInstalled"));
    } else {
        KMessageBox::enableMessage( QString::fromLatin1("mplayerNotInstalled") );

        if ( FeatureDialog::ffmpegBinary().isEmpty() && !FeatureDialog::isMplayer2() ) {
            KMessageBox::information( this,
                                      i18n("<p>You have MPlayer installed on your system, but it is unfortunately not version 2. "
                                           "MPlayer2 is on most systems a separate package, please install that if at all possible, "
                                           "as that version has much better support for extracting thumbnails from videos.</p>"),
                                      i18n("MPlayer is too old"), QString::fromLatin1("mplayerVersionTooOld"));
        } else
            KMessageBox::enableMessage( QString::fromLatin1("mplayerVersionTooOld") );
    }
}

bool MainWindow::Window::anyVideosSelected() const
{
    Q_FOREACH(const DB::FileName& fileName, selected()) {
        if ( Utilities::isVideo(fileName))
            return true;
    }
    return false;
}

void MainWindow::Window::announceAndroidVersion()
{
    // Don't bother people with this information when they are starting KPA the first time
    if (DB::ImageDB::instance()->totalCount() < 100)
        return;

    const QString doNotShowKey = QString::fromLatin1( "announce_android_version_key" );
    const QString txt = i18n("<p>Did you know that there is an Android client for KPhotoAlbum?<br/>"
                             "With the Android client you can view your images from your desktop.</p>"
                             "<p><a href=\"https://www.youtube.com/results?search_query=kphotoalbum+on+android\">See youtube video</a> or "
                             "<a href=\"https://play.google.com/store/apps/details?id=org.kde.kphotoalbum\">install from google play</a></p>" );
    KMessageBox::information( this, txt, QString(), doNotShowKey, KMessageBox::AllowLink );
}

void MainWindow::Window::setHistogramVisibilty( bool visible ) const
{
    if (visible)
    {
        m_dateBar->show();
        m_dateBarLine->show();
    }
    else
    {
        m_dateBar->hide();
        m_dateBarLine->hide();
    }
}

void MainWindow::Window::slotImageRotated(const DB::FileName& fileName)
{
    // An image has been rotated by the annotation dialog or the viewer.
    // We have to reload the respective thumbnail to get it in the right angle
    ImageManager::ThumbnailCache::instance()->removeThumbnail(fileName);
}

bool MainWindow::Window::dbIsDirty() const
{
    return m_statusBar->mp_dirtyIndicator->isSaveDirty();
}

#ifdef HAVE_KGEOMAP
void MainWindow::Window::showPositionBrowser()
{
    Browser::PositionBrowserWidget *positionBrowser = positionBrowserWidget();
    m_stack->setCurrentWidget(positionBrowser);
    updateStates( false );
}

Browser::PositionBrowserWidget* MainWindow::Window::positionBrowserWidget()
{
    if (m_positionBrowser == 0) {
        m_positionBrowser = createPositionBrowser();
    }
    return m_positionBrowser;
}

Browser::PositionBrowserWidget* MainWindow::Window::createPositionBrowser()
{
    Browser::PositionBrowserWidget* widget = new Browser::PositionBrowserWidget(m_stack);
    m_stack->addWidget(widget);
    return widget;
}
#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
