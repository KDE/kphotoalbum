#include "drawhandler.h"
#include "options.h"
#include "draw.h"
#include "displayarea.h"
#include <qpainter.h>
#include "linedraw.h"
#include "rectdraw.h"
#include "circledraw.h"
DrawHandler::DrawHandler( DisplayArea* display )
    :DisplayAreaHandler( display ), _tool( None ), _activeTool( 0 )
{

}

bool DrawHandler::mousePressEvent( QMouseEvent* event, const QPoint& /*unTranslatedPos*/, double /*scaleFactor*/ )
{
    if ( _tool == None )
        return false;

    else if ( _tool == Select )  {
        _activeTool = findShape( event->pos() );
        emit redraw();
        if ( !_activeTool )
            return false;
    }
    else {
        _activeTool = createTool();
        _activeTool->startDraw( event );
        emit active();
    }
    return true;
}

bool DrawHandler::mouseMoveEvent( QMouseEvent* event, const QPoint& /*unTranslatedPos*/, double /*scaleFactor*/ )
{
   if ( _activeTool && _tool != Select && _tool != None) {
        QPainter* painter = _display->painter();
        setupPainter( painter );
        _activeTool->draw( painter, event );
        delete painter;
        return true;
    }
    else
        return false;
}

bool DrawHandler::mouseReleaseEvent( QMouseEvent*, const QPoint& /*unTranslatedPos*/, double /*scaleFactor*/ )
{
    if ( _tool == Select || _tool == None ) {
        return false;
    }
    else if ( _activeTool ) {
        _drawings.append( _activeTool );
        return true;
    }
    else
        return false;
}

void DrawHandler::slotLine()
{
    _tool = Line;
    emit redraw();
}

void DrawHandler::slotRectangle()
{
    _tool = Rectangle;
    emit redraw();
}

void DrawHandler::slotCircle()
{
    _tool = Circle;
    emit redraw();
}

void DrawHandler::slotSelect()
{
    _tool = Select;
    _activeTool = 0;
    emit redraw();
}

DrawList DrawHandler::drawList() const
{
    return _drawings;
}

void DrawHandler::setDrawList( const DrawList& list )
{
    _drawings = list;
    emit redraw();
}

Draw* DrawHandler::createTool()
{
    switch ( _tool ) {
    case Line: return new LineDraw();
    case Rectangle: return new RectDraw();
    case Circle: return new CircleDraw();
    default:
    {
        Q_ASSERT( false );
        return 0;
    }
    }
}

void DrawHandler::drawAll( QPainter& painter )
{
    if ( Options::instance()->showDrawings() || _tool != None ) {
        for( QValueList<Draw*>::Iterator it = _drawings.begin(); it != _drawings.end(); ++it ) {
            painter.save();
            setupPainter( &painter );
            (*it)->draw( &painter, 0 );
            painter.restore();
            if ( _tool == Select ) {
                PointList list = (*it)->anchorPoints();
                for( PointListIterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
                    painter.save();
                    if ( *it == _activeTool )
                        painter.setBrush( Qt::red );
                    else
                        painter.setBrush( Qt::blue );

                    // This part is tricky. The painter is transformed, but nevertheless the
                    // transformation I wsant a 8x8 rect on screen.
                    QPoint point = *it2;
                    point = painter.xForm( point );
                    QRect rect( point.x()-4, point.y()-4, 8, 8 );
                    rect = painter.xFormDev( rect );
                    painter.drawRect( rect );
                    painter.restore();
                }
            }
        }
    }
}

Draw* DrawHandler::findShape( const QPoint& pos)
{
    QPainter* painter = _display->painter();
    for( QValueList<Draw*>::Iterator it = _drawings.begin(); it != _drawings.end(); ++it ) {
        PointList list = (*it)->anchorPoints();
        for( PointListIterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QPoint point = *it2;
            point = painter->xForm( point );
            QRect rect( point.x()-4, point.y()-4, 8, 8 );
            rect = painter->xFormDev( rect );
            if ( rect.contains( pos ) ) {
                delete painter;
                return *it;
            }
        }
    }
    delete painter;
    return 0;
}

void DrawHandler::cut()
{
    if ( _activeTool )  {
        _drawings.remove( _activeTool );
        delete _activeTool;
        _activeTool = 0;
        emit redraw();
    }
}

void DrawHandler::stopDrawing()
{
    _activeTool = 0;
    _tool = None;
}

void DrawHandler::setupPainter( QPainter* painter )
{
    painter->setPen( QPen( Qt::black, 3 ) );
}

bool DrawHandler::hasDrawings() const
{
    return _drawings.size() != 0;
}


#include "drawhandler.moc"
