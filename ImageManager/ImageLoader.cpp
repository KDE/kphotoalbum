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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ImageLoader.h"
#include <qwaitcondition.h>
#include "ImageDecoder.h"
#include "RawImageDecoder.h"
#include "Manager.h"
#include "Utilities/Util.h"
#include <qfileinfo.h>
#include <qapplication.h>
#include <qdir.h>

extern "C" {
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
#include <kurl.h>
#include <kmdcodec.h>
#include <qpixmapcache.h>

namespace ImageManager
{
    // Create a global instance. Its constructor will itself register it.
    RAWImageDecoder rawdecoder;
}


ImageManager::ImageLoader::ImageLoader( QWaitCondition* sleeper )
    : _sleeper( sleeper )
{
}

void ImageManager::ImageLoader::run()
{
    while ( true ) {
        ImageRequest* request = Manager::instance()->next();

        if ( request ) {
            bool ok;
            QImage img = tryLoadThumbnail( request, ok );
            if ( ! ok ) {
                img = loadImage( request, ok );
                if ( ok ) {
                    writeThumbnail( request, img );
                }
            }

            if ( ok )
                img = scaleAndRotate( request, img );

            request->setLoadedOK( ok );
            ImageEvent* iew = new ImageEvent( request, img );
            QApplication::postEvent( Manager::instance(),  iew );
        }
        else
            _sleeper->wait();
    }
}

QImage ImageManager::ImageLoader::rotateAndScale( QImage img, int width, int height, int angle )
{
    if ( angle != 0 )  {
        QWMatrix matrix;
        matrix.rotate( angle );
        img = img.xForm( matrix );
    }
    img = Utilities::scaleImage(img, width, height, QImage::ScaleMin );
    return img;
}

void ImageManager::ImageLoader::removeThumbnail( const QString& imageFile )
{
    KURL url;
    url.setPath( imageFile );
    QFile::remove( thumbnailPath( url.url(), 256 ) );
    QFile::remove( thumbnailPath( url.url(), 128 ) );
    QPixmapCache::remove( imageFile );
}

QImage ImageManager::ImageLoader::tryLoadThumbnail( ImageRequest* request, bool& ok )
{
    ok = false;
    QString path = thumbnailPath( request );
    if ( QFile::exists( path ) ) {
        QImage img;
        ok = img.load( path );
        if ( QFileInfo(request->fileName()).exists() ) {
            // Only do the test of the original image exsts.
            if ( img.text( "Thumb::MTime" ).toUInt() != QFileInfo(request->fileName()).lastModified().toTime_t() )
                ok = false;
        }
        return img;
    }
    return QImage();
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
    QDir dir(QDir::homeDirPath());
    dir.mkdir( QString::fromLatin1( ".thumbnails" ) );
    dir.cd( QString::fromLatin1( ".thumbnails" ) );
    dir.mkdir( QString::fromLatin1( "normal" ) );
    dir.mkdir( QString::fromLatin1( "large" ) );

    int dim = calcLoadSize( request );
    QValueList<int> list;
    list << 128;
    if ( dim == 256 )
        list << 256;

    for( QValueList<int>::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString path = thumbnailPath( requestURL( request ), *it );
        if ( path.isNull() )
            continue;

        QFileInfo fi( request->fileName() );
        QImage scaledImg = Utilities::scaleImage(img, *it, *it, QImage::ScaleMin );
        scaledImg.setText( "Software","",QString::fromLatin1( "KPhotoAlbum" ) );
        scaledImg.setText( "Thumb::URI", "", requestURL( request ) );
        scaledImg.setText( "Thumb::MTime", "", QString::number( fi.lastModified().toTime_t() ) );
        scaledImg.setText( "Thumb::Size", "", QString::number( fi.size() ) );
        scaledImg.setText( "Thumb::Image::Width", "", QString::number( request->fullSize().width() ) );
        scaledImg.setText( "Thumb::Image::Height", "", QString::number( request->fullSize().height() ) );
        scaledImg.save( path, "PNG" );
    }
}

int ImageManager::ImageLoader::calcLoadSize( ImageRequest* request )
{
    if ( request->width() == -1 )
        return -1;

    int max = QMAX( request->width(), request->height() );
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
        QWMatrix matrix;
        matrix.rotate( request->angle() );
        img = img.xForm( matrix );
        int angle = (request->angle() + 360)%360;
        Q_ASSERT( angle >= 0 && angle <= 360 );
        if ( angle == 90 || angle == 270 )
            request->setFullSize( QSize( request->fullSize().height(), request->fullSize().width() ) );
    }

    // If we are looking for a scaled version, then scale
    if ( shouldImageBeScale( img, request ) )
        img = Utilities::scaleImage(img, request->width(), request->height(), QImage::ScaleMin );

    return img;
}

QString ImageManager::ImageLoader::thumbnailPath( ImageRequest* request )
{
    return thumbnailPath( requestURL( request ), calcLoadSize( request ) );
}

QString ImageManager::ImageLoader::thumbnailPath( QString uri, int dim )
{
    QString dir;
    if ( dim == 256 )
        dir = QString::fromLatin1( "large" );
    else if ( dim == 128 )
        dir = QString::fromLatin1( "normal" );
    else
        return QString::null;

    KMD5 md5( uri.utf8() );
    return QString::fromLatin1( "%1/.thumbnails/%2/%3.png" ).arg(QDir::homeDirPath()).arg(dir).arg(md5.hexDigest());
}

QString ImageManager::ImageLoader::requestURL( ImageRequest* request )
{
    KURL url;
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
