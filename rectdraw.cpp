#include "rectdraw.h"
#include <qpainter.h>
RectDraw::RectDraw( QWidget* widget ) :Draw( widget )
{

}

void RectDraw::draw( QPainter& painter, QMouseEvent* event )
{
    Draw::draw( painter, event );
    painter.drawRect( _startPos.x(), _startPos.y(), _lastPos.x()-_startPos.x(), _lastPos.y()-_startPos.y() );
}

PointList RectDraw::anchorPoints()
{
    PointList res;
    res << _startPos << _lastPos << QPoint( _startPos.x(), _lastPos.y() )
        << QPoint( _lastPos.x(), _startPos.y() );
    return res;
}
