#include "linedraw.h"
#include <qpainter.h>
#include <qevent.h>
#include <math.h>
#include <iostream.h>
LineDraw::LineDraw()
{

}

void LineDraw::draw( QPainter& painter, QMouseEvent* event )
{
    Draw::draw( painter, event );

    painter.save();
    painter.setPen( QPen( Qt::black, 3 ) );
    painter.drawLine( _startPos, _lastPos );

    double x0 = _startPos.x();
    double y0 = _startPos.y();
    double x1 = _lastPos.x();
    double y1 = _lastPos.y();
    double a = (y1-y0)/(x1-x0);
    double angle =  atan(a) * 360.0/(2*3.14);
    if ( x1 < x0 )
        angle = 180+angle;
    // angle is now the angle of the line.

    angle = angle + 180 - 15;
    painter.translate( x1, y1 );
    painter.rotate( angle );
    painter.drawLine( QPoint(0,0), QPoint( 30,0 ) );

    painter.rotate( 30 );
    painter.drawLine( QPoint(0,0), QPoint( 30,0 ) );

    painter.restore();
}
