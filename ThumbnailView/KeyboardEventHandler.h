#ifndef KEYBOARDEVENTHANDLER_H
#define KEYBOARDEVENTHANDLER_H
#include "Cell.h"
#include "ThumbnailComponent.h"
#include "enums.h"
class QKeyEvent;
class ThumbnailFactory;

namespace ThumbnailView
{
class KeyboardEventHandler :public ThumbnailComponent
{
public:
    KeyboardEventHandler( ThumbnailFactory* factory );
    void keyPressEvent( QKeyEvent* event );
    bool keyReleaseEvent( QKeyEvent* );
    void keyboardMoveEvent( QKeyEvent* );

private:
    bool isMovementKey( int key );

private:
    // For Shift + movement key selection handling
    Cell _cellOnFirstShiftMovementKey;
    IdSet _selectionOnFirstShiftMovementKey;

};
}

#endif /* KEYBOARDEVENTHANDLER_H */

