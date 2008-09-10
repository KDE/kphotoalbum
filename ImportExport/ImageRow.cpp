#include "ImageRow.h"
#include "KimFileReader.h"
#include "ImportDialog.h"
#include <QCheckBox>
#include "MiniViewer.h"
#include <QImage>
#include <kio/netaccess.h>
#include "MainWindow/Window.h"

using namespace ImportExport;

ImageRow::ImageRow( DB::ImageInfoPtr info, ImportDialog* import, KimFileReader* kimFileReader, QWidget* parent )
    : QObject( parent ), m_info( info ), m_import(import), m_kimFileReader( kimFileReader )
{
    m_checkbox = new QCheckBox( QString::null, parent );
    m_checkbox->setChecked( true );
}

void ImageRow::showImage()
{
    if ( m_import->_externalSource ) {
        KUrl src1 = m_import->_kimFile;
        KUrl src2 = m_import->_baseUrl + QString::fromLatin1( "/" );
        for ( int i = 0; i < 2; ++i ) {
            // First try next to the .kim file, then the external URL
            KUrl src = src1;
            if ( i == 1 )
                src = src2;
            src.setFileName( m_info->fileName( DB::RelativeToImageRoot ) );
            QString tmpFile;

            if( KIO::NetAccess::download( src, tmpFile, MainWindow::Window::theMainWindow() ) ) {
                QImage img( tmpFile );
                MiniViewer::show( img, m_info, static_cast<QWidget*>( parent() ) );
                KIO::NetAccess::removeTempFile( tmpFile );
                break;
            }
        }
    }
    else {
        QImage img = QImage::fromData(m_kimFileReader->loadImage( m_info->fileName( DB::RelativeToImageRoot) ) );
        MiniViewer::show( img, m_info, static_cast<QWidget*>( parent() ) );
    }
}

