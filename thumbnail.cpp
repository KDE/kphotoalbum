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


#include "thumbnail.h"
#include <qpixmap.h>
#include <qimage.h>
#include <qiconview.h>
#include "thumbnailview.h"
#include <qpainter.h>
#include "options.h"
#include "imageinfo.h"
#include "imagemanager.h"
#include <qdatetime.h>
#include <qcursor.h>
#include <qmessagebox.h>
#include <klocale.h>
#include <kmessagebox.h>

ThumbNail::ThumbNail( ImageInfo* imageInfo, ThumbNailView* parent )
    :QIconViewItem( parent ),  _imageInfo( imageInfo ), _parent( parent )
{
    init();
}

ThumbNail::ThumbNail( ImageInfo* imageInfo, ThumbNail* after, ThumbNailView* parent )
    :QIconViewItem( parent, after ),  _imageInfo( imageInfo ), _parent( parent )
{
    init();
}


void ThumbNail::init()
{
    int size = Options::instance()->thumbSize();
    _pixmap.resize( size, size );
    QPainter painter( &_pixmap );
    painter.fillRect( 0, 0,  size, size,  white );
    painter.drawRect( 0, 0, size, size );
    setPixmap( _pixmap );
    setText( _imageInfo->label() );
    ImageManager::instance()->load( _imageInfo->fileName(),  this, _imageInfo->angle(), size, size, true, false );
    setDropEnabled( true );
}




QString ThumbNail::fileName() const
{
    return _imageInfo->fileName();
}

ImageInfo* ThumbNail::imageInfo()
{
    return _imageInfo;
}

void ThumbNail::pixmapLoaded( const QString&, int, int, int, const QImage& image  )
{
    if ( !image.isNull() )
        _pixmap.convertFromImage( image );

    if ( !_imageInfo->imageOnDisk() ) {
        QPainter p( &_pixmap );
        p.setBrush( white );
        p.setWindow( 0, 0, 100, 100 );
        QPointArray pts;
        pts.setPoints( 3, 70,-1,  100,-1,  100,30 );
        p.drawConvexPolygon( pts );
    }
    setPixmap( _pixmap );
}
void ThumbNail::dragMove()
{
    QPixmap pix( _pixmap );
    QPainter p( &pix );
    if ( atRightSizeOfItem() )
        p.fillRect( pix.width()-5, 0, 5, pix.height(), red );
    else
        p.fillRect( 0, 0, 5, pix.height(), red );

    setPixmap( pix );
    _parent->setHighlighted( this );
}

void ThumbNail::dragLeft()
{
    setPixmap( _pixmap );
}

bool ThumbNail::acceptDrop( const QMimeSource * /*mime*/ ) const
{
    return true; // Actually this is too much, but its no big deal.
}

void ThumbNail::dropped( QDropEvent * e, const QValueList<QIconDragItem> & /* lst */ )
{
    setPixmap( _pixmap );
    if ( e->source() != iconView() ) {
        KMessageBox::sorry( 0, i18n("KimDaBa does not support data being dragged onto it.") );
        return;
    }

    QPtrList<ThumbNail> list;
    ImageInfoList imageList;
    for ( QIconViewItem* item = iconView()->firstItem(); item; item = item->nextItem() ) {
        ThumbNail* tn = dynamic_cast<ThumbNail*>( item );
        if ( item->isSelected() ) {
            list.append(tn);
            imageList.append( tn->_imageInfo );
            // Protect against a drop on yourself.
            if ( item == this ) {
                return;
            }
        }
    }

    _parent->reorder( _imageInfo, imageList, atRightSizeOfItem() );

    ThumbNail* last;
    if ( atRightSizeOfItem() ) {
        last = this;
    }
    else {
        if ( prevItem() ) {
            last = dynamic_cast<ThumbNail*>(prevItem());
            Q_ASSERT( last );
        }
        else {
            // We try to put them at the first position
            last = this;
            list.append( this );
        }
    }

    for( QPtrListIterator<ThumbNail> it( list ); *it; ++it ) {
        ThumbNail* item = *it;
        _parent->takeItem( item );
        _parent->insertItem( item, last );
        last = item;
    }
}

bool ThumbNail::atRightSizeOfItem()
{
    QPoint cursorPos = iconView()->mapFromGlobal( QCursor::pos() );
    QPoint myPos = pos();
    int xDiff = (cursorPos-myPos).x();
    return ( xDiff > width()/2 );
}


