#include "viewer.h"
#include <qlayout.h>
#include <qlabel.h>
#include "imageinfo.h"
#include "imagemanager.h"
#include <qtimer.h>

Viewer::Viewer( QWidget* parent, const char* name )
    :QDialog( parent,  name ), _info(0)
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    _label = new QLabel( this );
    _label->setAlignment( AlignCenter );
    layout->addWidget( _label );
    _timer = new QTimer( this );
    connect( _timer, SIGNAL( timeout() ), this,  SLOT( resizeImage() ) );
}

void Viewer::load( ImageInfo* info )
{
    _info = info;
    _label->setText( "Loading..." );
    ImageManager::instance()->load( info->fileName(), this, info->angle(), _label->width(),  _label->height(), false );
}

void Viewer::resizeEvent( QResizeEvent* e )
{
    _timer->start( 1000, true );
    QDialog::resizeEvent( e );
}

void Viewer::pixmapLoaded( const QString&, int, int, int, const QPixmap& pixmap )
{
    _label->setPixmap( pixmap );
}

void Viewer::show()
{
    resize( 800, 600 );
    QDialog::show();
}

void Viewer::resizeImage()
{
    if ( _info )
        load( _info );
}

