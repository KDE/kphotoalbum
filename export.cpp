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
#include <kdialogbase.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

void Export::imageExport( const ImageInfoList& list )
{
    ExportConfig config;
    config.exec();

    int maxSize = -1;
    if ( config._enforeMaxSize->isChecked() )
        maxSize = config._maxSize->value();

    new Export( list, config._compress->isChecked(), maxSize );
}

ExportConfig::ExportConfig()
    : KDialogBase( KDialogBase::Plain, i18n("Export Configuration"), KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help,
                   KDialogBase::Ok, 0, "export config" )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    _compress = new QCheckBox( i18n("Compress Export File"), top );
    lay1->addWidget( _compress );

    QHBoxLayout* lay2 = new QHBoxLayout( lay1, 6 );
    _enforeMaxSize = new QCheckBox( i18n( "Limit maximum dimension of images to: " ), top, "_enforeMaxSize" );
    lay2->addWidget( _enforeMaxSize );

    _maxSize = new QSpinBox( 100, 4000, 50, top, "_maxSize" );
    lay2->addWidget( _maxSize );
    _maxSize->setValue( 800 );

    connect( _enforeMaxSize, SIGNAL( toggled( bool ) ), _maxSize, SLOT( setEnabled( bool ) ) );
    _maxSize->setEnabled( false );

    connect( this, SIGNAL( helpClicked() ), this, SLOT( slotHelp() ) );

    QString txt = i18n( "<qt><p>If your images are stored in a non compressed file format, then you may check this, "
                        "otherwise, this is just a waste of time during import and export operations.</p>"
                        "<p>In other words, don't check this if your images are stored in jpg, png, gif, but do check this "
                        "if your images are stored in tiff.</p></qt>" );
    QWhatsThis::add( _compress, txt );

    txt = i18n( "<qt><p>With this option you may limit the maximum dimension (widh or hight) of your images. "
                "Doing so will make the resulting export file smaller, but will of course also make the quality "
                "worse if someone wants to see the images in a larger dimension.</p></qt>" );
    QWhatsThis::add( _enforeMaxSize, txt );
    QWhatsThis::add( _maxSize, txt );
}

void ExportConfig::slotHelp()
{
    QWhatsThis::enterWhatsThisMode();
}


Export::Export( const ImageInfoList& list, bool compress, int maxSize ) : _ok( true ), _maxSize( maxSize )
{
    // Ask for zip file name, and create it.
    QString zipFile = KFileDialog::getSaveFileName( QString::null, QString::fromLatin1( "*.kim|KimDaBa export files" ), 0 );
    if ( zipFile.isNull() )
        return;

    _zip = new KZip( zipFile );
    _zip->setCompression( compress ? KZip::DeflateCompression : KZip::NoCompression );
    if ( ! _zip->open( IO_WriteOnly ) ) {
        KMessageBox::error( 0, i18n("Error creating zip file") );
        return;
    }

    // Create progress dialog
    int total = 2 * list.count(); // number of images * ( createh thumbnails + copy image )
    _steps = 0;
    _progressDialog = new QProgressDialog( QString::null, i18n("Cancel"), total, 0, "progress dialog", true );
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



void Export::generateThumbnails( const ImageInfoList& list )
{
    _progressDialog->setLabelText( i18n("Creating thumbnails") );
    _loopEntered = false;
    _subdir = QString::fromLatin1( "Thumbnails/" );
    _filesRemaining = list.count(); // Used to break the event loop.
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        ImageManager::instance()->load( (*it)->fileName(), this, (*it)->angle(), 128, 128, false, true );
    }
    if ( _filesRemaining > 0 ) {
        _loopEntered = true;
        qApp->eventLoop()->enterLoop();
    }
}

void Export::copyImages( const ImageInfoList& list )
{
    _loopEntered = false;
    _subdir = QString::fromLatin1( "Images/" );

    _progressDialog->setLabelText( i18n("Copying image files") );

    int index = 0;
    _filesRemaining = 0;
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        QString file =  (*it)->fileName();
        QString zippedName = QString::fromLatin1( "image%1.%2" ).arg( Util::pad(6,++index) ).arg( QFileInfo( file).extension() );
        _map.insert( file, zippedName );

        if ( _maxSize == -1 ) {
            if ( QFileInfo( file ).isSymLink() )
                file = QFileInfo(file).readLink();

            _zip->addLocalFile( file, QString::fromLatin1( "Images/" ) + zippedName );
            _steps++;
            _progressDialog->setProgress( _steps );
        }
        else {
            _filesRemaining++;
            ImageManager::instance()->load( (*it)->fileName(), this, (*it)->angle(), _maxSize, _maxSize, false, true );
        }

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
    if ( _filesRemaining > 0 ) {
        _loopEntered = true;
        qApp->eventLoop()->enterLoop();
    }
}

void Export::pixmapLoaded( const QString& fileName, int /*width*/, int /*height*/, int /*angle*/, const QImage& image )
{
    // Add the file to the zip archive
    QString zipFileName = _subdir + QFileInfo( _map[fileName] ).baseName() + QString::fromLatin1(".jpg");
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


