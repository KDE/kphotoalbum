#include "displayarea.h"
#include "draw.h"
#include "linedraw.h"
#include "rectdraw.h"
#include "circledraw.h"
#include <qpainter.h>
#include <qpicture.h>

DisplayArea::DisplayArea( QWidget* parent, const char* name )
    :QLabel( parent, name ), _tool( None ), _activeTool( 0 )
{
    setAlignment( AlignCenter );
    setBackgroundMode( NoBackground );
}

void DisplayArea::slotLine()
{
    _tool = Line;
}

void DisplayArea::slotRectangle()
{
    _tool = Rectangle;
}

void DisplayArea::slotCircle()
{
    _tool = Circle;
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
    QPainter p( &_origPixmap );
    for( QValueList<Draw*>::Iterator it = _drawings.begin(); it != _drawings.end(); ++it ) {
        (*it)->draw( p, 0 );
    }
}

