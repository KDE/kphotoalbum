#include "speeddisplay.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>

SpeedDisplay::SpeedDisplay( QWidget* parent, const char* name )
    :QDialog( parent, name, false, WStyle_Customize | WStyle_NoBorder | WX11BypassWM| WStyle_StaysOnTop )
{
    _label = new QLabel( this );
    _layout = new QHBoxLayout( this );
    _layout->addWidget( _label );
    _timer = new QTimer( this );
    connect( _timer, SIGNAL( timeout() ), this, SLOT( hide() ) );

    _label->setPaletteBackgroundColor( yellow );
    _label->setFrameStyle( QFrame::Box | QFrame::Plain );
}

void SpeedDisplay::display( int i )
{
    _label->setText( tr("<qt><center><font size=\"+4\">%1&nbsp;s</font></center></qt>").arg( QString::number( i/1000.0, 'f', 1 ) ) );
    go();
}

void SpeedDisplay::start()
{
    _label->setText( tr("<qt><center><font size=\"+4\">Starting Slideshow</font></center></qt>") );
    go();
}

void SpeedDisplay::go()
{
    _layout->invalidate();
    resize( sizeHint() );
    QWidget* p = static_cast<QWidget*>( parent() );
    move( ( p->width() - width() )/2 + p->x(), ( p->height() - height() )/2 + p->y() );
    show();
    _timer->start( 1000 );
}

void SpeedDisplay::end()
{
    _label->setText( tr("<qt><center><font size=\"+4\">Ending Slideshow</font></center></qt>") );
    go();
}
