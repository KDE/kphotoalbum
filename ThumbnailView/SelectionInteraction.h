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
#ifndef SELECTIONINTERACTION_H
#define SELECTIONINTERACTION_H

#include "ThumbnailComponent.h"
#include "enums.h"
#include "MouseInteraction.h"
#include <qobject.h>

namespace DB { class Id; }

class QMouseEvent;
namespace ThumbnailView
{
class ThumbnailFactory;

class SelectionInteraction : public QObject, public MouseInteraction, private ThumbnailComponent {
    Q_OBJECT

public:
    SelectionInteraction( ThumbnailFactory* factory );
    OVERRIDE bool mousePressEvent( QMouseEvent* );
    OVERRIDE bool mouseMoveEvent( QMouseEvent* );
    bool isDragging() const;

protected:
    void startDrag();

private:
    typedef QSet<DB::Id> IdSet;
    /**
     * This variable contains the position the mouse was pressed down.
     * The point is in contents coordinates.
     */
    QPoint _mousePressPos;

    /**
     * Did the mouse interaction start with the mouse on top of an icon.
     */
    bool _isMouseDragOperation;

    IdSet _originalSelectionBeforeDragStart;
    bool _dragInProgress;
    bool _dragSelectionInProgress;
    ThumbnailFactory* _JUMP;
};

}

#endif /* SELECTIONINTERACTION_H */

