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
    int w = Options::instance()->thumbWidth();
    int h = Options::instance()->thumbHeight();
    QPixmap img( w, h );
    QPainter painter( &img );
    painter.fillRect( 0, 0,  w, h,  white );
    painter.drawRect( 0, 0, w-1, h-1 );
    setPixmap( img );
    setText( imageInfo->label() );
    ImageManager::instance()->load( _imageInfo->fileName(),  this,  w,  h );
}

QString ThumbNail::fileName() const
{
    return _imageInfo->fileName();
}

ImageInfo* ThumbNail::imageInfo()
{
    return _imageInfo;
}

void ThumbNail::pixmapLoaded( const QString&, int, int, const QPixmap& pixmap )
{
    setPixmap( pixmap );
}





