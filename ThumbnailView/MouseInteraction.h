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

