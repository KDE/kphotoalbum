#include "viewer.h"
#include <qlayout.h>
#include <qlabel.h>
#include "imageinfo.h"

Viewer::Viewer( QWidget* parent, const char* name )
    :QDialog( parent,  name ), _info(0)
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    _label = new QLabel( this );
    layout->addWidget( _label );
}

void Viewer::load( ImageInfo* info )
{
    _info = info;
    _label->setPixmap( info->pixmap( _label->width(),  _label->height() ) );
}

void Viewer::resizeEvent( QResizeEvent* e )
{
    QDialog::resizeEvent( e );
    if ( _info )
        load( _info );
}
