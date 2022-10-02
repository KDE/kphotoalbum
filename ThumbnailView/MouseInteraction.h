// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MOUSEINTERACTION_H
#define MOUSEINTERACTION_H

#include <QMouseEvent>
#include <qevent.h>

namespace ThumbnailView
{

/**
 * Mouse Event Handling for the ThumbnailView class is handled by subclasses of this class.
 *
 * Tree event handlers exists:
 * \ref GridResizeInteraction - Resizing the grid
 * \ref SelectionInteraction - handling selection
 * \ref MouseTrackingInteraction - Mouse tracking Q_EMIT current file under point, when mouse is not pressed down.
 */
class MouseInteraction
{
public:
    virtual ~MouseInteraction() { }
    virtual bool mousePressEvent(QMouseEvent *) { return false; }
    virtual bool mouseMoveEvent(QMouseEvent *) { return false; }
    virtual bool mouseReleaseEvent(QMouseEvent *) { return false; }
    virtual bool isResizingGrid() { return false; }
};

}

#endif /* MOUSEINTERACTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
