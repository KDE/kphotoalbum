#include "draw.h"
#include <qevent.h>
#include <qwidget.h>
void Draw::startDraw( QMouseEvent* event )
{
    _startPos = w2g( event->pos() );
    _lastPos = QPoint();
}

void Draw::draw( QPainter&, QMouseEvent* event )
{
    if ( event )
        _lastPos = w2g( event->pos() );
}

QPoint Draw::w2g( const QPoint& point )
{
    return QPoint( point.x() * 10000 / _widget->width(),  point.y() * 10000 / _widget->height() );
}

QPoint Draw::g2w( const QPoint& point )
{
    return QPoint( point.x() / 10000 * _widget->width(),  point.y() / 10000 * _widget->height() );
}
