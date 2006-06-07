#include "PixmapSnapShot.h"
#include <qlabel.h>
#include <klocale.h>
#include "Outline.h"
#include <qlayout.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qapplication.h>

Video::PixmapSnapShot::PixmapSnapShot()
    : QDialog( 0, "pixmapsnapshot", false, WStyle_Customize | WStyle_NoBorder | WX11BypassWM| WStyle_StaysOnTop ),
      _activated( false )
{
    QLabel* label = new QLabel( this );
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->addWidget( label );

    label->setPaletteBackgroundColor( yellow );
    label->setFrameStyle( QFrame::Box | QFrame::Plain );
    label->setText( i18n("<qt>Drag a rectangle on your desktop to form a snapshot.</qt>") );

    _outline = new Outline(this);

    exec();
}

void Video::PixmapSnapShot::mousePressEvent( QMouseEvent* event )
{
    _outline->resize(0,0);
    _outline->show();
    _start = event->globalPos();
    resize(1,1);
}

void Video::PixmapSnapShot::mouseMoveEvent( QMouseEvent* event )
{
    _end = event->globalPos();
    QRect rect( _start, _end );
    _outline->setGeometry( rect );
}

void Video::PixmapSnapShot::mouseReleaseEvent( QMouseEvent* )
{
    releaseMouse();
    accept();

    QRect r( _start + QPoint( 2,2 ), _end - QPoint( 2,2 ));
    QPixmap p = QPixmap::grabWindow( QApplication::desktop()->winId(), r.left(),r.top(), r.width(), r.height());
    QLabel* label = new QLabel( 0 );
    label->setPixmap( p );
    label->show();
}


/**
 * We need to show the widget before we can grab the mouse. Unfortunately the widget isn't shown enough when
 * show event is executed, so we need to wait a bit extra.
 */
void Video::PixmapSnapShot::showEvent( QShowEvent* event )
{
    QDialog::showEvent( event );
    QTimer::singleShot( 100, this, SLOT( doGrab() ) );
}

void Video::PixmapSnapShot::doGrab()
{
    grabMouse();
}
