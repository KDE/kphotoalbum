#ifndef DISPLAYAREAHANDLER_H
#define DISPLAYAREAHANDLER_H
class QMouseEvent;
#include <qobject.h>
#include "displayarea.h"

class DisplayAreaHandler :public QObject
{
public:
    DisplayAreaHandler( DisplayArea* display ) : QObject( display, "display handler" ), _display( display ) {}
    virtual bool mousePressEvent ( QMouseEvent* e, const QPoint& /*unTranslatedPos*/, double scaleFactor ) = 0;
    virtual bool mouseReleaseEvent ( QMouseEvent* e, const QPoint& /*unTranslatedPos*/, double scaleFactor ) = 0;
    virtual bool mouseMoveEvent ( QMouseEvent* e, const QPoint& /*unTranslatedPos*/, double scaleFactor ) = 0;

protected:
    DisplayArea* _display;
};

#endif /* DISPLAYAREAHANDLER_H */

