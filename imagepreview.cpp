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
    Viewer* viewer = new Viewer( this, "image viewer" );
    viewer->show();
    viewer->load( _info );
}

void ImagePreview::keyPressEvent( QKeyEvent* ev)
{
    if ( ev->key() == Key_PageDown )  {
        _info->rotate( 90 );
        reload();
    }
    else if ( ev->key() == Key_PageUp )  {
        _info->rotate( -90 );
        reload();
    }
}

void ImagePreview::reload()
{
    setText( "Loading..." );
    ImageManager::instance()->load( _info->fileName(), this, _info->angle(), 256, 256, false );
}

void ImagePreview::pixmapLoaded( const QString&, int, int, int, const QPixmap& pix )
{
    setPixmap( pix );
}

