/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "mainview.h"
#include <optionsdialog.h>
#include <qapplication.h>
#include "thumbnailview.h"
#include "thumbnail.h"
#include "imageconfig.h"
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qdict.h>
#include "viewer.h"
#include <welcomedialog.h>
#include <qcursor.h>
#include "showbusycursor.h"
#include <klocale.h>
#include <qhbox.h>
#include <qwidgetstack.h>
#include <kstandarddirs.h>
#include "htmlexportdialog.h"
#include <kstatusbar.h>
#include "imagecounter.h"
#include <qtimer.h>
#include <kmessagebox.h>
#include "options.h"
#include "browser.h"
#include "imagedb.h"
#include "util.h"
#include <kapplication.h>
#include <ktip.h>
#include <kprocess.h>
#include "deletedialog.h"
#include <ksimpleconfig.h>
#include <kcmdlineargs.h>
#include <qregexp.h>
#include <stdlib.h>
#include <qpopupmenu.h>

MainView::MainView( QWidget* parent, const char* name )
    :KMainWindow( parent,  name ), _imageConfigure(0), _dirty( false ), _deleteDialog( 0 )
{
    load();

    // To avoid a race conditions where both the image loader thread creates an instance of
    // Options, and where the main thread crates an instance, we better get it created now.
    connect( Options::instance(), SIGNAL( changed() ), this, SLOT( slotChanges() ) );

    _stack = new QWidgetStack( this );
    _browser = new Browser( _stack );
    connect( _browser, SIGNAL( showingOverview() ), this, SLOT( showBrowser() ) );
    connect( _browser, SIGNAL( pathChanged( const QString& ) ), this, SLOT( pathChanged( const QString& ) ) );
    _thumbNailView = new ThumbNailView( _stack );
    _thumbNailView->load( &ImageDB::instance()->images() );

    _stack->addWidget( _browser );
    _stack->addWidget( _thumbNailView );
    setCentralWidget( _stack );
    _stack->raiseWidget( _browser );

    // Setting up status bar
    ImageCounter* partial = new ImageCounter( statusBar() );
    statusBar()->addWidget( partial, 0, true );

    ImageCounter* total = new ImageCounter( statusBar() );
    statusBar()->addWidget( total, 0, true );

    _optionsDialog = 0;
    setupMenuBar();

    _autoSaveTimer = new QTimer( this );
    connect( _autoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
    startAutoSaveTimer();

    connect( ImageDB::instance(), SIGNAL( matchCountChange( int, int, int ) ),
             partial, SLOT( setMatchCount( int, int, int ) ) );
    connect( _browser, SIGNAL( showingOverview() ), partial, SLOT( showingOverview() ) );
    connect( ImageDB::instance(), SIGNAL( searchCompleted() ), this, SLOT( showThumbNails() ) );
    connect( Options::instance(), SIGNAL( optionGroupsChanged() ), this, SLOT( slotOptionGroupChanged() ) );

    // PENDING(blackie) ImageDB should emit a signal when total changes.
    total->setTotal( ImageDB::instance()->totalCount() );
    statusBar()->message(i18n("Welcome to KimDaba"), 5000 );

    KTipDialog::showTip( this );
}

bool MainView::slotExit()
{
    if ( _dirty || !ImageDB::instance()->isClipboardEmpty() ) {
        int ret = QMessageBox::warning( this, i18n("Save Changes?"),
                                        i18n("Do you want to save the changes?"),
                                        QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );
        if ( ret == QMessageBox::Cancel )
            return false;
        if ( ret == QMessageBox::Yes ) {
            slotSave();
        }
        if ( ret == QMessageBox::No ) {
            QDir().remove( Options::instance()->imageDirectory() + QString::fromLatin1("/.#index.xml") );
        }
    }

    qApp->quit();
    return true;
}

void MainView::slotOptions()
{
    if ( ! _optionsDialog ) {
        _optionsDialog = new OptionsDialog( this );
        connect( _optionsDialog, SIGNAL( changed() ), _thumbNailView, SLOT( reload() ) );
    }
    _optionsDialog->exec();
    startAutoSaveTimer(); // In case auto save period has changed, we better restart the timer.
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
    ImageInfoList list = selected();
    if ( list.count() == 0 )  {
        QMessageBox::warning( this,  i18n("No Selection"),  i18n("No item is selected.") );
    }
    else {
        createImageConfig();
        _imageConfigure->configure( list,  oneAtATime );
    }
}

void MainView::slotSearch()
{
    createImageConfig();
    ImageSearchInfo searchInfo = _imageConfigure->search();
    _browser->addSearch( searchInfo );
}

void MainView::createImageConfig()
{
    ShowBusyCursor dummy;
    if ( _imageConfigure )
        return;

    _imageConfigure = new ImageConfig( this,  "_imageConfigure" );
    connect( _imageConfigure, SIGNAL( changed() ), this, SLOT( slotChanges() ) );
    connect( _imageConfigure, SIGNAL( deleteOption( const QString&, const QString& ) ),
             this, SLOT( slotDeleteOption( const QString&, const QString& ) ) );
    connect( _imageConfigure, SIGNAL( renameOption( const QString& , const QString& , const QString&  ) ),
             this, SLOT( slotRenameOption( const QString& , const QString& , const QString&  ) ) );
}

void MainView::slotSave()
{
    statusBar()->message(i18n("Saving..."), 5000 );
    save( Options::instance()->imageDirectory() + QString::fromLatin1("/index.xml") );
    _dirty = false;
    QDir().remove( Options::instance()->imageDirectory() + QString::fromLatin1("/.#index.xml") );
    statusBar()->message(i18n("Saving... Done"), 5000 );
}

void MainView::save( const QString& fileName )
{
    ShowBusyCursor dummy;

    QDomDocument doc;

    // PENDING(blackie) The user should be able to specify the coding himself.
    doc.appendChild( doc. createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );
    QDomElement elm = doc.createElement( QString::fromLatin1("KimDaBa") );
    doc.appendChild( elm );

    Options::instance()->save( elm );
    ImageDB::instance()->save( elm );

    QFile out( fileName );

    if ( !out.open( IO_WriteOnly ) )
        KMessageBox::sorry( this, i18n( "Could not open file '%1'" ).arg( fileName ) );
    else {
        QTextStream stream( &out );
        stream << doc.toString().utf8();
        out.close();
    }
}


void MainView::slotDeleteSelected()
{
    if ( ! _deleteDialog )
        _deleteDialog = new DeleteDialog( this );
    if ( _deleteDialog->exec( selected() ) == QDialog::Accepted )
        _dirty = true;
    _thumbNailView->reload();
}


ImageInfoList MainView::selected()
{
    ImageInfoList list;
    for ( QIconViewItem* item = _thumbNailView->firstItem(); item; item = item->nextItem() ) {
        if ( item->isSelected() ) {
            ThumbNail* tn = dynamic_cast<ThumbNail*>( item );
            Q_ASSERT( tn );
            list.append( tn->imageInfo() );
        }
    }
    return list;
}

void MainView::slotViewSelectedNewWindow()
{
    slotViewSelected( false );
}

void MainView::slotViewSelected( bool reuse )
{
    ImageInfoList list = selected();
    ImageInfoList list2;
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        if ( (*it)->imageOnDisk() )
            list2.append( *it );
    }
    if ( list.count() == 0 )
        QMessageBox::warning( this,  i18n("No Selection"),  i18n("No item is selected.") );
    else if ( list2.count() == 0 )
        QMessageBox::warning( this, i18n("No Images to Display"),
                              i18n("None of the seleceted images were available on disk.") );
    else {
        Viewer* viewer;
        if ( reuse && Viewer::latest() ) {
            viewer = Viewer::latest();
            topLevelWidget()->raise();
            setActiveWindow();
        }
        else {
            // We don't want this to be child of anything. Originally it was child of the mainwindow
            // but that had the effect that it would always be on top of it.
            viewer = new Viewer( 0 );
            viewer->show();
        }
        viewer->load( list2 );
    }
}

QString MainView::welcome()
{
    WelComeDialog dialog( this );
    dialog.exec();
    return dialog.configFileName();
}

void MainView::slotChanges()
{
    _dirty = true;
}

void MainView::closeEvent( QCloseEvent* e )
{
    bool quit = true;
    quit = slotExit();
    // If I made it here, then the user canceled
    if ( !quit )
        e->ignore();
}


void MainView::slotLimitToSelected()
{
    ShowBusyCursor dummy;
    for ( QIconViewItem* item = _thumbNailView->firstItem(); item; item = item->nextItem() ) {
        ThumbNail* tn = dynamic_cast<ThumbNail*>( item );
        Q_ASSERT( tn );
        tn->imageInfo()->setVisible( item->isSelected() );
    }
    _thumbNailView->reload();
}

void MainView::setupMenuBar()
{
    // File menu
    KStdAction::save( this, SLOT( slotSave() ), actionCollection() );
    KStdAction::quit( this, SLOT( slotExit() ), actionCollection() );
    new KAction( i18n("Export to HTML..."), 0, this, SLOT( slotExportToHTML() ), actionCollection(), "exportHTML" );
    KAction* a = KStdAction::back( _browser, SLOT( back() ), actionCollection() );
    connect( _browser, SIGNAL( canGoBack( bool ) ), a, SLOT( setEnabled( bool ) ) );
    a->setEnabled( false );

    a = KStdAction::forward( _browser, SLOT( forward() ), actionCollection() );
    connect( _browser, SIGNAL( canGoForward( bool ) ), a, SLOT( setEnabled( bool ) ) );
    a->setEnabled( false );

    a = KStdAction::home( _browser, SLOT( home() ), actionCollection() );

    // The Edit menu
    KStdAction::cut( _thumbNailView, SLOT( slotCut() ), actionCollection() );
    KStdAction::paste( _thumbNailView, SLOT( slotPaste() ), actionCollection() );
    new KAction( i18n( "Options" ), CTRL+Key_O, this, SLOT( slotOptions() ),
                 actionCollection(), "options" );
    KStdAction::selectAll( _thumbNailView, SLOT( slotSelectAll() ), actionCollection() );
    KStdAction::find( this, SLOT( slotSearch() ), actionCollection() );
    new KAction( i18n( "Delete Selected" ), Key_Delete, this, SLOT( slotDeleteSelected() ),
                 actionCollection(), "deleteSelected" );
    new KAction( i18n( "&One at a Time" ), CTRL+Key_1, this, SLOT( slotConfigureImagesOneAtATime() ),
                 actionCollection(), "oneProp" );
    new KAction( i18n( "&All Simultaneously" ), CTRL+Key_2, this, SLOT( slotConfigureAllImages() ),
                 actionCollection(), "allProp" );

    // The Images menu
    new KAction( i18n("View Selected"), Key_I, this, SLOT( slotViewSelected() ),
                 actionCollection(), "viewImages" );
    new KAction( i18n("View Selected (In new window)"), CTRL+Key_I, this, SLOT( slotViewSelectedNewWindow() ),
                 actionCollection(), "viewImagesNewWindow" );
    new KAction( i18n("Limit View to Marked"), 0, this, SLOT( slotLimitToSelected() ),
                 actionCollection(), "limitToMarked" );

    // The help menu
    KStdAction::tipOfDay( this, SLOT(showTipOfDay()), actionCollection() );
    new KAction( i18n("Show Tooltips on Images"), CTRL+Key_T, _thumbNailView, SLOT( showToolTipsOnImages() ),
                 actionCollection(), "showToolTipOnImages" );
    new KAction( i18n("Run KimDaBa Demo"), 0, this, SLOT( runDemo() ),
                 actionCollection(), "runDemo" );

    connect( _thumbNailView, SIGNAL( changed() ), this, SLOT( slotChanges() ) );
    createGUI( QString::fromLatin1( "kimdabaui.rc" ) );
}

void MainView::slotExportToHTML()
{
    ImageInfoList list = selected();
    if ( list.count() == 0 )  {
        QMessageBox::warning( this,  i18n("No Selection"),  i18n("No item selected.") );
    }

    HTMLExportDialog dialog( list, this, "htmlExportDialog" );
    dialog.exec();
}

void MainView::slotDeleteOption( const QString& optionGroup, const QString& which )
{
    for( ImageInfoListIterator it( ImageDB::instance()->images() ); *it; ++it ) {
        (*it)->removeOption( optionGroup, which );
    }
    Options::instance()->removeOption( optionGroup, which );
    _dirty=true;
}


void MainView::slotRenameOption( const QString& optionGroup, const QString& oldValue, const QString& newValue )
{
    for( ImageInfoListIterator it( ImageDB::instance()->images() ); *it; ++it ) {
        (*it)->renameOption( optionGroup, oldValue, newValue );
    }
    Options::instance()->removeOption( optionGroup, oldValue );
    Options::instance()->addOption( optionGroup, newValue );
    _dirty = true;
}

void MainView::startAutoSaveTimer()
{
    int i = Options::instance()->autoSave();
    _autoSaveTimer->stop();
    if ( i != 0 ) {
        _autoSaveTimer->start( i * 1000 * 60 );
    }
}

void MainView::slotAutoSave()
{
    if ( _dirty ) {
        statusBar()->message(i18n("Auto saving...."));
        save( Options::instance()->imageDirectory() + QString::fromLatin1("/.#index.xml") );
        statusBar()->message(i18n("Auto saving.... Done"), 5000);
    }
}


void MainView::showThumbNails()
{
    _thumbNailView->reload();
    _stack->raiseWidget( _thumbNailView );
}

void MainView::showBrowser()
{
    _stack->raiseWidget( _browser );
}


void MainView::slotOptionGroupChanged()
{
    Q_ASSERT( !_imageConfigure || !_imageConfigure->isShown() );
    delete _imageConfigure;
    _imageConfigure = 0;
}

void MainView::showTipOfDay()
{
    KTipDialog::showTip( this, QString::null, true );
}

void MainView::pathChanged( const QString& path )
{
    static bool itemVisible = false;
    if ( path.isEmpty() ) {
        if ( itemVisible ) {
            statusBar()->removeItem( 0 );
            itemVisible = false;
        }
    }
    else if ( !itemVisible ) {
        statusBar()->insertItem( path, 0 );
        itemVisible = true;
    }
    else
        statusBar()->changeItem( path, 0 );

}

void MainView::runDemo()
{
    KProcess* process = new KProcess;
    *process << "kimdaba" << "-demo";
    process->start();
}

void MainView::load()
{

    // Let first try to find a config file.
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString configFile = QString::null;

    if ( args->isSet( "c" ) )
        configFile = args->getOption( "c" );
    else if ( args->isSet( "demo" ) )
        configFile = Util::setupDemo();
    else {
        KSimpleConfig config( QString::fromLatin1("kimdaba") );
        if ( config.hasKey( QString::fromLatin1("configfile") ) ) {
            configFile = config.readEntry( QString::fromLatin1("configfile") );
            if ( !QFileInfo( configFile ).exists() )
                configFile = welcome();
        }
        else
            configFile = welcome();
    }

    Util::checkForBackupFile( configFile );


    QDomDocument doc;
    QFile file( configFile );
    if ( !file.exists() ) {
        // Load a default setup
        QFile file( locate( "data", QString::fromLatin1( "kimdaba/default-setup" ) ) );
        if ( !file.open( IO_ReadOnly ) ) {
            KMessageBox::information( 0, i18n( "KimDaBa was unable to load a default setup, which indicates an installation error" ), i18n("No default setup file found") );
        }
        else {
            QTextStream stream( &file );
            QString str = stream.read();
            str = str.replace( QString::fromLatin1( "Persons" ), i18n( "Persons" ) );
            str = str.replace( QString::fromLatin1( "Locations" ), i18n( "Locations" ) );
            str = str.replace( QString::fromLatin1( "Keywords" ), i18n( "Keywords" ) );
            str = str.replace( QRegExp( QString::fromLatin1("imageDirectory=\"[^\"]*\"")), QString::fromLatin1("") );
            str = str.replace( QRegExp( QString::fromLatin1("htmlBaseDir=\"[^\"]*\"")), QString::fromLatin1("") );
            str = str.replace( QRegExp( QString::fromLatin1("htmlBaseURL=\"[^\"]*\"")), QString::fromLatin1("") );
            doc.setContent( str );
        }
    }
    else {
        if ( !file.open( IO_ReadOnly ) ) {
            KMessageBox::error( this, i18n("Unable to open '%1' for reading").arg( configFile ), i18n("Error running demo") );
            exit(-1);
        }

        QString errMsg;
        int errLine;
        int errCol;

        if ( !doc.setContent( &file, false, &errMsg, &errLine, &errCol )) {
            KMessageBox::error( this, i18n("Error on line %1 column %2 in file %3: %4").arg( errLine ).arg( errCol ).arg( configFile ).arg( errMsg ) );
            exit(-1);
        }
    }

    // Now read the content of the file.
    QDomElement top = doc.documentElement();
    if ( top.isNull() ) {
        KMessageBox::error( this, i18n("Error in file %1: No elements found").arg( configFile ) );
        exit(-1);
    }

    if ( top.tagName().lower() != QString::fromLatin1( "kimdaba" ) ) {
        KMessageBox::error( this, i18n("Error in file %1: expected 'KimDaBa' as top element but found '%2'").arg( configFile ).arg( top.tagName() ) );
        exit(-1);
    }

    QDomElement config;
    QDomElement options;
    QDomElement configWindowSetup;
    QDomElement images;
    QDomElement blockList;
    QDomElement memberGroups;

    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( node.isElement() ) {
            QDomElement elm = node.toElement();
            QString tag = elm.tagName().lower();
            if ( tag == QString::fromLatin1( "config" ) )
                config = elm;
            else if ( tag == QString::fromLatin1( "options" ) )
                options = elm;
            else if ( tag == QString::fromLatin1( "configwindowsetup" ) )
                configWindowSetup = elm;
            else if ( tag == QString::fromLatin1("images") )
                images = elm;
            else if ( tag == QString::fromLatin1( "blocklist" ) )
                blockList = elm;
            else if ( tag == QString::fromLatin1( "member-groups" ) )
                memberGroups = elm;
            else {
                KMessageBox::error( this, i18n("Error in file %1: unexpected element: '%2*").arg( configFile ).arg( tag ) );
            }
        }
    }

    if ( config.isNull() )
        KMessageBox::sorry( this, i18n("Didn't find 'Config' tag in configuration file %1").arg( configFile ) );
    if ( options.isNull() )
        KMessageBox::sorry( this, i18n("Didn't find 'Options' tag in configuration file %1").arg( configFile ) );
    if ( configWindowSetup.isNull() )
        KMessageBox::sorry( this, i18n("Didn't find 'ConfigWindowSetup' tag in configuration file %1").arg( configFile ) );
    if ( images.isNull() )
        KMessageBox::sorry( this, i18n("Didn't find 'Images' tag in configuration file %1").arg( configFile ) );

    file.close();

    Options::setup( config, options, configWindowSetup, memberGroups, QFileInfo( configFile ).dirPath( true ) );
    ImageDB::setup( images, blockList );
}

void MainView::contextMenuEvent( QContextMenuEvent* )
{
    QPopupMenu menu;
    int id1 = menu.insertItem( i18n( "Set properties one image at a time" ),
                               this, SLOT( slotConfigureImagesOneAtATime() ) );
    int id2 = menu.insertItem( i18n( "Set properties for all selected images" ),
                               this, SLOT( slotConfigureAllImages() ) );

    ImageInfoList sel = selected();
    if ( sel.count() <= 1 )
        menu.setItemEnabled( id2, false );
    if ( sel.count() == 0 )
        menu.setItemEnabled( id1, false );

    menu.exec( QCursor::pos() );
}


#include "mainview.moc"
