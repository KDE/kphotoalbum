/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#include "ImageLoader.h"

#include "ImageDecoder.h"
#include "Manager.h"
#include "RawImageDecoder.h"
#include "Utilities/Util.h"
#include "ThumbnailStorage.h"

#include <qapplication.h>
#include <qfileinfo.h>

extern "C" {
 #include <limits.h>
 #include <setjmp.h>
 #include <stdio.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <unistd.h>
}

#include <kcodecs.h>
#include <kurl.h>
#include <qmatrix.h>

namespace ImageManager
{
    // Create a global instance. Its constructor will itself register it.
    RAWImageDecoder rawdecoder;
}



ImageManager::ImageLoader::ImageLoader(ThumbnailStorage* storage)
    : _storage(storage) {
    /* nop */
}

void ImageManager::ImageLoader::run()
{
    while ( true ) {
        ImageRequest* request = Manager::instance()->next();
        Q_ASSERT( request );
        bool ok;

        QImage img = tryLoadThumbnail( request, ok );
        if ( ! ok ) {
            img = loadImage( request, ok );
            if ( ok ) {
                writeThumbnail( request, img );
            }
        }

        if ( ok ) {
            img = scaleAndRotate( request, img );
        }

        request->setLoadedOK( ok );
        ImageEvent* iew = new ImageEvent( request, img );
        QApplication::postEvent( Manager::instance(),  iew );
    }
}

QImage ImageManager::ImageLoader::rotateAndScale( QImage img, int width, int height, int angle )
{
    if ( angle != 0 )  {
        QMatrix matrix;
        matrix.rotate( angle );
        img = img.transformed( matrix );
    }
    img = Utilities::scaleImage(img, width, height, Qt::KeepAspectRatio );
    return img;
}

QImage ImageManager::ImageLoader::tryLoadThumbnail( ImageRequest* request, bool& ok )
{
    ok = false;
    QString key = thumbnailKey( request );

    QImage result;
    ok = _storage->retrieve(key, &result);
    if (!ok)
        return result;

    // TODO(hzeller): ppm do not store meta tags - we always accept them,
    // because we cannot check 'outdatedness'.
    // Find workaround (mtime(thumbnail) ? )
    time_t thumbTime = result.text( "Thumb::MTime" ).toUInt();

    if ( thumbTime != 0 && QFileInfo(request->fileName()).exists() ) {
        time_t fileTime = QFileInfo(request->fileName()).lastModified().toTime_t();
        ok = (thumbTime == fileTime);
    }

    return result;
}

QImage ImageManager::ImageLoader::loadImage( ImageRequest* request, bool& ok )
{
    int dim = calcLoadSize( request );
    QSize fullSize;

    ok = false;
    if ( !QFile( request->fileName() ).exists() )
        return QImage();

    QImage img;
    if (Utilities::isJPEG(request->fileName())) {
        ok = Utilities::loadJPEG(&img, request->fileName(),  &fullSize, dim);
        if (ok == true)
            request->setFullSize( fullSize );
    }

    else {
        // At first, we have to give our RAW decoders a try. If we allowed
        // QImage's load() method, it'd for example load a tiny thumbnail from
        // NEF files, which is not what we want.
        ok = ImageDecoder::decode( &img, request->fileName(),  &fullSize, dim);
        if (ok)
            request->setFullSize( img.size() );
    }

    if (!ok) {
        // Now we can try QImage's stuff as a fallback...
        ok = img.load( request->fileName() );
        if (ok)
            request->setFullSize( img.size() );

    }

    return img;
}

void ImageManager::ImageLoader::writeThumbnail( ImageRequest* request, QImage img )
{
    int dim = calcLoadSize( request );
     QList<int> list;
    list << 128;
    if ( dim == 256 )
        list << 256;

     for( QList<int>::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString path = thumbnailKey( requestURL( request ), *it );
        if ( path.isNull() )
            continue;

        QFileInfo fi( request->fileName() );
        QImage scaledImg = Utilities::scaleImage(img, *it, *it, Qt::KeepAspectRatio );
        scaledImg.setText( "Software","",QString::fromLatin1( "KPhotoAlbum" ) );
        scaledImg.setText( "Thumb::URI", "", requestURL( request ) );
        scaledImg.setText( "Thumb::MTime", "", QString::number( fi.lastModified().toTime_t() ) );
        scaledImg.setText( "Thumb::Size", "", QString::number( fi.size() ) );
        scaledImg.setText( "Thumb::Image::Width", "", QString::number( request->fullSize().width() ) );
        scaledImg.setText( "Thumb::Image::Height", "", QString::number( request->fullSize().height() ) );
        _storage->store(path, scaledImg);
    }
}

int ImageManager::ImageLoader::calcLoadSize( ImageRequest* request )
{
    if ( request->width() == -1 )
        return -1;

    int max = qMax( request->width(), request->height() );
    if ( max > 256 )
        return max;
    else if ( max > 128 )
        return 256;
    else
        return 128;
}

QImage ImageManager::ImageLoader::scaleAndRotate( ImageRequest* request, QImage img )
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

QString ImageManager::ImageLoader::thumbnailKey( ImageRequest* request )
{
    return thumbnailKey( requestURL( request ), calcLoadSize( request ) );
}

QString ImageManager::ImageLoader::thumbnailKey( QString uri, int dim )
{
    QString dir;
    if ( dim == 256 )
        dir = QString::fromLatin1( "large" );
    else if ( dim == 128 )
        dir = QString::fromLatin1( "normal" );
    else
        return QString::null;

    KMD5 md5( uri.toUtf8() );
    return QString::fromLatin1( "%1/%2" ).arg(dir).arg(QString::fromUtf8(md5.hexDigest()));
}

QString ImageManager::ImageLoader::requestURL( ImageRequest* request )
{
    KUrl url;
    url.setPath( request->fileName() );
    return url.url();
}

bool ImageManager::ImageLoader::shouldImageBeScale( const QImage& img, ImageRequest* request )
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
