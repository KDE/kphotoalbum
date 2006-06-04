#ifndef GRIDRESIZEINTERACTION_H
#define GRIDRESIZEINTERACTION_H
#include "MouseInteraction.h"

namespace ThumbnailView
{
class ThumbnailWidget;

class GridResizeInteraction : public MouseInteraction {
public:
    GridResizeInteraction( ThumbnailWidget* );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void mouseReleaseEvent( QMouseEvent* );
    virtual bool isResizingGrid();

private:
    /**
     * The position the mouse was pressed down, in view port coordinates
     */
    QPoint _mousePressPos;

    /**
     * This variable contains the size of a cell prior to the beginning of
     * resizing the grid.
     */
    QSize _origSize;

    ThumbnailWidget* _view;

    bool _resizing;
};

}


#endif /* GRIDRESIZEINTERACTION_H */

