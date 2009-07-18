/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "KeyboardEventHandler.h"
#include "Cell.h"
#include "enums.h"
#include "DB/CategoryCollection.h"
#include "ThumbnailWidget.h"
#include "MainWindow/DirtyIndicator.h"
#include "DB/ImageDB.h"
#include "ThumbnailModel.h"

ThumbnailView::KeyboardEventHandler::KeyboardEventHandler( ThumbnailFactory* factory )
    : ThumbnailComponent( factory ),
     _cellOnFirstShiftMovementKey(Cell::invalidCell())
{

}

void ThumbnailView::KeyboardEventHandler::keyPressEvent( QKeyEvent* event )
{
    if ( event->modifiers() == Qt::NoModifier && ( event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z ) ) {
        QString token = event->text().toUpper().left(1);
        bool mustRemoveToken = false;
        bool hadHit          = false;

        IdSet selection = model()->selectionSet();
        for( IdSet::const_iterator it = selection.begin(); it != selection.end(); ++it ) {
            DB::ImageInfoPtr info = (*it).fetchInfo();
            if ( ! hadHit ) {
                mustRemoveToken = info->hasCategoryInfo( QString::fromLatin1("Tokens"), token );
                hadHit = true;
            }

            if ( mustRemoveToken )
                info->removeCategoryInfo( QString::fromLatin1("Tokens"), token );
            else
                info->addCategoryInfo( QString::fromLatin1("Tokens"), token );

            widget()->updateCell( *it );
        }

        if ( hadHit )
            widget()->updateCellSize();

        DB::ImageDB::instance()->categoryCollection()->categoryForName( QString::fromLatin1("Tokens") )->addItem( token );
        MainWindow::DirtyIndicator::markDirty();
    }

    if ( isMovementKey( event->key() ) )
        keyboardMoveEvent( event );

    if ( event->key() == Qt::Key_Return )
        emit showSelection();

    if ( event->key() == Qt::Key_Space )
        model()->toggleSelection( model()->currentItem() );
}

/**
   Handle key release event.
   \return true if the event should propogate
*/
bool ThumbnailView::KeyboardEventHandler::keyReleaseEvent( QKeyEvent* event )
{
    if ( widget()->_wheelResizing && event->key() == Qt::Key_Control ) {
        widget()->_wheelResizing = false;
        widget()->repaintScreen();
        return false; // Don't propogate the event - I'm not sure why.
    }
    else {
        if ( event->key() == Qt::Key_Shift )
            _cellOnFirstShiftMovementKey = Cell::invalidCell();
        return true; // Do propogate the event
    }
}

void ThumbnailView::KeyboardEventHandler::keyboardMoveEvent( QKeyEvent* event )
{
    if ( !( event->modifiers()& Qt::ShiftModifier ) && !( event->modifiers() &  Qt::ControlModifier ) ) {
        model()->clearSelection();
    }

    // Decide the next keyboard focus cell
    Cell currentPos(0,0);
    if ( !model()->currentItem().isNull() )
        currentPos = model()->positionForMediaId( model()->currentItem() );

    // Update current position if it is outside view and we do not have any modifiers
    // that is if we just scroll arround.
    //
    // Use case is following: There is a selected item which is not
    // visible because user has scrolled by other means than the
    // keyboard (scrollbar or mouse wheel). In that case if the user
    // presses keyboard movement key, the selection is forgotten and
    // instead a currently visible cell is selected. So no scrolling
    // of the view will be done.
    if ( !( event->modifiers()& Qt::ShiftModifier ) && !( event->modifiers() &  Qt::ControlModifier ) ) {
        if ( currentPos.row() < widget()->firstVisibleRow( PartlyVisible ) )
            currentPos = Cell( widget()->firstVisibleRow( FullyVisible ), currentPos.col() );
        else if ( currentPos.row() > widget()->lastVisibleRow( PartlyVisible ) )
            currentPos = Cell( widget()->lastVisibleRow( FullyVisible ), currentPos.col() );
    }

    Cell newPos;
    switch (event->key() ) {
    case Qt::Key_Left:
        newPos = currentPos;
        newPos.col()--;

        if ( newPos.col() < 0 )
            newPos = Cell( newPos.row()-1, widget()->numCols()-1 );
        break;

    case Qt::Key_Right:
        newPos = currentPos;
        newPos.col()++;
        if ( newPos.col() == widget()->numCols() )
            newPos = Cell( newPos.row()+1, 0 );
        break;

    case Qt::Key_Down:
        newPos = Cell( currentPos.row()+1, currentPos.col() );
        break;

    case Qt::Key_Up:
        newPos = Cell( currentPos.row()-1, currentPos.col() );
        break;

    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
    {
        int rows = (event->key() == Qt::Key_PageDown) ? 1 : -1;
        if ( event->modifiers() & (Qt::AltModifier | Qt::MetaModifier) )
            rows *= widget()->numRows() / 20;
        else
            rows *= widget()->numRowsPerPage();

        newPos = Cell( currentPos.row() + rows, currentPos.col() );
        break;
    }
    case Qt::Key_Home:
        newPos = Cell( 0, 0 );
        break;

    case Qt::Key_End:
        newPos = widget()->lastCell();
        break;
    }

    // Check for overruns
    if ( newPos > widget()->lastCell() )
        newPos = widget()->lastCell();
    if ( newPos < Cell(0,0) )
        newPos = Cell(0,0);

    if ( event->modifiers() & Qt::ShiftModifier ) {
        if ( _cellOnFirstShiftMovementKey == Cell::invalidCell() ) {
            _cellOnFirstShiftMovementKey = currentPos;
            _selectionOnFirstShiftMovementKey = model()->selectionSet();
        }

        model()->setSelection( _selectionOnFirstShiftMovementKey );
        model()->selectRange( _cellOnFirstShiftMovementKey, newPos );
    }

    if ( ! (event->modifiers() & Qt::ControlModifier ) ) {
        model()->select( newPos );
        widget()->updateCell( currentPos.row(), currentPos.col() );
    }
    widget()->scrollToCell( newPos );
}

bool ThumbnailView::KeyboardEventHandler::isMovementKey( int key )
{
    return ( key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right ||
             key == Qt::Key_Home || key == Qt::Key_End || key == Qt::Key_PageUp || key == Qt::Key_PageDown );
}

