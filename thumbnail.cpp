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

ThumbNail::ThumbNail( ImageInfo* imageInfo, ThumbNailView* parent, const char* name )
    :QIconViewItem( parent,  name ),  _imageInfo( imageInfo )
{
    int size = Options::instance()->thumbSize();
    QPixmap img( size, size );
    QPainter painter( &img );
    painter.fillRect( 0, 0,  size, size,  white );
    painter.drawRect( 0, 0, size-1, size-1 );
    setPixmap( img );
    setText( imageInfo->label() );
    ImageManager::instance()->load( _imageInfo->fileName( false ),  this, _imageInfo->angle(), size, size );
}

QString ThumbNail::fileName() const
{
    return _imageInfo->fileName( false );
}

ImageInfo* ThumbNail::imageInfo()
{
    return _imageInfo;
}

void ThumbNail::pixmapLoaded( const QString&, int, int, int, const QPixmap& pixmap )
{
    setPixmap( pixmap );
}





