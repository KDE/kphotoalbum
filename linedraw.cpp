#include "linedraw.h"
#include <qpainter.h>
#include <qevent.h>
LineDraw::LineDraw()
{

}

void LineDraw::draw( QMouseEvent* event, QPainter& painter )
{
    painter.drawLine( _startPos, event->pos() );
    Draw::draw( event, painter );
}
