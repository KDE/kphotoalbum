#include "viewhandler.h"
#include <qevent.h>
#include <qpainter.h>
#include "displayarea.h"
ViewHandler::ViewHandler( DisplayArea* display )
    :DisplayAreaHandler( display )
{

}

bool ViewHandler::mousePressEvent( QMouseEvent*e  )
{
    _pan = false;
    _scale = false;

    if ( ! (e->button() & Qt::LeftButton ) )
        return false;

    if ( e->state() & Qt::ControlButton ) {
        // panning
        _pan = true;
    }
    else {
        // scaling
        _scale = true;
        _start = e->pos();
    }
    return true;
}

bool ViewHandler::mouseMoveEvent( QMouseEvent* e )
{
    if ( _scale ) {
        QPainter* p = _display->painter();
        p->setPen( QPen(Qt::black, 3, Qt::DashDotLine) );
        p->drawRect( QRect(_start, e->pos()) );
        delete p;
        return true;
    }
    else if ( _pan ) {
        return true;
    }
    else
        return false;
}

bool ViewHandler::mouseReleaseEvent( QMouseEvent* e )
{
    if ( _scale ) {
        _display->zoom( _start, e->pos() );
        return true;
    }
    else if ( _pan ) {
        return true;
    }
    else
        return false;
}
