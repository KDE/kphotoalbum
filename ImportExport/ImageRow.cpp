#include "ImageRow.h"
#include "ImportDialog.h"
#include <QCheckBox>
#include "MiniViewer.h"
#include <QImage>
#include <kio/netaccess.h>
#include "MainWindow/Window.h"

using namespace ImportExport;

ImageRow::ImageRow( DB::ImageInfoPtr info, ImportDialog* import, QWidget* parent )
    : QObject( parent ), _info( info ), _import( import )
{
    _checkbox = new QCheckBox( QString::null, parent );
    _checkbox->setChecked( true );
}

void ImageRow::showImage()
{
    if ( _import->_externalSource ) {
        KUrl src1 =_import->_kimFile;
        KUrl src2 = _import->_baseUrl + QString::fromLatin1( "/" );
        for ( int i = 0; i < 2; ++i ) {
            // First try next to the .kim file, then the external URL
            KUrl src = src1;
            if ( i == 1 )
                src = src2;
            src.setFileName( _info->fileName( true ) );
            QString tmpFile;

            if( KIO::NetAccess::download( src, tmpFile, MainWindow::Window::theMainWindow() ) ) {
                QImage img( tmpFile );
                MiniViewer::show( img, _info, static_cast<QWidget*>( parent() ) );
                KIO::NetAccess::removeTempFile( tmpFile );
                break;
            }
        }
    }
    else {
        QImage img = QImage::fromData(_import->loadImage( _info->fileName(true) ) );
        MiniViewer::show( img, _info, static_cast<QWidget*>( parent() ) );
    }
}

