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
    Q_ASSERT( _widget );
    return QPoint( (int) (point.x() * 10000.0 / _widget->width()),  (int) (point.y() * 10000.0 / _widget->height()) );
}

QPoint Draw::g2w( const QPoint& point )
{
    Q_ASSERT( _widget );
    return QPoint( (int) (point.x() / 10000.0 * _widget->width()), (int) (point.y() / 10000.0 * _widget->height()) );
}

void Draw::saveDrawAttr( QDomElement* elm )
{
    elm->setAttribute( QString::fromLatin1("_startPos.x"), _startPos.x() );
    elm->setAttribute( QString::fromLatin1("_startPos.y"), _startPos.y() );
    elm->setAttribute( QString::fromLatin1("_lastPos.x"), _lastPos.x() );
    elm->setAttribute( QString::fromLatin1("_lastPos.y"), _lastPos.y() );
}

void Draw::readDrawAttr( QDomElement elm )
{
    _startPos = QPoint( elm.attribute( QString::fromLatin1("_startPos.x"), QString::fromLatin1("0") ).toInt(),
                        elm.attribute( QString::fromLatin1("_startPos.y"), QString::fromLatin1("0") ).toInt() );
    _lastPos  = QPoint( elm.attribute( QString::fromLatin1("_lastPos.x"), QString::fromLatin1("0") ).toInt(),
                        elm.attribute( QString::fromLatin1("_lastPos.y"), QString::fromLatin1("0") ).toInt() );
}

void Draw::setWidget( QWidget* widget )
{
    _widget= widget;
}
