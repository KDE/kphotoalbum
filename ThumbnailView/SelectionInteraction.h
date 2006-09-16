#ifndef SELECTIONINTERACTION_H
#define SELECTIONINTERACTION_H

#include "MouseInteraction.h"
#include <qobject.h>
#include "Utilities/Set.h"
#include "Cell.h"

namespace ThumbnailView
{
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
    bool toggleSelectionOfFile( const QMouseEvent* ) const;
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

    Set<QString> _originalSelectionBeforeDragStart;
    ThumbnailWidget* _view;
    QTimer* _dragTimer;
    bool _dragInProgress;
};

}

#endif /* SELECTIONINTERACTION_H */

