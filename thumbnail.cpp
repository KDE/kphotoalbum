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
#include <math.h>

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

void ThumbNail::pixmapLoaded( const QString&, const QSize& /*size*/, const QSize& /*fullSize*/, int, const QImage& image  )
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
    _parent->setDragLeft( this );
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

void ThumbNail::calcRect( const QString& text )
{
    int size = Options::instance()->thumbSize();
    QIconViewItem::calcRect( text );
    if ( !Options::instance()->displayLabels() ) {
        setTextRect( QRect(0,0,0,0) );
        setText( QString::null );
        setItemRect( QRect( rect().x(), rect().y(), size, size ) );
    }
    else
        setText( _imageInfo->label() );
}

void ThumbNail::paintItem( QPainter * p, const QColorGroup & cg )
{
    QColorGroup cgCopy = cg;
    QColor col = Options::instance()->thumbNailBackgroundColor();

    // Calculate the foreground color.
    int dist = (int) pow(pow(col.red(),3) + pow(col.blue(),3) + pow(col.green(),3), 1.0/3);
    int max = (int) pow( 3*pow(255,3), 1.0/3 ); // QPoint( 255, 255, 255 );
    if ( dist > max/2 )
        col = black;
    else
        col = white;

    cgCopy.setColor( QColorGroup::Text, col );
    QIconViewItem::paintItem( p, cgCopy );
}
