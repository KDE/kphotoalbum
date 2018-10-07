/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ImageLoaderThread.h"
#include "ThumbnailCache.h"

#include "ImageDecoder.h"
#include "AsyncLoader.h"
#include "RawImageDecoder.h"
#include "Utilities/FastJpeg.h"
#include "Utilities/Util.h"

#include <qapplication.h>
#include <qfileinfo.h>

extern "C" {
 #include <limits.h>
 #include <setjmp.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <unistd.h>
}

#include <kcodecs.h>
#include <qmatrix.h>
#include "ImageEvent.h"

namespace ImageManager
{
    // Create a global instance. Its constructor will itself register it.
    RAWImageDecoder rawdecoder;
}

ImageManager::ImageLoaderThread::ImageLoaderThread( size_t bufsize )
    : m_imageLoadBuffer( new char[bufsize] ),
      m_bufSize( bufsize )
{
}

ImageManager::ImageLoaderThread::~ImageLoaderThread()
{
    delete[] m_imageLoadBuffer;
}

void ImageManager::ImageLoaderThread::run()
{
    while ( true ) {
        ImageRequest* request = AsyncLoader::instance()->next();
        Q_ASSERT( request );
        if ( request->isExitRequest() ) {
            return;
        }
        bool ok;

        QImage img = loadImage( request, ok );

        if ( ok ) {
            img = scaleAndRotate( request, img );
        }

        request->setLoadedOK( ok );
        ImageEvent* iew = new ImageEvent( request, img );
        QApplication::postEvent( AsyncLoader::instance(),  iew );
    }
}

QImage ImageManager::ImageLoaderThread::loadImage( ImageRequest* request, bool& ok )
{
    int dim = calcLoadSize( request );
    QSize fullSize;

    ok = false;
    if ( !request->fileSystemFileName().exists() )
        return QImage();

    QImage img;
    if (Utilities::isJPEG(request->fileSystemFileName())) {
      ok = Utilities::loadJPEG( &img, request->fileSystemFileName(),  &fullSize, dim, 
                                m_imageLoadBuffer, m_bufSize );
        if (ok == true)
            request->setFullSize( fullSize );
    }

    else {
        // At first, we have to give our RAW decoders a try. If we allowed
        // QImage's load() method, it'd for example load a tiny thumbnail from
        // NEF files, which is not what we want.
        ok = ImageDecoder::decode( &img, request->fileSystemFileName(),  &fullSize, dim);
        if (ok)
            request->setFullSize( img.size() );
    }

    if (!ok) {
        // Now we can try QImage's stuff as a fallback...
        ok = img.load( request->fileSystemFileName().absolute() );
        if (ok)
            request->setFullSize( img.size() );

    }

    return img;
}


int ImageManager::ImageLoaderThread::calcLoadSize( ImageRequest* request )
{
    return qMax( request->width(), request->height() );
}

QImage ImageManager::ImageLoaderThread::scaleAndRotate( ImageRequest* request, QImage img )
{
    if ( request->angle() != 0 )  {
        QMatrix matrix;
        matrix.rotate( request->angle() );
        img = img.transformed( matrix );
        int angle = (request->angle() + 360)%360;
        Q_ASSERT( angle >= 0 && angle <= 360 );
        if ( angle == 90 || angle == 270 )
            request->setFullSize( QSize( request->fullSize().height(), request->fullSize().width() ) );
    }

    // If we are looking for a scaled version, then scale
    if ( shouldImageBeScale( img, request ) )
        img = Utilities::scaleImage(img, request->width(), request->height(), Qt::KeepAspectRatio );

    return img;
}

bool ImageManager::ImageLoaderThread::shouldImageBeScale( const QImage& img, ImageRequest* request )
{
    // No size specified, meaning we want it full size.
    if ( request->width() == -1 )
        return false;

    if ( img.width() < request->width() && img.height() < request->height() ) {
        // The image is smaller than the requets.
        return request->doUpScale();
    }

    return true;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
