#include "draw.h"
#include <qevent.h>
void Draw::startDraw( QMouseEvent* event )
{
    _startPos = event->pos();
    _lastPos = QPoint();
}

void Draw::draw( QMouseEvent* event, QPainter& )
{
    _lastPos = event->pos();
}
