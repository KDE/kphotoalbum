#include "viewer.h"
#include <qlayout.h>
#include <qlabel.h>
#include "imageinfo.h"
#include "imagemanager.h"
#include <qsizepolicy.h>

Viewer* Viewer::_instance = 0;

Viewer* Viewer::instance()
{
    if ( !_instance )
        _instance = new Viewer( 0, "viewer" );
    return _instance;
}

Viewer::Viewer( QWidget* parent, const char* name )
    :QDialog( parent,  name )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setResizeMode( QLayout::FreeResize );
    _label = new QLabel( this );
    _label->setAlignment( AlignCenter );
    layout->addWidget( _label );
}

void Viewer::load( const ImageInfoList& list, int index )
{
    _list = list;
    _current = index;
    _info = *( _list.at( _current ) );
    load();
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

void Viewer::keyPressEvent( QKeyEvent* e )
{
    if ( e->key() == Key_Plus )  {
        resize( width()*4/3,  height()*4/3 );
        load();
    }

    else if ( e->key() == Key_Minus )  {
        resize( width()*3/4,  height()*3/4 );
        load();
    }

    else if ( e->key() == Key_9 )  {
        _info.rotate( 90 );
        load();
    }

    else if ( e->key() == Key_7 )  {
        _info.rotate( -90 );
        load();
    }

    else if ( e->key() == Key_8 )  {
        _info.rotate( 180 );
        load();
    }
}

void Viewer::load()
{
    _label->setText( "Loading..." );
    ImageManager::instance()->load( _info.fileName( false ), this, _info.angle(), _label->width(),  _label->height(), false );
}
