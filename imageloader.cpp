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

            QString cacheDir = QFileInfo( li.fileName() ).dirPath() + "/ThumbNails";
            QString cacheFile = cacheDir + QString("/%1x%2-%3-%4").arg(li.width()).arg(li.height()).arg( li.angle()).arg( QFileInfo( li.fileName() ).baseName() );
            // Try to load thumbernail from cache
            QImage img;
            if ( QFileInfo( cacheFile ).exists() ) {
                if ( img.load( cacheFile ) )  {
                    ImageEvent* iew = new ImageEvent( li, img );
                    QApplication::postEvent( ImageManager::instance(),  iew );
                    continue; // Done.
                }
            }

            img.load( li.fileName() );

            if ( li.angle() != 0 )  {
                QWMatrix matrix;
                matrix.rotate( li.angle() );
                img = img.xForm( matrix );
            }

            // If we are looking for a scaled version, then scale
            if ( li.width() != -1 && li.height() != -1 )
                img = img.scale( li.width(), li.height(), QImage::ScaleMin );

            // should we compress the image, this is needed for thumbnail overview of say 2500 images
            if ( li.compress() )
                img = img.convertDepth(8);

            // Save thumbnail to disk
            if (  Options::instance()->cacheThumbNails() && li.cache() ) {
                if ( ! QDir( cacheDir ).exists() ) {
                    QDir().mkdir( cacheDir, true );
                }
                img.save( cacheFile, "JPEG" );
            }

            ImageEvent* iew = new ImageEvent( li, img );
            QApplication::postEvent( ImageManager::instance(),  iew );
        }
        else
            _sleeper->wait();
    }
}
