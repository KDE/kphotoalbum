#include "draw.h"
#include <qevent.h>
void Draw::startDraw( QMouseEvent* event )
{
    _startPos = event->pos();
    _lastPos = QPoint();
}

void Draw::draw( QPainter&, QMouseEvent* event )
{
    if ( event )
        _lastPos = event->pos();
}
