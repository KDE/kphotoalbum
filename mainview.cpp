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

MainView::MainView( QWidget* parent, const char* name )
    :MainViewUI( parent,  name )
{
    _optionsDialog = 0;
    _imageConfigure = 0;

    QString directory = "/home/blackie/Images";

    // Load the information from the XML file.
    QString xmlFile = directory + "/index.xml";
    QMap<QString, QDomElement> map;
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
                if ( fileName.isNull() )  {
                    qWarning( "Element did not contain a file attirbute" );
                }
                else {
                    map[fileName] = elm;
                }
            }
        }
    }

    QDir dir( directory );
    QStringList dirList = dir.entryList();
    for( QStringList::Iterator it = dirList.begin(); it != dirList.end(); ++it ) {
        if ( (*it) != "." && (*it) != ".." &&
             ( (*it).endsWith( ".jpg" ) || (*it).endsWith( ".jpeg" ) || (*it).endsWith( ".png" ) ||
               (*it).endsWith( ".tiff" ) || (*it).endsWith( ".gif" ) ) ) {
            ImageInfo* info = new ImageInfo( directory + "/" + *it, map[*it] );
            _images.append(info);
        }
    }
    thumbNailView->load( &_images );
}

void MainView::slotExit()
{
    qApp->quit();
}

void MainView::slotOptions()
{
    if ( ! _optionsDialog ) {
        _optionsDialog = new OptionsDialog( this );
        connect( _optionsDialog, SIGNAL( changed() ), thumbNailView, SLOT( reload() ) );
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
    if ( ! _imageConfigure ) {
        _imageConfigure = new ImageConfig( this,  "_imageConfigure" );
    }

    ImageInfoList list;
    for ( QIconViewItem* item = thumbNailView->firstItem(); item; item = item->nextItem() ) {
        if ( item->isSelected() ) {
            ThumbNail* tn = dynamic_cast<ThumbNail*>( item );
            Q_ASSERT( tn );
            list.append( tn->imageInfo() );
        }
    }
    if ( list.count() == 0 )  {
        QMessageBox::warning( this,  tr("No Selection"),  tr("No item selected.") );
    }
    else {
        _imageConfigure->configure( list,  oneAtATime );
        save();
    }
}

void MainView::slotSearch()
{
    if ( ! _imageConfigure ) {
        _imageConfigure = new ImageConfig( this,  "_imageConfigure" );
    }
    int ok = _imageConfigure->search();
    if ( ok == QDialog::Accepted )  {
        _curView.clear();
        for( ImageInfoListIterator it( _images ); *it; ++it ) {
            if ( _imageConfigure->match( *it ) )
                 _curView.append( *it );
        }
        thumbNailView->load( &_curView );
    }
}

void MainView::save()
{
    QMap<QString, QDomDocument> docs;
    for( QPtrListIterator<ImageInfo> it( _images ); *it; ++it ) {
        QString fileName = (*it)->fileName();
        QString outputFile = QFileInfo( fileName ).dirPath() + "/index.xml";
        if ( !docs.contains( outputFile ) )  {
            QDomDocument tmp;
            tmp.setContent( QString("<Images/>") );
            docs[outputFile] = tmp;
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
            stream << it.data().toString();
            out.close();
        }
    }
}

void MainView::slotDeleteSelected()
{
    qDebug("NYI!");
}

