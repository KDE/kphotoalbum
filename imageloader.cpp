/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "imageloader.h"
#include <qwaitcondition.h>
#include "imagemanager.h"
#include "thumbnail.h"
#include "util.h"
#include <qfileinfo.h>
#include <qapplication.h>
#include "options.h"
#include <qdir.h>

extern "C" {
#define XMD_H // prevent INT32 clash from jpeglib
#include <jpeglib.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <sys/types.h>
}

#include <qwmatrix.h>

ImageLoader::ImageLoader( QWaitCondition* sleeper )
    : _sleeper( sleeper )
{
}

void ImageLoader::run()
{
    while ( true ) {
        ImageRequest* request = ImageManager::instance()->next();

        if ( request ) {
            bool ok = false;
            QImage img;
            bool imageLoaded = false;

            QString cacheDir =  Util::getThumbnailDir( request->fileName() );
            QString cacheFile = Util::getThumbnailFile( request->fileName(), request->width(), request->height(), request->angle() );
            // Try to load thumbernail from cache
            if ( QFileInfo( cacheFile ).exists() ) {
                if ( img.load( cacheFile ) )  {
                    imageLoaded = true;
                    ok = true;
                }
            }

            if ( !imageLoaded && QFile( request->fileName() ).exists() ) {
                if (Util::isJPEG(request->fileName())) {
                    QSize fullSize;
                    ok = Util::loadJPEG(&img, request->fileName(),  &fullSize, request->width(), request->height());
                    if (ok == true)
                        request->setFullSize( fullSize );
                } else if( Util::isCRW(request->fileName())) {
                    QSize fullSize;
                    ok = Util::loadCRW(&img, request->fileName(),  &fullSize, request->width(), request->height());
                    if (ok)
                        request->setFullSize( fullSize );
                } else {
                    ok = img.load( request->fileName() );
                    if (ok)
                        request->setFullSize( img.size() );
                }

                if (ok) {
                    if ( request->angle() != 0 )  {
                        QWMatrix matrix;
                        matrix.rotate( request->angle() );
                        img = img.xForm( matrix );
                        int angle = (request->angle() + 360)%360;
                        Q_ASSERT( angle >= 0 && angle <= 360 );
                        if ( angle == 90 || angle == 270 )
                            request->setFullSize( QSize( request->fullSize().height(), request->fullSize().width() ) );

                    }

                    // If we are looking for a scaled version, then scale
                    if ( request->width() != -1 && request->height() != -1 )
                        img = img.smoothScale( request->width(), request->height(), QImage::ScaleMin );

                    // Save thumbnail to disk
                    if ( request->cache() ) {
                        if ( ! QDir( cacheDir ).exists() ) {
                            QDir().mkdir( cacheDir, true );
                        }
                        img.save( cacheFile, "JPEG" );
                    }
                }

                imageLoaded = true;
            }

            request->setLoadedOK( ok );
            ImageEvent* iew = new ImageEvent( request, img );
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
    img = img.smoothScale( width, height, QImage::ScaleMin );
    return img;
}

void ImageLoader::removeThumbnail( const QString& imageFile )
{
    QFileInfo fi( imageFile );
    QString tnPattern = QString::fromLatin1( "*-%2.%3" ).arg(fi.baseName()).arg(fi.extension());
    QDir dir( QString::fromLatin1( "%1/ThumbNails" ).arg( fi.dirPath() ) );
    QStringList files = dir.entryList( tnPattern );
    for( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it ) {
        dir.remove( *it );
    }
}
