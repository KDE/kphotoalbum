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
the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "imageloader.h"
#include <qwaitcondition.h>
#include "imagedecoder.h"
#include "imagemanager.h"
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
#include <kurl.h>
#include <kdebug.h>
#include <kmdcodec.h>

ImageLoader::ImageLoader( QWaitCondition* sleeper )
    : _sleeper( sleeper )
{
}

void ImageLoader::run()
{
    while ( true ) {
        ImageRequest* request = ImageManager::instance()->next();

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

QImage ImageLoader::tryLoadThumbnail( ImageRequest* request, bool& ok )
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

QImage ImageLoader::loadImage( ImageRequest* request, bool& ok )
{
    int dim = calcLoadSize( request );
    QSize fullSize;

    ok = false;
    if ( !QFile( request->fileName() ).exists() )
        return QImage();

    QImage img;
    if (Util::isJPEG(request->fileName())) {
        ok = Util::loadJPEG(&img, request->fileName(),  &fullSize, dim);
        if (ok == true)
            request->setFullSize( fullSize );
    } else {
        ok = img.load( request->fileName() );
        if (ok)
            request->setFullSize( img.size() );
    }
    if (!ok) {
        // Still didn't work, try with our own decoders
        ok = ImageDecoder::decode( &img, request->fileName(),  &fullSize, dim);
        if (ok)
            request->setFullSize( img.size() );
    }

    return img;
}

void ImageLoader::writeThumbnail( ImageRequest* request, QImage img )
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
        QImage scaledImg = img.smoothScale( *it, *it, QImage::ScaleMin );
        scaledImg.setText( "Software","en",QString::fromLatin1( "KPhotoAlbum" ) );
        scaledImg.setText( "Thumb::URI", "en", requestURL( request ) );
        scaledImg.setText( "Thumb::MTime", "en", QString::number( fi.lastModified().toTime_t() ) );
        scaledImg.setText( "Thumb::Size", "en", QString::number( fi.size() ) );
        scaledImg.setText( "Thumb::Image::Width", "en", QString::number( request->fullSize().width() ) );
        scaledImg.setText( "Thumb::Image::Height", "en", QString::number( request->fullSize().height() ) );
        scaledImg.save( path, "PNG" );
    }
}

int ImageLoader::calcLoadSize( ImageRequest* request )
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

QImage ImageLoader::scaleAndRotate( ImageRequest* request, QImage img )
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
    if ( request->width() != -1 )
        img = img.smoothScale( request->width(), request->height(), QImage::ScaleMin );

    return img;
}

QString ImageLoader::thumbnailPath( ImageRequest* request )
{
    return thumbnailPath( requestURL( request ), calcLoadSize( request ) );
}

QString ImageLoader::thumbnailPath( QString uri, int dim )
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

QString ImageLoader::requestURL( ImageRequest* request )
{
    KURL url;
    url.setPath( request->fileName() );
    return url.url();
}
