#include "displayarea.h"
#include "draw.h"
#include "linedraw.h"
#include "rectdraw.h"
#include "circledraw.h"
#include <qpainter.h>
#include <qpicture.h>

DisplayArea::DisplayArea( QWidget* parent, const char* name )
    :QLabel( parent, name ), _tool( None ), _activeTool( 0 ), _showAnchors( false )
{
    setAlignment( AlignCenter );
    setBackgroundMode( NoBackground );
}

void DisplayArea::slotLine()
{
    _tool = Line;
    if ( _showAnchors ) {
        _showAnchors = false;
        drawAll();
    }
}

void DisplayArea::slotRectangle()
{
    _tool = Rectangle;
    if ( _showAnchors ) {
        _showAnchors = false;
        drawAll();
    }
}

void DisplayArea::slotCircle()
{
    _tool = Circle;
    if ( _showAnchors ) {
        _showAnchors = false;
        drawAll();
    }
}

void DisplayArea::mousePressEvent( QMouseEvent* event )
{
    if ( _tool == None )
        QLabel::mousePressEvent( event );
    else {
        _activeTool = createTool();
        _activeTool->startDraw( event );
    }
}

void DisplayArea::mouseMoveEvent( QMouseEvent* event )
{
    if ( _activeTool ) {
        QPixmap pix = _curPixmap;
        QPainter painter( &pix );
        _activeTool->draw( painter, event );
        QLabel::setPixmap( pix );
    }
    else
        QLabel::mouseMoveEvent( event );
}

void DisplayArea::mouseReleaseEvent( QMouseEvent* event )
{
    if ( _activeTool ) {
        QPixmap pix = _curPixmap;
        QPainter painter( &pix );
        _activeTool->draw( painter, event );
        _curPixmap = pix;
        QLabel::setPixmap( pix );
        _drawings.append( _activeTool );
    }
    else
        QLabel::mouseReleaseEvent( event );
}

Draw* DisplayArea::createTool()
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

void DisplayArea::setPixmap( const QPixmap& pixmap )
{
    _origPixmap = pixmap;
    drawAll();
    QLabel::setPixmap( _curPixmap );
}

void DisplayArea::drawAll()
{
    _curPixmap = _origPixmap;
    QPainter painter( &_curPixmap );
    for( QValueList<Draw*>::Iterator it = _drawings.begin(); it != _drawings.end(); ++it ) {
        (*it)->draw( painter, 0 );
        if ( _showAnchors ) {
            PointList list = (*it)->anchorPoints();
            for( PointListIterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
                QPoint point = *it2;
                painter.save();
                painter.setBrush( QBrush( Qt::blue ) );
                painter.drawRect( point.x()-4, point.y()-4, 8, 8 );
                painter.restore();
            }
        }
    }
    QLabel::setPixmap( _curPixmap );
}

void DisplayArea::slotSelect()
{
    _showAnchors = true;
    drawAll();
}

