#include "imageloader.h"
#include <qwaitcondition.h>
#include "imagemanager.h"
#include "thumbnail.h"
#include <qfileinfo.h>
#include <qapplication.h>
#include "options.h"
#include <qdir.h>

extern "C" {
#include <jpeglib.h>
#include <stdio.h>
}

#include <qwmatrix.h>

ImageLoader::ImageLoader( QWaitCondition* sleeper )
    : _sleeper( sleeper )
{
}

void ImageLoader::run()
{
    while ( true ) {
        LoadInfo li = ImageManager::instance()->next();

        if ( !li.isNull() ) {

            QString cacheDir = QFileInfo( li.fileName() ).dirPath() + "/ThumbNails";
            QString cacheFile = cacheDir + QString("/%1x%2-%3").arg(li.width()).arg(li.height()).arg( QFileInfo( li.fileName() ).baseName() );
            if ( li.width() != -1 )  {
                // Try to load thumbernail from cache
                QImage img;
                if ( QFileInfo( cacheFile ).exists() ) {
                    if ( img.load( cacheFile ) )  {
                        ImageEvent* iew = new ImageEvent( li, img );
                        QApplication::postEvent( ImageManager::instance(),  iew );
                        continue; // Done.
                    }
                }
            }

            QImage orig;
            if (!isJPEG(li) || !loadJPEG(&orig, li)) {
                orig.load( li.fileName() );
            }


            // If we are looking for a scaled version, then scale
            if ( li.width() != -1 )  {
                QImage scaled = orig.scale( li.width(), li.height(), QImage::ScaleMin );

                // Save thumbnail to disk
                if (  Options::instance()->cacheThumbNails() ) {
                    if ( ! QDir( cacheDir ).exists() ) {
                        QDir().mkdir( cacheDir, true );
                    }
                    scaled.save( cacheFile, "JPEG" );
                    }

                if ( li.angle() != 0 )  {
                    QWMatrix matrix;
                    matrix.rotate( li.angle() );
                    scaled = scaled.xForm( matrix );
                }

                ImageEvent* iew = new ImageEvent( li, scaled );
                QApplication::postEvent( ImageManager::instance(),  iew );
            }
        }
        else
            _sleeper->wait();
    }
}


bool ImageLoader::isJPEG( const LoadInfo& li )
{
    QString format=QImageIO::imageFormat( li.fileName() );
    return format=="JPEG";
}

// Fudged Fast JPEG decoding code from GWENVIEW (picked out out digikam)

bool ImageLoader::loadJPEG(QImage* image, const LoadInfo& li )
{
    FILE* inputFile=fopen(li.fileName().data(), "rb");
    if(!inputFile) return false;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, inputFile);
    jpeg_read_header(&cinfo, TRUE);

    int size = li.width();
    int imgSize = QMAX(cinfo.image_width, cinfo.image_height);

    int scale=1;
    while(size*scale*2<=imgSize) {
        scale*=2;
    }
    if(scale>8) scale=8;

    cinfo.scale_num=1;
    cinfo.scale_denom=scale;

    // Create QImage
    jpeg_start_decompress(&cinfo);

    switch(cinfo.output_components) {
    case 3:
    case 4:
        image->create( cinfo.output_width, cinfo.output_height, 32 );
        break;
    case 1: // B&W image
        image->create( cinfo.output_width, cinfo.output_height, 8, 256 );
        for (int i=0; i<256; i++)
            image->setColor(i, qRgb(i,i,i));
        break;
    default:
        return false;
    }

    uchar** lines = image->jumpTable();
    while (cinfo.output_scanline < cinfo.output_height)
        jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline,
                            cinfo.output_height);
    jpeg_finish_decompress(&cinfo);

    // Expand 24->32 bpp
    if ( cinfo.output_components == 3 ) {
        for (uint j=0; j<cinfo.output_height; j++) {
            uchar *in = image->scanLine(j) + cinfo.output_width*3;
            QRgb *out = (QRgb*)( image->scanLine(j) );

            for (uint i=cinfo.output_width; i--; ) {
                in-=3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }

    //int newMax = QMAX(cinfo.output_width, cinfo.output_height);
    //int newx = size*cinfo.output_width / newMax;
    //int newy = size*cinfo.output_height / newMax;

    //image=image.smoothScale( newx, newy);

    jpeg_destroy_decompress(&cinfo);
    fclose(inputFile);

    return true;
}
