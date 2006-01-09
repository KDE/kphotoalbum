#include "GridResizeInteraction.h"
#include "ThumbnailView.h"
#include <options.h>

ThumbnailView::GridResizeInteraction::GridResizeInteraction( ThumbnailView* view )
    : _view( view )
{
}

void ThumbnailView::GridResizeInteraction::mousePressEvent( QMouseEvent* event )
{
    _resizing = true;
    _mousePressPos = event->pos();
    _view->setContentsPos( 0, 0 );
    _origSize = _view->cellWidth();
}


void ThumbnailView::GridResizeInteraction::mouseMoveEvent( QMouseEvent* event )
{
    QPoint dist = event->pos() - _mousePressPos;
    int size = QMAX( 32, _origSize + (dist.x() + dist.y())/10 );
    _view->setCellWidth( size );
    _view->setCellHeight( size );
    _view->updateGridSize();
}


void ThumbnailView::GridResizeInteraction::mouseReleaseEvent( QMouseEvent* )
{
    Options::instance()->setThumbSize( _view->cellWidth() - ThumbnailView::SPACE );
    if ( !_view->_currentItem.isNull() ) {
        Cell cell = _view->positionForFileName( _view->_currentItem );
        _view->ensureCellVisible( cell.row(), cell.col() );
    }
    _resizing = false;
    _view->repaintScreen();
}

bool ThumbnailView::GridResizeInteraction::isResizingGrid()
{
    return _resizing;
}


