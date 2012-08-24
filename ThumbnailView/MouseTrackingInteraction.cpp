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
#include "MouseTrackingInteraction.h"
#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"
#include <QMouseEvent>
#include "VideoThumbnailCycler.h"
#include <DB/FileName.h>

ThumbnailView::MouseTrackingInteraction::MouseTrackingInteraction( ThumbnailFactory* factory )
    : ThumbnailComponent( factory ),
      _cursorWasAtStackIcon(false)
{
}

bool ThumbnailView::MouseTrackingInteraction::mouseMoveEvent( QMouseEvent* event )
{
    updateStackingIndication( event );
    handleCursorOverNewIcon();

    if ((event->modifiers() & Qt::ControlModifier) != 0 && widget()->isItemUnderCursorSelected())
        VideoThumbnailCycler::instance()->stopCycle();
    else
        VideoThumbnailCycler::instance()->setActive(widget()->mediaIdUnderCursor());
    return false;
}

void ThumbnailView::MouseTrackingInteraction::updateStackingIndication( QMouseEvent* event )
{
    bool interestingArea = widget()->isMouseOverStackIndicator( event->pos() );
    if ( interestingArea && ! _cursorWasAtStackIcon ) {
        widget()->setCursor( Qt::PointingHandCursor );
        _cursorWasAtStackIcon = true;
    } else if ( ! interestingArea && _cursorWasAtStackIcon ) {
        widget()->unsetCursor();
        _cursorWasAtStackIcon = false;
    }

}

void ThumbnailView::MouseTrackingInteraction::handleCursorOverNewIcon()
{
    static DB::FileName lastFileNameUderCursor;
    const DB::FileName fileName = widget()->mediaIdUnderCursor();
    if ( fileName != lastFileNameUderCursor ) {
        emit fileIdUnderCursorChanged(fileName);
        model()->updateCell(lastFileNameUderCursor);
        model()->updateCell(fileName);
        lastFileNameUderCursor = fileName;
    }
}
