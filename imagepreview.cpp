#include "imagepreview.h"
#include "viewer.h"
#include "imagemanager.h"

ImagePreview::ImagePreview( QWidget* parent, const char* name )
    : QLabel( parent, name )
{
    setAlignment( AlignCenter );
    setFocusPolicy( WheelFocus );
}

void ImagePreview::setInfo( ImageInfo* info )
{
    _info = info;
}

void ImagePreview::mouseDoubleClickEvent( QMouseEvent* )
{
    emit doubleClicked();
}

void ImagePreview::keyPressEvent( QKeyEvent* ev)
{
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
    setText( "Loading..." );
    ImageManager::instance()->load( _info->fileName( false ), this, _info->angle(), 256, 256, false, true );
}

void ImagePreview::pixmapLoaded( const QString&, int, int, int, const QPixmap& pix )
{
    setPixmap( pix );
}


#include "imagepreview.moc"
