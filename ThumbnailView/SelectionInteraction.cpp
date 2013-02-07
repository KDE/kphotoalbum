/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include "SelectionInteraction.h"
#include "ThumbnailModel.h"
#include "ThumbnailFactory.h"
#include "CellGeometry.h"

#include <qtimer.h>
#include <QMouseEvent>
#include <qcursor.h>
#include <qapplication.h>
#include <kurl.h>

#include "MainWindow/Window.h"
#include "ThumbnailWidget.h"
#include <DB/FileNameList.h>

ThumbnailView::SelectionInteraction::SelectionInteraction( ThumbnailFactory* factory )
    : ThumbnailComponent( factory ),
      _dragInProgress( false ), _dragSelectionInProgress( false )
{
}


bool ThumbnailView::SelectionInteraction::mousePressEvent( QMouseEvent* event )
{
    _mousePressPos = event->pos();
    const DB::FileName fileName = widget()->mediaIdUnderCursor();
    _isMouseDragOperation = widget()->isSelected(fileName) && !event->modifiers();
    return _isMouseDragOperation;
}


bool ThumbnailView::SelectionInteraction::mouseMoveEvent( QMouseEvent* event )
{
    if ( _isMouseDragOperation ) {
        if ( (_mousePressPos - event->pos()).manhattanLength() > QApplication::startDragDistance() )
            startDrag();
        return true;
    }
    return false;
}


void ThumbnailView::SelectionInteraction::startDrag()
{
    _dragInProgress = true;
    KUrl::List urls;
    Q_FOREACH(const DB::FileName& fileName, widget()->selection( NoExpandCollapsedStacks )) {
        urls.append( fileName.absolute() );
    }
    QDrag* drag = new QDrag( MainWindow::Window::theMainWindow() );
    QMimeData* data = new QMimeData;
    urls.populateMimeData(data);
    drag->setMimeData( data );

    drag->exec(Qt::ActionMask);

    widget()->_mouseHandler = &(widget()->_mouseTrackingHandler);
    _dragInProgress = false;
}

bool ThumbnailView::SelectionInteraction::isDragging() const
{
    return _dragInProgress;
}

#include "SelectionInteraction.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
