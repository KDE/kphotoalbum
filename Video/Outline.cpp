#include "Outline.h"
#include <qpainter.h>
Video::Outline::Outline( QWidget* parent )
    : QWidget( parent, "outline",  WStyle_Customize | WStyle_NoBorder | WX11BypassWM| WStyle_StaysOnTop | WType_TopLevel )
{
}

void Video::Outline::paintEvent( QPaintEvent* )
{
    QPainter p( this );
    p.fillRect( rect(), black );
}

void Video::Outline::resizeEvent( QResizeEvent* )
{
    // 2 pixels of border.
    QRegion region( rect() );
    region -= QRegion( 2,2, width()-4, height()-4 );
    setMask( region );
}
