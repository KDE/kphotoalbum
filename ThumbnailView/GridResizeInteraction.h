/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
