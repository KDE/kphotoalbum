#ifndef DRAGINTERACTION_H
#define DRAGINTERACTION_H

#include "MouseInteraction.h"
#include <qobject.h>
#include "set.h"


namespace ThumbnailView
{
class ThumbnailView;

class DragInteraction : public QObject, public MouseInteraction {
    Q_OBJECT

public:
    DragInteraction( ThumbnailView* );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void mouseReleaseEvent( QMouseEvent* );

protected slots:
    void handleDragSelection();

private:
    /**
     * This variable contains the position the mouse was pressed down.
     * The point is in contents coordinates.
     */
    QPoint _mousePressPos;

    Set<QString> _originalSelectionBeforeDragStart ;

    ThumbnailView* _view;

    QTimer* _dragTimer;
};

}

#endif /* DRAGINTERACTION_H */

