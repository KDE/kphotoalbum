/* Copyright (C) 2003-2004 Jesper K. Pedersen <blackie@kde.org>

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
        ImageRequest li = ImageManager::instance()->next();

        if ( !li.isNull() ) {
            bool ok = false;
            QImage img;
            bool imageLoaded = false;

            QString cacheDir =  Util::getThumbnailDir( li.fileName() );
            QString cacheFile = Util::getThumbnailFile( li.fileName(), li.width(), li.height(), li.angle() );
            // Try to load thumbernail from cache
            if ( QFileInfo( cacheFile ).exists() ) {
                if ( img.load( cacheFile ) )  {
                    imageLoaded = true;
                    ok = true;
                }
            }

            if ( !imageLoaded && QFile( li.fileName() ).exists() ) {
                if (Util::isJPEG(li.fileName())) {
                    QSize fullSize;
                    ok = Util::loadJPEG(&img, li.fileName(),  &fullSize, li.width(), li.height());
                    if (ok == true)
                        li.setFullSize( fullSize );
                } else if( Util::isCRW(li.fileName())) {
                    QSize fullSize;
                    ok = Util::loadCRW(&img, li.fileName(),  &fullSize, li.width(), li.height());
                    if (ok)
                        li.setFullSize( fullSize );
                } else {
                    ok = img.load( li.fileName() );
                    if (ok)
                        li.setFullSize( img.size() );
                }

                if (ok) {
                    if ( li.angle() != 0 )  {
                        QWMatrix matrix;
                        matrix.rotate( li.angle() );
                        img = img.xForm( matrix );
                        int angle = (li.angle() + 360)%360;
                        Q_ASSERT( angle >= 0 && angle <= 360 );
                        if ( angle == 90 || angle == 270 )
                            li.setFullSize( QSize( li.fullSize().height(), li.fullSize().width() ) );

                    }

                    // If we are looking for a scaled version, then scale
                    if ( li.width() != -1 && li.height() != -1 )
                        img = img.smoothScale( li.width(), li.height(), QImage::ScaleMin );

                    // Save thumbnail to disk
                    if ( li.cache() ) {
                        if ( ! QDir( cacheDir ).exists() ) {
                            QDir().mkdir( cacheDir, true );
                        }
                        img.save( cacheFile, "JPEG" );
                    }
                }

                imageLoaded = true;
            }

            li.setLoadedOK( ok );
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
