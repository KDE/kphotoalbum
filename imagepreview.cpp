/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
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

#include "imagepreview.h"
#include "viewer.h"
#include "imagemanager.h"
#include <klocale.h>
#include <qwmatrix.h>
#include "imageloader.h"

ImagePreview::ImagePreview( QWidget* parent, const char* name )
    : QLabel( parent, name )
{
    setAlignment( AlignCenter );
}

void ImagePreview::resizeEvent( QResizeEvent* )
{
    reload();
}

QSize ImagePreview::sizeHint() const
{
    return QSize( 128,128 );
}

void ImagePreview::rotate(int angle)
{
    if ( !_info.isNull() )
        _info.rotate( angle );
    else
        _angle += angle;
    reload();
}

void ImagePreview::setImage( const ImageInfo& info )
{
    _info = info;
    reload();
}

/**
   This method should only be used for the non-user images. Currently this includes
   two images: the search image and the configure several images at a time image.
*/
void ImagePreview::setImage( const QString& fileName )
{
    _fileName = fileName;
    _info = ImageInfo();
    _angle = 0;
    reload();
}


void ImagePreview::reload()
{
    if ( !_info.isNull() ) {
        QImage img = _info.load( width(), height() );
        setPixmap( img );
    }
    else {
        QImage img( _fileName );
        img = ImageLoader::rotateAndScale( img, width(), height(), _angle );
        setPixmap( img );
    }
}

int ImagePreview::angle() const
{
    Q_ASSERT( _info.isNull() );
    return _angle;
}

#include "imagepreview.moc"
