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
    // PENDING(blackie) Consider whether this can be replaced with KIO::PreviewJob
    ImageManager::instance()->load( _imageInfo->fileName( false ),  this, _imageInfo->angle(), size, size, true, false, true );
    setDropEnabled( true );
}




QString ThumbNail::fileName() const
{
    return _imageInfo->fileName( false );
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
    return true; // PENDING(blackie) CHANGE THIS!
}

void ThumbNail::dropped( QDropEvent * e, const QValueList<QIconDragItem> & /* lst */ )
{
    setPixmap( _pixmap );
    if ( !e->source() || e->source()->parent() != iconView() ) {
        // PENDING(blackie) Show a message box
        qDebug("Nope we don't want drops from outside!");
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
                QMessageBox::information( _parent, i18n("Self Drop"), i18n("You can't drag images to be next to themself."),
                                          QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
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
        ThumbNail* tn = new ThumbNail( item->_imageInfo, last, _parent);
        delete item;
        last = tn;
    }
}

bool ThumbNail::atRightSizeOfItem()
{
    QPoint cursorPos = iconView()->mapFromGlobal( QCursor::pos() );
    QPoint myPos = pos();
    int xDiff = (cursorPos-myPos).x();
    return ( xDiff > width()/2 );
}
