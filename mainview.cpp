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
#include <wellcomedialog.h>
#include <qcursor.h>
#include "showbusycursor.h"

MainView::MainView( QWidget* parent, const char* name )
    :MainViewUI( parent,  name ), _dirty( false )
{
    _optionsDialog = 0;

    // For modality to work, we need to create these here.
    // Otherwise we would have this problem:
    // The image config is brough up, from there we start the viewer
    // The viewer was, however, already created, so all that is happening is a show
    // The viewer is now not a child of the image config, and is therefore inaccessible
    // given that the image config is modal.
    _imageConfigure = new ImageConfig( this,  "_imageConfigure" );
    connect( _imageConfigure, SIGNAL( changed() ), this, SLOT( slotChanges() ) );
    (void) Viewer::instance( _imageConfigure );

    if ( Options::configFileExists() )
        load();
    else
        wellcome();
    connect( thumbNailView, SIGNAL( changed() ), this, SLOT( slotChanges() ) );
}

bool MainView::slotExit()
{
    if ( _dirty || !thumbNailView->isClipboardEmpty() ) {
        int ret = QMessageBox::warning( this, "Save Changes", "Save Changes?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );
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
        connect( _optionsDialog, SIGNAL( changed() ), thumbNailView, SLOT( reload() ) );
        connect( _optionsDialog, SIGNAL( imagePathChanged() ), this, SLOT( load() ) );
    }
    _optionsDialog->exec();
    Options::instance()->save();
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
        QMessageBox::warning( this,  tr("No Selection"),  tr("No item selected.") );
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
    if ( ok == QDialog::Accepted )  {
        ShowBusyCursor dummy;
        for( ImageInfoListIterator it( _images ); *it; ++it ) {
            (*it)->setVisible( _imageConfigure->match( *it ) );
        }
        thumbNailView->reload();
    }
}

void MainView::slotSave()
{
    ShowBusyCursor dummy;

    QMap<QString, QDomDocument> docs;
    ImageInfoList list = _images;
    if ( !thumbNailView->isClipboardEmpty() ) {
        ImageInfoList clip= thumbNailView->clipboard();
        for( ImageInfoListIterator it(clip); *it; ++it ) {
            list.append( *it );
        }
    }

    for( ImageInfoListIterator it( list ); *it; ++it ) {
        QString indexDirectory = (*it)->indexDirectory();

        QString outputFile = indexDirectory + "/index.xml";
        if ( !docs.contains( outputFile ) )  {
            QDomDocument tmp;
            docs[outputFile] = tmp;
            // PENDING(blackie) The user should be able to specify the coding himself.
            tmp.appendChild( tmp. createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
            tmp.appendChild( tmp.createElement( "Images" ) );
        }
        QDomDocument doc = docs[outputFile];
        QDomElement elm = doc.documentElement();
        elm.appendChild( (*it)->save( doc ) );
    }

    for( QMapIterator<QString,QDomDocument> it= docs.begin(); it != docs.end(); ++it ) {
        QFile out( it.key() );

        if ( !out.open( IO_WriteOnly ) )  {
            qWarning( "Could not open file '%s'", it.key().latin1() );
        }
        else {
            QTextStream stream( &out );
            stream << it.data().toString().utf8();
            out.close();
        }
    }
    _dirty = false;
}

void MainView::slotDeleteSelected()
{
    qDebug("NYI!");
}

void MainView::load()
{
    ShowBusyCursor dummy;
    _images.clear();

    QString directory = Options::instance()->imageDirectory();
    if ( directory.isEmpty() )
        return;
    if ( directory.endsWith( "/" ) )
        directory = directory.mid( 0, directory.length()-1 );


    // Load the information from the XML file.
    QDict<void> loadedFiles( 6301 /* a large prime */ );

    QString xmlFile = directory + "/index.xml";
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

                QString fileName = elm.attribute( "file" );
                if ( fileName.isNull() )
                    qWarning( "Element did not contain a file attirbute" );
                else if ( ! QFileInfo( directory + "/" + fileName ).exists() )
                    qWarning( "File %s didn't exists", fileName.latin1());
                else if ( loadedFiles.find( fileName ) != 0 )
                    qWarning( "XML file contained image %s, more than ones - only first one will be loaded", fileName.latin1());
                else {
                    loadedFiles.insert( directory + "/" + fileName, (void*)0x1 /* void pointer to nothing I never need the value,
                                                                                  just its existsance, must be != 0x0 though.*/ );
                    load( directory, fileName, elm );
                }
            }
        }
    }

    loadExtraFiles( loadedFiles, directory, directory );
    thumbNailView->load( &_images );
}

void MainView::load( const QString& indexDirectory,  const QString& fileName, QDomElement elm )
{
    ImageInfo* info = new ImageInfo( indexDirectory, fileName, elm );
    _images.append(info);
}

void MainView::loadExtraFiles( const QDict<void>& loadedFiles, const QString& indexDirectory, QString directory )
{
    if ( directory.endsWith( "/" ) )
        directory = directory.mid( 0, directory.length()-1 );
    QDir dir( directory );
    QStringList dirList = dir.entryList();
    for( QStringList::Iterator it = dirList.begin(); it != dirList.end(); ++it ) {
        QString file = directory + "/" + *it;
        QFileInfo fi( file );
        if ( (*it) == "." || (*it) == ".." || (*it) == "ThumbNails" || !fi.isReadable() )
                continue;

        if ( fi.isFile() && (loadedFiles.find( file ) == 0) &&
             ( (*it).endsWith( ".jpg" ) || (*it).endsWith( ".jpeg" ) || (*it).endsWith( ".png" ) ||
                 (*it).endsWith( ".tiff" ) || (*it).endsWith( ".gif" ) ) )  {
            QString baseName = file.mid( indexDirectory.length()+1 );

            ImageInfo* info = new ImageInfo( indexDirectory, baseName  );
            if (Options::instance()->markNew() )
                info->addOption( "keyWords", QStringList() << "new" );

            _images.append(info);
        }
        else if ( fi.isDir() )  {
            loadExtraFiles( loadedFiles,  indexDirectory, file );
        }
    }
}

ImageInfoList MainView::selected()
{
    ImageInfoList list;
    for ( QIconViewItem* item = thumbNailView->firstItem(); item; item = item->nextItem() ) {
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
    if ( list.count() == 0 )
        QMessageBox::warning( this,  tr("No Selection"),  tr("No item selected.") );
    else {
        Viewer* viewer = Viewer::instance();
        viewer->load( list );
        viewer->show();
    }
}

void MainView::wellcome()
{
    WellComeDialog* dialog = new WellComeDialog( this );
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
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->setVisible( true );
    }
    thumbNailView->reload();
}

#include "mainview.moc"
