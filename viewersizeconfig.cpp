#include "viewersizeconfig.h"
#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <klocale.h>
#include <qlabel.h>
ViewerSizeConfig::ViewerSizeConfig( const QString& title, QWidget* parent, const char* name )
    :QVGroupBox( title, parent, name )
{
    _fullScreen = new QCheckBox( i18n("Full screen" ), this );

    QWidget* sizeBox = new QWidget( this );
    QHBoxLayout* lay = new QHBoxLayout( sizeBox, 0, 6 );

    QLabel* label = new QLabel( i18n("Size"), sizeBox );
    lay->addWidget( label );

    _width = new QSpinBox( 100, 5000, 50, sizeBox );
    lay->addWidget( _width );

    label = new QLabel( QString::fromLatin1("x"), sizeBox );
    lay->addWidget( label );

    _height = new QSpinBox( 100, 5000, 50, sizeBox );
    lay->addWidget( _height );

    lay->addStretch( 1 );

    connect( _fullScreen, SIGNAL( toggled( bool ) ), sizeBox, SLOT( setDisabled( bool ) ) );
}

void ViewerSizeConfig::setSize( const QSize& size  )
{
    int width = size.width();
    int height = size.height();

    _fullScreen->setChecked( width == -1 );
    if ( width != -1 )
        _width->setValue( width );
    if ( height != -1 )
        _height->setValue( height );
}

QSize ViewerSizeConfig::size()
{
    if ( _fullScreen->isChecked() )
        return QSize( -1, -1 );
    else
        return QSize( _width->value(), _height->value() );
}
