#include "circledraw.h"
#include <qpainter.h>
CircleDraw::CircleDraw( QWidget* widget ) :Draw( widget )
{
}

void CircleDraw::draw( QPainter& painter, QMouseEvent* event )
{
    Draw::draw( painter, event );
    painter.drawEllipse( g2w(_startPos).x(), g2w(_startPos).y(), g2w(_lastPos).x()-g2w(_startPos).x(), g2w(_lastPos).y()-g2w(_startPos).y() );
}

PointList CircleDraw::anchorPoints()
{
    PointList res;
    QPoint center = g2w(_startPos + (_lastPos-_startPos)/2);
    res << QPoint( center.x(), g2w(_startPos).y() )
        << QPoint( center.x(), g2w(_lastPos).y() )
        << QPoint( g2w(_startPos).x(), center.y() )
        << QPoint( g2w(_lastPos).x(), center.y() );
    return res;
}

Draw* CircleDraw::clone()
{
    CircleDraw* res = new CircleDraw( _widget );
    *res = *this;
    return res;

}
