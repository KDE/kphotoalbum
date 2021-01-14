/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SELECTIONINTERACTION_H
#define SELECTIONINTERACTION_H

#include "MouseInteraction.h"
#include "ThumbnailComponent.h"
#include "enums.h"

#include <kpabase/FileName.h>

#include <QObject>

class QMouseEvent;
namespace ThumbnailView
{
class ThumbnailFactory;

class SelectionInteraction : public QObject, public MouseInteraction, private ThumbnailComponent
{
    Q_OBJECT

public:
    explicit SelectionInteraction(ThumbnailFactory *factory);
    bool mousePressEvent(QMouseEvent *) override;
    bool mouseMoveEvent(QMouseEvent *) override;
    bool isDragging() const;

protected:
    void startDrag();

private:
    /**
     * This variable contains the position the mouse was pressed down.
     * The point is in contents coordinates.
     */
    QPoint m_mousePressPos;

    /**
     * Did the mouse interaction start with the mouse on top of an icon.
     */
    bool m_isMouseDragOperation;

    // PENDING(blackie) this instance variable is unused!
    DB::FileNameSet m_originalSelectionBeforeDragStart;
    bool m_dragInProgress;
};
}

#endif /* SELECTIONINTERACTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
