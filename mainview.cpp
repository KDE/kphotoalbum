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
#include "infopage.h"
#include "htmlexportdialog.h"
#include <kstatusbar.h>
#include "imagecounter.h"
#include <qtimer.h>
#include <kmessagebox.h>
#include "options.h"

MainView::MainView( QWidget* parent, const char* name )
    :KMainWindow( parent,  name ), _dirty( false )
{
    _stack = new QWidgetStack( this );
    _welcome = new InfoPage( _stack, "infopage" );
    _thumbNailView = new ThumbNailView( _stack );
    _stack->addWidget( _welcome );
    _stack->addWidget( _thumbNailView );
    setCentralWidget( _stack );
    _stack->raiseWidget( _welcome );

    _counter = new ImageCounter( statusBar() );
    statusBar()->addWidget( _counter, 0, true );

    _optionsDialog = 0;
    setupMenuBar();

    // For modality to work, we need to create these here.
    // Otherwise we would have this problem:
    // The image config is brough up, from there we start the viewer
    // The viewer was, however, already created, so all that is happening is a show
    // The viewer is now not a child of the image config, and is therefore inaccessible
    // given that the image config is modal.
    _imageConfigure = new ImageConfig( this,  "_imageConfigure" );
    connect( _imageConfigure, SIGNAL( changed() ), this, SLOT( slotChanges() ) );
    connect( _imageConfigure, SIGNAL( deleteOption( const QString&, const QString& ) ),
             this, SLOT( slotDeleteOption( const QString&, const QString& ) ) );
    connect( _imageConfigure, SIGNAL( renameOption( const QString& , const QString& , const QString&  ) ),
             this, SLOT( slotRenameOption( const QString& , const QString& , const QString&  ) ) );
    (void) Viewer::instance( _imageConfigure );

    if ( Options::configFileExists() )
        load();
    else
        welcome();
    _counter->setTotal( _images.count() );
    statusBar()->message(i18n("Welcome to KimDaba"), 5000 );

    _autoSaveTimer = new QTimer( this );
    connect( _autoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
    startAutoSaveTimer();
}

bool MainView::slotExit()
{
    if ( _dirty || !_thumbNailView->isClipboardEmpty() ) {
        int ret = QMessageBox::warning( this, i18n("Save Changes?"), i18n("Do you wnat to save the changes?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );
        if ( ret == QMessageBox::Cancel )
            return false;
        if ( ret == QMessageBox::Yes ) {
            slotSave();
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
        connect( _optionsDialog, SIGNAL( imagePathChanged() ), this, SLOT( load() ) );
    }
    _optionsDialog->exec();
    Options::instance()->save();
    startAutoSaveTimer();
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
        _imageConfigure->configure( list,  oneAtATime );
        Options::instance()->save();
    }
}

void MainView::slotSearch()
{
    if ( ! _imageConfigure ) {
        _imageConfigure = new ImageConfig( this,  "_imageConfigure" );
    }
    int ok = _imageConfigure->search();
    int count = 0;
    if ( ok == QDialog::Accepted )  {
        ShowBusyCursor dummy;
        for( ImageInfoListIterator it( _images ); *it; ++it ) {
            bool match = _imageConfigure->match( *it );
            (*it)->setVisible( match );
            if ( match )
                ++count;
        }
        _thumbNailView->reload();
    }
    _stack->raiseWidget( _thumbNailView );
   _counter->setPartial(count);
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

    ImageInfoList list = _images;

    // Copy files from clipboard to end of overview, so we don't loose them
    if ( !_thumbNailView->isClipboardEmpty() ) {
        ImageInfoList clip= _thumbNailView->clipboard();
        for( ImageInfoListIterator it(clip); *it; ++it ) {
            list.append( *it );
        }
    }

    // Open the output file
    QDomDocument doc;

    // PENDING(blackie) The user should be able to specify the coding himself.
    doc.appendChild( doc. createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );
    QDomElement elm = doc.createElement( QString::fromLatin1("Images") );
    doc.appendChild( elm );

    for( ImageInfoListIterator it( list ); *it; ++it ) {
        elm.appendChild( (*it)->save( doc ) );
    }

    QFile out( fileName );

    if ( !out.open( IO_WriteOnly ) )  {
        qWarning( "Could not open file '%s'", fileName.latin1() );
    }
    else {
        QTextStream stream( &out );
        stream << doc.toString().utf8();
        out.close();
    }
}

void MainView::slotDeleteSelected()
{
    qDebug("NYI!");
}

void MainView::load()
{
    checkForBackupFile();

    ShowBusyCursor dummy;
    _images.clear();

    QString directory = Options::instance()->imageDirectory();
    if ( directory.isEmpty() )
        return;
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );


    // Load the information from the XML file.
    QDict<void> loadedFiles( 6301 /* a large prime */ );

    QString xmlFile = directory + QString::fromLatin1("/index.xml");
    if ( QFileInfo( xmlFile ).exists() )  {
        QFile file( xmlFile );
        if ( ! file.open( IO_ReadOnly ) )  {
            qWarning( "Couldn't read file %s",  xmlFile.latin1() );
        }
        else {
            QDomDocument doc;
            doc.setContent( &file );
            for ( QDomNode node = doc.documentElement().firstChild(); !node.isNull(); node = node.nextSibling() )  {
                QDomElement elm;
                if ( node.isElement() )
                    elm = node.toElement();
                else
                    continue;

                QString fileName = elm.attribute( QString::fromLatin1("file") );
                if ( fileName.isNull() )
                    qWarning( "Element did not contain a file attribute" );
                else if ( loadedFiles.find( fileName ) != 0 )
                    qWarning( "XML file contained image %s, more than ones - only first one will be loaded", fileName.latin1());
                else {
                    loadedFiles.insert( directory + QString::fromLatin1("/") + fileName,
                                        (void*)0x1 /* void pointer to nothing I never need the value,
                                                      just its existsance, must be != 0x0 though.*/ );
                    load( fileName, elm );
                }
            }
        }
    }

    loadExtraFiles( loadedFiles, directory );
    _thumbNailView->load( &_images );
}

void MainView::load( const QString& fileName, QDomElement elm )
{
    ImageInfo* info = new ImageInfo( fileName, elm );
    info->setVisible( false );
    _images.append(info);
}

void MainView::loadExtraFiles( const QDict<void>& loadedFiles, QString directory )
{
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );
    QString imageDir = Options::instance()->imageDirectory();
    if ( imageDir.endsWith( QString::fromLatin1("/") ) )
        imageDir = imageDir.mid( 0, imageDir.length()-1 );
    QDir dir( directory );
    QStringList dirList = dir.entryList();
    for( QStringList::Iterator it = dirList.begin(); it != dirList.end(); ++it ) {
        QString file = directory + QString::fromLatin1("/") + *it;
        QFileInfo fi( file );
        if ( (*it) == QString::fromLatin1(".") || (*it) == QString::fromLatin1("..") || (*it) == QString::fromLatin1("ThumbNails") || !fi.isReadable() )
                continue;

        if ( fi.isFile() && (loadedFiles.find( file ) == 0) &&
             ( (*it).endsWith( QString::fromLatin1(".jpg") ) || (*it).endsWith( QString::fromLatin1(".jpeg") ) || (*it).endsWith( QString::fromLatin1(".png") ) ||
                 (*it).endsWith( QString::fromLatin1(".tiff") ) || (*it).endsWith( QString::fromLatin1(".gif") ) ) )  {
            QString baseName = file.mid( imageDir.length()+1 );

            ImageInfo* info = new ImageInfo( baseName  );
            _images.append(info);
        }
        else if ( fi.isDir() )  {
            loadExtraFiles( loadedFiles, file );
        }
    }
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

void MainView::slotViewSelected()
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
        Viewer* viewer = Viewer::instance();
        viewer->load( list2 );
        viewer->show();
    }
}

void MainView::welcome()
{
    WelComeDialog* dialog = new WelComeDialog( this );
    dialog->exec();
    delete dialog;
    slotOptions();
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


void MainView::slotShowAllThumbNails()
{
    ShowBusyCursor dummy;
    statusBar()->message(i18n("Showing all thumbnails"), 5000 );

    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->setVisible( true );
    }
    _thumbNailView->reload();
    _stack->raiseWidget( _thumbNailView );

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
    new KAction( i18n("View Selected"), CTRL+Key_I, this, SLOT( slotViewSelected() ),
                 actionCollection(), "viewImages" );
    new KAction( i18n("Limit View to Marked"), 0, this, SLOT( slotLimitToSelected() ),
                 actionCollection(), "limitToMarked" );
    new KAction( i18n("Display All Thumbnails"), 0, this, SLOT( slotShowAllThumbNails() ),
                 actionCollection(), "displayAllThumbs" );

    // The help menu
    new KAction( i18n("Show Tooltips on Images"), CTRL+Key_T, _thumbNailView, SLOT( showToolTipsOnImages() ),
                 actionCollection(), "showToolTipOnImages" );

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
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->removeOption( optionGroup, which );
    }
    Options::instance()->removeOption( optionGroup, which );
    _dirty=true;
}


void MainView::slotRenameOption( const QString& optionGroup, const QString& oldValue, const QString& newValue )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->renameOption( optionGroup, oldValue, newValue );
    }
    Options::instance()->removeOption( optionGroup, oldValue );
    Options::instance()->addOption( optionGroup, newValue );
    Options::instance()->save();
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
        save ( Options::instance()->imageDirectory() + QString::fromLatin1("/.#index.xml") );
        statusBar()->message(i18n("Auto saving.... Done"), 5000);
    }
}

void MainView::checkForBackupFile()
{
    QString backupNm = Options::instance()->imageDirectory() + QString::fromLatin1("/.#index.xml");
    QString indexNm = Options::instance()->imageDirectory() + QString::fromLatin1("/index.xml");
    QFileInfo backUpFile( backupNm);
    QFileInfo indexFile( indexNm );
    if ( !backUpFile.exists() || indexFile.lastModified() > backUpFile.lastModified() )
        return;

    int code = KMessageBox::questionYesNo( this, i18n("Backup file '%1' exists and is newer than '%2'. "
                                                      "Should I use the backup file?")
                                           .arg(backupNm).arg(indexNm),
                                           i18n("Found Backup File") );
    if ( code == KMessageBox::Yes ) {
        QFile in( backupNm );
        if ( in.open( IO_ReadOnly ) ) {
            QFile out( indexNm );
            if (out.open( IO_WriteOnly ) ) {
                char data[1024];
                int len;
                while ( (len = in.readBlock( data, 1024 ) ) )
                    out.writeBlock( data, len );
            }
        }
    }
}

#include "mainview.moc"
