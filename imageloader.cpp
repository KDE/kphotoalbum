#include "imageloader.h"
#include <qwaitcondition.h>
#include "imagemanager.h"
#include "thumbnail.h"
#include <qfileinfo.h>
#include <qapplication.h>
#include "options.h"
#include <qdir.h>

extern "C" {
#define XMD_H // prevent INT32 clash from jpeglib
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
            QImage img;
            bool imageLoaded = false;

            QString cacheDir = QFileInfo( li.fileName() ).dirPath() + QString::fromLatin1("/ThumbNails");
            QString cacheFile = cacheDir + QString::fromLatin1("/%1x%2-%3-%4")
                                .arg(li.width()).arg(li.height())
                                .arg( li.angle()).arg( QFileInfo( li.fileName() ).baseName() );
            // Try to load thumbernail from cache
            if ( QFileInfo( cacheFile ).exists() ) {
                if ( img.load( cacheFile ) )  {
                    imageLoaded = true;
                }
            }

            if ( !imageLoaded && QFile( li.fileName() ).exists() ) {
                img.load( li.fileName() );

                if ( li.angle() != 0 )  {
                    QWMatrix matrix;
                    matrix.rotate( li.angle() );
                    img = img.xForm( matrix );
                }

                // If we are looking for a scaled version, then scale
                if ( li.width() != -1 && li.height() != -1 )
                    img = img.scale( li.width(), li.height(), QImage::ScaleMin );

                // Save thumbnail to disk
                if ( ! QDir( cacheDir ).exists() ) {
                    QDir().mkdir( cacheDir, true );
                }
                img.save( cacheFile, "JPEG" );
                imageLoaded = true;
            }

            ImageEvent* iew = new ImageEvent( li, img );
            QApplication::postEvent( ImageManager::instance(),  iew );
        }
        else
            _sleeper->wait();
    }
}

QImage ImageLoader::rotateAndScale( QImage img, int width, int height, int angle )
{
    if ( angle != 0 )  {
        QWMatrix matrix;
        matrix.rotate( angle );
        img = img.xForm( matrix );
    }
    img = img.scale( width, height, QImage::ScaleMin );
    return img;
}
