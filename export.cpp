/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
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

#include "export.h"
#include <kfiledialog.h>
#include <kzip.h>
#include <qfileinfo.h>
#include <time.h>
#include "util.h"
#include <qprogressdialog.h>
#include <qeventloop.h>
#include <klocale.h>
#include "imagemanager.h"
#include <qapplication.h>
#include <kmessagebox.h>

void Export::imageExport( const ImageInfoList& list )
{
    new Export( list );
}


QCString Export::createIndexXML( const ImageInfoList& list )
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );

    QDomElement top = doc.createElement( QString::fromLatin1( "KimDaBa-export" ) );
    doc.appendChild( top );


    for( ImageInfoListIterator it( list ); *it; ++it ) {
        QString file = (*it)->fileName();
        QString mappedFile = _map[file];
        QDomElement elm = (*it)->save( doc );
        elm.setAttribute( QString::fromLatin1( "file" ), mappedFile );
        top.appendChild( elm );
    }
    return doc.toCString();
}

Export::Export( const ImageInfoList& list ) : _ok( true )
{
    // Ask for zip file name, and create it.
    QString zipFile = KFileDialog::getSaveFileName( QString::null, QString::fromLatin1( "*.kim|KimDaBa export files" ), 0 );
    if ( zipFile.isNull() )
        return;

    _zip = new KZip( zipFile );
    if ( ! _zip->open( IO_WriteOnly ) ) {
        KMessageBox::error( 0, i18n("Error creating zip file") );
        return;
    }

    // Create progress dialog
    int total = 2 * list.count(); // number of images * ( createh thumbnails + copy image )
    _steps = 0;
    _progressDialog = new QProgressDialog( i18n("Generating Thumbnails"), i18n("Cancel"), total, 0, "progress dialog", true );
    _progressDialog->setProgress( 0 );
    _progressDialog->show();

    // Copy image files and generate thumbnails
    copyImages( list );
    if ( _ok )
        generateThumbnails( list );

    if ( _ok ) {
        // Create the index.xml file
        QCString indexml = createIndexXML( list );
        time_t t;
        time(&t);
        _zip->writeFile( QString::fromLatin1( "index.xml" ), QString::null, QString::null, indexml.size()-1,
                         0444, t, t, t, indexml.data() );

        _zip->close();
    }
}


void Export::generateThumbnails( const ImageInfoList& list )
{
    _filesRemaining = list.count(); // Used to break the event loop.
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        ImageManager::instance()->load( (*it)->fileName(), this, (*it)->angle(), 128, 128, false, true );
    }
    qApp->eventLoop()->enterLoop();
}

void Export::copyImages( const ImageInfoList& list )
{
    _progressDialog->setLabelText( i18n("Copying image files") );

    int index = 0;
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        QString file =  (*it)->fileName();
        QString zippedName = QString::fromLatin1( "image%1.%2" ).arg( Util::pad(6,++index) ).arg( QFileInfo( file).extension() );
        _map.insert( file, zippedName );

        if ( QFileInfo( file ).isSymLink() )
            file = QFileInfo(file).readLink();

        _zip->addLocalFile( file, QString::fromLatin1( "Images/" ) + zippedName );
        _steps++;
        _progressDialog->setProgress( _steps );

        // Test if the cancel button was pressed.
        qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

#if QT_VERSION < 0x030104
        bool canceled = _progressDialog->wasCancelled();
#else
        bool canceled =  _progressDialog->wasCanceled();
#endif
        if ( canceled ) {
            _ok = false;
            return;
        }
    }
}

void Export::pixmapLoaded( const QString& fileName, int /*width*/, int /*height*/, int /*angle*/, const QImage& image )
{
    // Add the file to the zip archive
    QString zipFileName = QString::fromLatin1( "Thumbnails/" ) + QFileInfo( _map[fileName] ).baseName() + QString::fromLatin1(".jpg");
    QByteArray data;
    QBuffer buffer( data );
    buffer.open( IO_WriteOnly );
    image.save( &buffer, "JPEG" );

    _zip->writeFile( zipFileName, QString::null, QString::null, data.size(), data );

    qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

#if QT_VERSION < 0x030104
    bool canceled = _progressDialog->wasCancelled();
#else
    bool canceled =  _progressDialog->wasCanceled();
#endif

    if ( canceled ) {
        _ok = false;
        qApp->eventLoop()->exitLoop();
        ImageManager::instance()->stop( this );
        return;
    }

    _steps++;
    _filesRemaining--;
    _progressDialog->setProgress( _steps );

    if ( _filesRemaining == 0 )
        qApp->eventLoop()->exitLoop();
}
