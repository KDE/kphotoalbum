#include "DragInteraction.h"
#include <qtimer.h>
#include "ThumbnailView.h"
#include <qcursor.h>

ThumbnailView::DragInteraction::DragInteraction( ThumbnailView* view )
    :_view( view )
{
    _dragTimer = new QTimer( this );
    connect( _dragTimer, SIGNAL( timeout() ), this, SLOT( handleDragSelection() ) );
}


void ThumbnailView::DragInteraction::mousePressEvent( QMouseEvent* event )
{
    _mousePressPos = _view->viewportToContents( event->pos() );
    if ( !( event->state() & ControlButton ) && !( event->state() & ShiftButton ) ) {
        // Unselect every thing
        Set<QString> oldSelection = _view->_selectedFiles;
        _view->_selectedFiles.clear();
        _view->repaintScreen();
    }

    QString file = _view->fileNameAtViewportPos( event->pos() );
    if ( !file.isNull() ) {
        if ( event->state() & ShiftButton ) {
            _view->selectAllCellsBetween( _view->positionForFileName( _view->_currentItem ), _view->cellAtViewportPos( event->pos() ) );
        }
        else {
            _view->_selectedFiles.insert( file );
            _view->repaintCell( file );
            _originalSelectionBeforeDragStart = _view->_selectedFiles;
        }
        _view->_currentItem = _view->fileNameAtViewportPos( event->pos() );
    }
    _view->possibleEmitSelectionChanged();

}


void ThumbnailView::DragInteraction::mouseMoveEvent( QMouseEvent* event )
{
    if ( event->state() & LeftButton ) {
        handleDragSelection();
        if ( event->pos().y() < 0 || event->pos().y() > _view->height() )
            _dragTimer->start( 100 );
        else
            _dragTimer->stop();
    }
    else {
        // normal mouse tracking should show file under cursor.
    }
    _view->possibleEmitSelectionChanged();
}


void ThumbnailView::DragInteraction::mouseReleaseEvent( QMouseEvent* )
{
    _dragTimer->stop();
}


void ThumbnailView::DragInteraction::handleDragSelection()
{
    int col1 = _view->columnAt( _mousePressPos.x() );
    int row1 = _view->rowAt( _mousePressPos.y() );

    QPoint viewportPos = _view->viewport()->mapFromGlobal( QCursor::pos() );
    QPoint pos = _view->viewportToContents( viewportPos );
    int col2 = _view->columnAt( pos.x() );
    int row2 = _view->rowAt( pos.y() );
    _view->_currentItem = _view->fileNameInCell( row2, col2 );

    if ( viewportPos.y() < 0 )
        _view->scrollBy( 0, viewportPos.y()/2 );
    else if ( viewportPos.y() > _view->height() )
        _view->scrollBy( 0, (viewportPos.y() - _view->height())/3 );

    Set<QString> oldSelection = _view->_selectedFiles;
    _view->_selectedFiles = _originalSelectionBeforeDragStart;
    _view->selectAllCellsBetween( QPoint( col1,row1 ), QPoint( col2,row2 ), false );

    for( Set<QString>::Iterator it = oldSelection.begin(); it != oldSelection.end(); ++it ) {
        if ( !_view->_selectedFiles.contains( *it ) )
            _view->repaintCell( *it );
    }

    for( Set<QString>::Iterator it = _view->_selectedFiles.begin(); it != _view->_selectedFiles.end(); ++it ) {
        if ( !oldSelection.contains( *it ) )
            _view->repaintCell( *it );
    }

}

