#include "viewersizeconfig.h"
#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <klocale.h>
#include <qlabel.h>
ViewerSizeConfig::ViewerSizeConfig( const QString& title, QWidget* parent, const char* name )
    :QVGroupBox( title, parent, name )
{
    _fullScreen = new QCheckBox( i18n("Launch in full screen" ), this );

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
}

void ViewerSizeConfig::setSize( const QSize& size  )
{
    _width->setValue( size.width() );
    _height->setValue( size.height() );
}

QSize ViewerSizeConfig::size()
{
    return QSize( _width->value(), _height->value() );
}

void ViewerSizeConfig::setLaunchFullScreen( bool b )
{
    _fullScreen->setChecked( b );
}

bool ViewerSizeConfig::launchFullScreen() const
{
    return _fullScreen->isChecked();
}

#include "viewersizeconfig.moc"
