#include "circledraw.h"
#include <qpainter.h>
CircleDraw::CircleDraw( QWidget* widget ) :Draw( widget )
{
}

void CircleDraw::draw( QPainter& painter, QMouseEvent* event )
{
    Draw::draw( painter, event );
    painter.drawEllipse( _startPos.x(), _startPos.y(), _lastPos.x()-_startPos.x(), _lastPos.y()-_startPos.y() );
}

PointList CircleDraw::anchorPoints()
{
    PointList res;
    QPoint center = _startPos + (_lastPos-_startPos)/2;
    res << QPoint( center.x(), _startPos.y() )
        << QPoint( center.x(), _lastPos.y() )
        << QPoint( _startPos.x(), center.y() )
        << QPoint( _lastPos.x(), center.y() );
    return res;
}
