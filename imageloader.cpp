/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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
                                .arg( li.angle()).arg( QFileInfo( li.fileName() ).fileName() );
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

                // HACK ALERT  HACK ALERT  HACK ALERT  HACK ALERT  HACK ALERT  HACK ALERT  HACK ALERT
                // To optimize showing tooltip images in the browser, we better cache the 256x256 thumbnails here
                // this is of course not a proper way of doing this - an ugly side effect of this image loader in fact,
                // but in love and war (and software optimization) sometimes you must do something wrong to get it really good ;-)
                //  4 Jan. 2004 19:51 -- Jesper K. Pedersen
                // HACK ALERT  HACK ALERT  HACK ALERT  HACK ALERT  HACK ALERT  HACK ALERT  HACK ALERT
                {
                    QImage hack = img.scale( 256, 256, QImage::ScaleMin );
                    QString cacheFileForHack = cacheDir + QString::fromLatin1("/%1x%2-%3-%4")
                                               .arg(256).arg(256)
                                               .arg( li.angle()).arg( QFileInfo( li.fileName() ).fileName() );
                    hack.save( cacheFileForHack, "JPEG" );
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
