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

            QString cacheDir = QFileInfo( li.fileName() ).dirPath() + "/ThumbNails";
            QString cacheFile = cacheDir + QString("/%1x%2-%3-%4").arg(li.width()).arg(li.height())
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
                if (  Options::instance()->cacheThumbNails() && li.cache() ) {
                    if ( ! QDir( cacheDir ).exists() ) {
                        QDir().mkdir( cacheDir, true );
                    }
                    img.save( cacheFile, "JPEG" );
                }
                imageLoaded = true;
            }

            // should we compress the image, this is needed for thumbnail overview of say 2500 images
            if ( imageLoaded && li.compress() )
                img = img.convertDepth(8);

            ImageEvent* iew = new ImageEvent( li, img );
            QApplication::postEvent( ImageManager::instance(),  iew );
        }
        else
            _sleeper->wait();
    }
}
