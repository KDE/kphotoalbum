#include "rectdraw.h"
#include <qpainter.h>
RectDraw::RectDraw(QWidget* widget ) :Draw( widget)
{

}

void RectDraw::draw( QPainter& painter, QMouseEvent* event )
{
    Draw::draw( painter, event );
    painter.drawRect( g2w(_startPos).x(), g2w(_startPos).y(), g2w(_lastPos).x()-g2w(_startPos).x(), g2w(_lastPos).y()-g2w(_startPos).y() );
}

PointList RectDraw::anchorPoints()
{
    PointList res;
    res << g2w(_startPos) << g2w(_lastPos) << g2w(QPoint( _startPos.x(), _lastPos.y() ))
        << g2w(QPoint( _lastPos.x(), _startPos.y() ));
    return res;
}

Draw* RectDraw::clone()
{
    RectDraw* res = new RectDraw();
    *res = *this;
    return res;

}

QDomElement RectDraw::save( QDomDocument doc )
{
    QDomElement res = doc.createElement( QString::fromLatin1( "Rectangle" ) );
    saveDrawAttr( &res );
    return res;
}

RectDraw::RectDraw( QDomElement elm )
    : Draw()
{
    readDrawAttr( elm );
}


