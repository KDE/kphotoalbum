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

    double dx = _lastPos.x() - _startPos.x();
    double dy = _lastPos.y() - _startPos.y();

    if ( dx != 0 || dy != 0 ) {
        if( dy < 0 ) dx = -dx;
        double angle = acos(dx/sqrt( dx*dx+dy*dy ))*180./M_PI;
        if( dy < 0 ) angle += 180.;

        // angle is now the angle of the line.

        angle = angle + 180 - 15;
        painter.translate( _lastPos.x(), _lastPos.y() );
        painter.rotate( angle );
        painter.drawLine( QPoint(0,0), QPoint( 30,0 ) );

        painter.rotate( 30 );
        painter.drawLine( QPoint(0,0), QPoint( 30,0 ) );
    }

    painter.restore();
}

PointList LineDraw::anchorPoints()
{
    PointList res;
    res << _startPos << _lastPos << _startPos + (_lastPos - _startPos) / 2;
    return res;
}
