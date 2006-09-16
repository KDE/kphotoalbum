#include "SelectionInteraction.h"
#include <qtimer.h>
#include "ThumbnailWidget.h"
#include <qcursor.h>
#include <qapplication.h>
#include <kurl.h>
#include <kurldrag.h>

ThumbnailView::SelectionInteraction::SelectionInteraction( ThumbnailWidget* view )
    :_view( view ), _dragInProgress( false )
{
    _dragTimer = new QTimer( this );
    connect( _dragTimer, SIGNAL( timeout() ), this, SLOT( handleDragSelection() ) );
}


void ThumbnailView::SelectionInteraction::mousePressEvent( QMouseEvent* event )
{
    _mousePressWasOnIcon = isMouseOverIcon( event->pos() );
    _mousePressPos = _view->viewportToContents( event->pos() );
    QString file = _view->fileNameAtCoordinate( event->pos(), ViewportCoordinates );
    if ( deselectSelection( event ) )
        clearSelection();

    if ( !file.isNull() ) {
        if ( event->state() & ShiftButton )
            _view->selectAllCellsBetween( _view->positionForFileName( _view->_currentItem ),
                                          _view->cellAtCoordinate( event->pos(), ViewportCoordinates ) );

        _originalSelectionBeforeDragStart = _view->_selectedFiles;
        _view->_currentItem = _view->fileNameAtCoordinate( event->pos(), ViewportCoordinates );
    }
    _view->possibleEmitSelectionChanged();

}


void ThumbnailView::SelectionInteraction::mouseMoveEvent( QMouseEvent* event )
{
    if ( !(event->state() & LeftButton ) )
        return;

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


void ThumbnailView::SelectionInteraction::mouseReleaseEvent( QMouseEvent* event )
{
    QString file = _view->fileNameAtCoordinate( event->pos(), ViewportCoordinates );
    if ( toggleSelectionOfFile( event ) ) {
        if ( _view->_selectedFiles.contains( file ) && (event->button() & LeftButton) )
            _view->_selectedFiles.remove( file);
        else
            _view->_selectedFiles.insert( file );
        _view->updateCell( file );
    }

    _dragTimer->stop();
}


void ThumbnailView::SelectionInteraction::handleDragSelection()
{
    Cell pos1;
    Cell pos2;
    calculateSelection( &pos1, &pos2 );

    _view->_currentItem = _view->fileNameInCell( pos2 );

    // Auto scroll
    QPoint viewportPos = _view->viewport()->mapFromGlobal( QCursor::pos() );
    if ( viewportPos.y() < 0 )
        _view->scrollBy( 0, viewportPos.y()/2 );
    else if ( viewportPos.y() > _view->height() )
        _view->scrollBy( 0, (viewportPos.y() - _view->height())/3 );

    Set<QString> oldSelection = _view->_selectedFiles;
    _view->_selectedFiles = _originalSelectionBeforeDragStart;
    _view->selectAllCellsBetween( pos1, pos2, false );

    for( Set<QString>::Iterator it = oldSelection.begin(); it != oldSelection.end(); ++it ) {
        if ( !_view->_selectedFiles.contains( *it ) )
            _view->updateCell( *it );
    }

    for( Set<QString>::Iterator it = _view->_selectedFiles.begin(); it != _view->_selectedFiles.end(); ++it ) {
        if ( !oldSelection.contains( *it ) )
            _view->updateCell( *it );
    }

}

/**
 * Returns whether the point viewportPos is on top of the pixmap
 */
bool ThumbnailView::SelectionInteraction::isMouseOverIcon( const QPoint& viewportPos ) const
{
    QRect rect = iconRect( viewportPos, ViewportCoordinates );
    return rect.contains( _view->viewportToContents(viewportPos) );
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

void ThumbnailView::SelectionInteraction::calculateSelection( Cell* pos1, Cell* pos2 )
{
    *pos1 = _view->cellAtCoordinate( _mousePressPos, ContentsCoordinates );
    QPoint viewportPos = _view->viewport()->mapFromGlobal( QCursor::pos() );
    *pos2 = _view->cellAtCoordinate( viewportPos, ViewportCoordinates );

    if ( *pos1 < *pos2 ) {
        if ( atRightSide( _mousePressPos ) )
            *pos1 = nextCell( *pos1 );
        if ( atLeftSide( _view->viewportToContents( viewportPos ) ) )
            *pos2 = prevCell( *pos2 );
    }
    else if ( *pos1 > *pos2 ) {
        if ( atLeftSide( _mousePressPos ) ){
            *pos1 = prevCell( *pos1 );
        }
        if ( atRightSide( _view->viewportToContents( viewportPos ) ) ) {
            *pos2 = nextCell( *pos2 );
        }
    }

    // Selecting to the right of the thumbnailview result in a position at cols(), though we only have 0..cols()-1
    if( pos1->col() == _view->numCols() )
        pos1->col()--;
    if( pos2->col() == _view->numCols() )
        pos2->col()--;
}

bool ThumbnailView::SelectionInteraction::atLeftSide( const QPoint& contentCoordinates )
{
    QRect rect = iconRect( contentCoordinates, ContentsCoordinates );
    return contentCoordinates.x() < rect.left();
}

bool ThumbnailView::SelectionInteraction::atRightSide( const QPoint& contentCoordinates )
{
    QRect rect = iconRect( contentCoordinates, ContentsCoordinates );
    return contentCoordinates.x() > rect.right();
}

ThumbnailView::Cell ThumbnailView::SelectionInteraction::prevCell( const Cell& cell )
{
    Cell res( cell.row(), cell.col() -1 );
    if ( res.col() == -1 )
        res = Cell( cell.row()-1, _view->numCols()-1 );
    if ( res < Cell(0,0) )
        return Cell(0,0);
    else
        return res;
}

ThumbnailView::Cell ThumbnailView::SelectionInteraction::nextCell( const Cell& cell )
{
    Cell res( cell.row(), cell.col()+1 );
    if ( res.col() == _view->numCols() )
        res = Cell( cell.row()+1, 0 );
    if ( res > _view->lastCell() )
        return _view->lastCell();
    else
        return res;
}

QRect ThumbnailView::SelectionInteraction::iconRect( const QPoint& coordinate, CoordinateSystem system ) const
{
    Cell pos = _view->cellAtCoordinate( coordinate, system );
    QRect cellRect = const_cast<ThumbnailWidget*>(_view)->cellGeometry(pos.row(), pos.col() );
    QRect iconRect = _view->iconGeometry( pos.row(), pos.col() );

    // map iconRect from local coordinates within the cell to the coordinates requires
    iconRect.moveBy( cellRect.x(), cellRect.y() );

    return iconRect;
}

bool ThumbnailView::SelectionInteraction::deselectSelection( const QMouseEvent* event ) const
{
    // If control or shift is pressed down then do not deselect.
    if  ( event->state() & (ControlButton | ShiftButton) )
        return false;

    // right mouse button on a selected image should not clear, left mouse button should clear at release.
    if ( _view->_selectedFiles.contains( _view->fileNameAtCoordinate( event->pos(), ViewportCoordinates ) ) )
        return false;

    // otherwise deselect
    return true;
}

void ThumbnailView::SelectionInteraction::clearSelection()
{
    // Unselect every thing
    Set<QString> oldSelection = _view->_selectedFiles;
    _view->_selectedFiles.clear();
    _originalSelectionBeforeDragStart.clear();
    for( Set<QString>::Iterator it = oldSelection.begin(); it != oldSelection.end(); ++it ) {
        _view->updateCell( *it );
    }
}

bool ThumbnailView::SelectionInteraction::toggleSelectionOfFile( const QMouseEvent* event ) const
{
    // If I didn't press on an icon, then this doesn't apply
    if ( !_mousePressWasOnIcon )
        return false;

    // if it was the right mouse button, then we should of course not toggle selection
    if ( (event->button() & RightButton) != 0 )
        return false;

    // when Shift is down, we should not toggle selection as other code already have selected the item.
    if  ( event->state() & ShiftButton )
        return false;

    return true;

}

#include "SelectionInteraction.moc"
