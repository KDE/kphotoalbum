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

#include "imagepreview.h"
#include "viewer.h"
#include "imagemanager.h"
#include <klocale.h>
#include <qwmatrix.h>
#include "imageloader.h"

ImagePreview::ImagePreview( QWidget* parent, const char* name )
    : QLabel( parent, name ), _info(0)
{
    setAlignment( AlignCenter );
    setFocusPolicy( WheelFocus );
}

void ImagePreview::setInfo( ImageInfo* info )
{
    _info = info;
    _img = QImage();
    reload();
}

void ImagePreview::keyPressEvent( QKeyEvent* ev)
{
    if ( !_info )
        return;

    if ( ev->key() == Key_9 )  {
        _info->rotate( 90 );
        reload();
    }
    else if ( ev->key() == Key_7 )  {
        _info->rotate( -90 );
        reload();
    }

    else if ( ev->key() == Key_8 )  {
        _info->rotate( 180 );
        reload();
    }
}

void ImagePreview::reload()
{
    if ( !_info )
        return;

    if ( _img.isNull() ) {
        setText( i18n("Loading...") );
        ImageManager::instance()->load( _info->fileName( false ), this, 0, -1, -1, false, true );
    }
    else {
        QImage img = ImageLoader::rotateAndScale( _img, width(), height(), _info->angle() );
        QPixmap pix;
        pix.convertFromImage( img );
        setPixmap( pix );
    }
}

void ImagePreview::pixmapLoaded( const QString& fileName, int, int, int, const QImage& image )
{
    if ( fileName == _info->fileName( false ) ) {
        _img = image;
        reload();
    }
}


void ImagePreview::resizeEvent( QResizeEvent* )
{
    reload();
}

QSize ImagePreview::sizeHint() const
{
    return QSize( 128,128 );
}

#include "imagepreview.moc"
