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
#ifndef GRIDRESIZEINTERACTION_H
#define GRIDRESIZEINTERACTION_H
#include "MouseInteraction.h"
#include "ThumbnailComponent.h"

#include <QMouseEvent>

namespace ThumbnailView
{
class ThumbnailWidget;

class GridResizeInteraction : public MouseInteraction, private ThumbnailComponent
{
public:
    explicit GridResizeInteraction(ThumbnailFactory *factory);
    bool mousePressEvent(QMouseEvent *) override;
    bool mouseMoveEvent(QMouseEvent *) override;
    bool mouseReleaseEvent(QMouseEvent *) override;
    bool isResizingGrid() override;
    void enterGridResizingMode();
    void leaveGridResizingMode();

private:
    void setCellSize(int size);

    /**
     * The position the mouse was pressed down, in view port coordinates
     */
    QPoint m_mousePressPos;

    /**
     * This variable contains the size of a cell prior to the beginning of
     * resizing the grid.
     */
    int m_origWidth;

    bool m_resizing;
    int m_currentRow;
};
}

#endif /* GRIDRESIZEINTERACTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
