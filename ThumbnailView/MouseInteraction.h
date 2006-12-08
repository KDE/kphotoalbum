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
#ifndef MOUSEINTERACTION_H
#define MOUSEINTERACTION_H
#include <qevent.h>

namespace ThumbnailView {

/**
 * Mouse Event Handling for the ThumbnailView class is handled by subclasses of this class.
 *
 * Tree event handlers exists:
 * \ref GridResizeInteraction - Resizing the grid
 * \ref SelectionInteraction - handling selection
 * \ref MouseTrackingInteraction - Mouse tracking emit current file under point, when mouse is not pressed down.
 */
class MouseInteraction {
public:
    virtual void mousePressEvent( QMouseEvent* ) {};
    virtual void mouseMoveEvent( QMouseEvent* ) {};
    virtual void mouseReleaseEvent( QMouseEvent* ) {};
    virtual bool isResizingGrid() { return false; }
};

}

#endif /* MOUSEINTERACTION_H */

