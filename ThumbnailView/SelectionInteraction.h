/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef SELECTIONINTERACTION_H
#define SELECTIONINTERACTION_H

#include "MouseInteraction.h"
#include <qobject.h>
#include "Utilities/Set.h"
#include "Cell.h"

namespace ThumbnailView
{
using Utilities::StringSet;

class ThumbnailWidget;

class SelectionInteraction : public QObject, public MouseInteraction {
    Q_OBJECT

public:
    SelectionInteraction( ThumbnailWidget* );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void mouseReleaseEvent( QMouseEvent* );
    bool isDragging() const;

protected:
    bool isMouseOverIcon( const QPoint& viewportPos ) const;
    void startDrag();
    bool atLeftSide( const QPoint& contentCoordinates );
    bool atRightSide( const QPoint& contentCoordinates );
    Cell prevCell( const Cell& cell );
    Cell nextCell( const Cell& cell );
    QRect iconRect( const QPoint& pos, CoordinateSystem ) const;
    bool deselectSelection( const QMouseEvent* ) const;
    void clearSelection();

protected slots:
    void handleDragSelection();
    void calculateSelection( Cell* pos1, Cell* pos2 );

private:
    /**
     * This variable contains the position the mouse was pressed down.
     * The point is in contents coordinates.
     */
    QPoint _mousePressPos;

    /**
     * Did the mouse interaction start with the mouse on top of an icon.
     */
    bool _mousePressWasOnIcon;

    StringSet _originalSelectionBeforeDragStart;
    ThumbnailWidget* _view;
    QTimer* _dragTimer;
    bool _dragInProgress;
    bool _dragSelectionInProgress;
};

}

#endif /* SELECTIONINTERACTION_H */

