#include "viewhandler.h"
#include <qevent.h>
#include <qpainter.h>
#include "displayarea.h"
#include <qapplication.h>
#include <qcursor.h>
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
        _last = e->pos();
        qApp->setOverrideCursor( SizeAllCursor  );
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
        _display->pan(  _last - e->pos() );
        _last = e->pos();
        return true;
    }
    else
        return false;
}

bool ViewHandler::mouseReleaseEvent( QMouseEvent* e )
{
    if ( _scale && (e->pos()-_start).manhattanLength() > 1 ) {
        _display->zoom( _start, e->pos() );
        return true;
    }
    else if ( _pan ) {
        qApp->restoreOverrideCursor();
        return true;
    }
    else
        return false;
}
