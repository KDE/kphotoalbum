#include "SelectionInteraction.h"
#include <qtimer.h>
#include "ThumbnailView.h"
#include <qcursor.h>
#include <qapplication.h>
#include <kurl.h>
#include <kurldrag.h>

ThumbnailView::SelectionInteraction::SelectionInteraction( ThumbnailView* view )
    :_view( view )
{
    _dragTimer = new QTimer( this );
    connect( _dragTimer, SIGNAL( timeout() ), this, SLOT( handleDragSelection() ) );
}


void ThumbnailView::SelectionInteraction::mousePressEvent( QMouseEvent* event )
{
    _mousePressWasOnIcon = isMouseOverIcon( event->pos() );
    _mousePressPos = _view->viewportToContents( event->pos() );

    QString fileNameAtPos = _view->fileNameAtCoordinate( event->pos(), ViewportCoordinates );
    bool wasIconUnderMouseSelected = !fileNameAtPos.isNull() && _view->_selectedFiles.contains( fileNameAtPos );
    if ( !( event->state() & ControlButton ) && !( event->state() & ShiftButton ) && !wasIconUnderMouseSelected) {
        // Unselect every thing
        Set<QString> oldSelection = _view->_selectedFiles;
        _view->_selectedFiles.clear();
        _originalSelectionBeforeDragStart.clear();
        _view->repaintScreen();
    }

    QString file = _view->fileNameAtCoordinate( event->pos(), ViewportCoordinates );
    if ( !file.isNull() ) {
        if ( event->state() & ShiftButton ) {
            _view->selectAllCellsBetween( _view->positionForFileName( _view->_currentItem ),
                                          _view->cellAtCoordinate( event->pos(), ViewportCoordinates ) );
        }
        else {
            _view->_selectedFiles.insert( file );
            _view->repaintCell( file );
            _originalSelectionBeforeDragStart = _view->_selectedFiles;
        }
        _view->_currentItem = _view->fileNameAtCoordinate( event->pos(), ViewportCoordinates );
    }
    _view->possibleEmitSelectionChanged();

}


void ThumbnailView::SelectionInteraction::mouseMoveEvent( QMouseEvent* event )
{
    if ( _mousePressWasOnIcon &&
         (_view->viewportToContents(event->pos()) - _mousePressPos ).manhattanLength() > QApplication::startDragDistance() )
        startDrag();

    else {
        handleDragSelection();
        if ( event->pos().y() < 0 || event->pos().y() > _view->height() )
            _dragTimer->start( 100 );
        else
            _dragTimer->stop();
    }
    _view->possibleEmitSelectionChanged();
}


void ThumbnailView::SelectionInteraction::mouseReleaseEvent( QMouseEvent* )
{
    _dragTimer->stop();
}


void ThumbnailView::SelectionInteraction::handleDragSelection()
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
    _view->selectAllCellsBetween( Cell( row1, col1 ), Cell( row2, col2 ), false );

    for( Set<QString>::Iterator it = oldSelection.begin(); it != oldSelection.end(); ++it ) {
        if ( !_view->_selectedFiles.contains( *it ) )
            _view->repaintCell( *it );
    }

    for( Set<QString>::Iterator it = _view->_selectedFiles.begin(); it != _view->_selectedFiles.end(); ++it ) {
        if ( !oldSelection.contains( *it ) )
            _view->repaintCell( *it );
    }

}

/**
 * Returns whether the point viewportPos is on top of the pixmap
 */
bool ThumbnailView::SelectionInteraction::isMouseOverIcon( const QPoint& viewportPos ) const
{
    Cell pos = _view->cellAtCoordinate( viewportPos, ViewportCoordinates );
    QRect cellRect = const_cast<ThumbnailView*>(_view)->cellGeometry(pos.row(), pos.col() );
    QRect iconRect = _view->iconGeometry( pos.row(), pos.col() );

    // map iconRect from local coordinates within the cell to contents coordinates
    iconRect.moveBy( cellRect.x(), cellRect.y() );

    return iconRect.contains( _view->viewportToContents(viewportPos) );
}

void ThumbnailView::SelectionInteraction::startDrag()
{
    _dragInProgress = true;
    KURL::List l;
    QStringList selected = _view->selection();
    for( QStringList::Iterator fileIt = selected.begin(); fileIt != selected.end(); ++fileIt ) {
        l.append( KURL(*fileIt) );
    }
    KURLDrag* drag = new KURLDrag( l, _view, "drag" );
    drag->dragCopy();

    _view->_mouseHandler = &(_view->_mouseTrackingHandler);
    _dragInProgress = false;
}

bool ThumbnailView::SelectionInteraction::isDragging() const
{
    return _dragInProgress;
}
