#ifndef VIEWHANDLER_H
#define VIEWHANDLER_H
#include "displayareahandler.h"
#include <qpoint.h>

class ViewHandler :public DisplayAreaHandler {

public:
    ViewHandler( DisplayArea* display );
    virtual bool mousePressEvent ( QMouseEvent* e );
    virtual bool mouseReleaseEvent ( QMouseEvent* e );
    virtual bool mouseMoveEvent ( QMouseEvent* e );
private:
    bool _scale, _pan;
    QPoint _start;
};


#endif /* VIEWHANDLER_H */

