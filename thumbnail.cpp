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
#include <qpixmapcache.h>
#include "imageinfolist.h"

ThumbNail::ThumbNail( ImageInfo* imageInfo, ThumbNailView* parent )
    :QIconViewItem( parent ),  _imageInfo( imageInfo ), _parent( parent ), _highlightItem( false )
{
    init();
}

ThumbNail::ThumbNail( ImageInfo* imageInfo, ThumbNail* after, ThumbNailView* parent )
    :QIconViewItem( parent, after ),  _imageInfo( imageInfo ), _parent( parent ), _highlightItem( false )
{
    init();
}


void ThumbNail::init()
{
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

void ThumbNail::pixmapLoaded( const QString&, const QSize& size, const QSize& /*fullSize*/, int, const QImage& image, bool loadedOK )
{
    QPixmap* pixmap = new QPixmap( size );
    if ( loadedOK && !image.isNull() )
        pixmap->convertFromImage( image );
    else if ( !loadedOK)
        pixmap->fill(Qt::gray);


    if ( !loadedOK || !_imageInfo->imageOnDisk() ) {
        QPainter p( pixmap );
        p.setBrush( white );
        p.setWindow( 0, 0, 100, 100 );
        QPointArray pts;
        pts.setPoints( 3, 70,-1,  100,-1,  100,30 );
        p.drawConvexPolygon( pts );
    }

    pixmapCache().insert( _imageInfo->fileName(), pixmap );
    repaintItem();
}

void ThumbNail::dragMove()
{
    if ( !_highlightItem ) {
        _highlightItem = true;
        repaintItem();
        _parent->setHighlighted( this );
    }
}

void ThumbNail::dragLeft()
{
    _highlightItem = false;
    repaintItem();
    _parent->setDragLeft( this );
}

bool ThumbNail::acceptDrop( const QMimeSource * /*mime*/ ) const
{
    return true; // Actually this is too much, but its no big deal.
}

void ThumbNail::dropped( QDropEvent * e, const QValueList<QIconDragItem> & /* lst */ )
{
    _highlightItem = false;
    repaintItem();
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

bool ThumbNail::atRightSizeOfItem() const
{
    QPoint cursorPos = iconView()->mapFromGlobal( QCursor::pos() );
    QPoint myPos = pos();
    int xDiff = (cursorPos-myPos).x();
    return ( xDiff > width()/2 );
}

void ThumbNail::calcRect( const QString& text )
{
    if ( !Options::instance()->displayLabels() )
        setText( QString::null );
    else
        setText( _imageInfo->label() );

    if ( Options::instance()->displayLabels() )
        QIconViewItem::calcRect( text );
    else {
        int size = Options::instance()->thumbSize();
        QPixmap* pix = pixmap();
        QRect r( 0,0, size, size );
        if ( !pix->isNull() ) {
            int w = pix->width();
            int h = pix->height();
            r = QRect( (size-w)/2, 0, w, h );
        }

        setTextRect( QRect(0,0,0,0) );
        setItemRect( QRect( x(), y(), size, size ) );
        setPixmapRect( r );
    }
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

QPixmapCache& ThumbNail::pixmapCache()
{
    static QPixmapCache cache;
    static int lastSize = -1;
    cache.setCacheLimit( 4* 1024 ); // PENDING(blackie) make this size customizable
    int currentThumbSize = Options::instance()->thumbSize();
    if (lastSize != currentThumbSize) {
      cache.clear();
      lastSize = currentThumbSize;
    }
    return cache;
}

QPixmap* ThumbNail::pixmap() const
{
    QPixmap* pix = pixmapCache().find( _imageInfo->fileName() );
    if ( pix ) {
        if ( _highlightItem ) {
            highlightPixmap() = *pix;
            QPainter p( &highlightPixmap() );
            if ( atRightSizeOfItem() )
                p.fillRect( highlightPixmap().width()-5, 0, 5, highlightPixmap().height(), red );
            else
                p.fillRect( 0, 0, 5, highlightPixmap().height(), red );
            return &highlightPixmap();
        }
        else
            return pix;
    }

    int size = Options::instance()->thumbSize();
    ImageManager::instance()->load( _imageInfo->fileName(),  const_cast<ThumbNail*>( this ), _imageInfo->angle(), size, size, true, false );
    return emptyPixmap();
}

QPixmap* ThumbNail::emptyPixmap()
{
    static QPixmap pixmap;
    return &pixmap;
}

void ThumbNail::repaintItem()
{
    QRect oR = rect();
    calcRect();
    oR = oR.unite( rect() );

	if ( QRect( iconView()->contentsX(), iconView()->contentsY(),
                iconView()->visibleWidth(), iconView()->visibleHeight() ).
	     intersects( oR ) )
	    iconView()->repaintContents( oR.x() - 1, oR.y() - 1,
                                   oR.width() + 2, oR.height() + 2, FALSE );
}

QPixmap& ThumbNail::highlightPixmap()
{
    // We don't want to bother remembering to delete the highlight pixmap, so we just create a global one we reuse over and over again.
    static QPixmap pix;
    return pix;
}
