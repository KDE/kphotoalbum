#include "displayarea.h"
#include "draw.h"
#include "linedraw.h"
#include "rectdraw.h"
#include "circledraw.h"
#include <qpainter.h>

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
        QPixmap pix = _pixmap;
        QPainter painter( &pix );
        painter.setPen( 3 );
        _activeTool->draw( event, painter );
        QLabel::setPixmap( pix );
    }
    else
        QLabel::mouseMoveEvent( event );
}

void DisplayArea::mouseReleaseEvent( QMouseEvent* event )
{
    if ( _activeTool ) {
        //_activeTool->draw( event );
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
    _pixmap = pixmap;
    QLabel::setPixmap( pixmap );
}

QPixmap DisplayArea::pix()
{
    return _pixmap;
}

