#ifndef VIEWHANDLER_H
#define VIEWHANDLER_H
#include "displayareahandler.h"

class ViewHandler :public DisplayAreaHandler {

public:
    ViewHandler( DisplayArea* display );
    virtual bool mousePressEvent ( QMouseEvent* e );
    virtual bool mouseReleaseEvent ( QMouseEvent* e );
    virtual bool mouseMoveEvent ( QMouseEvent* e );

};


#endif /* VIEWHANDLER_H */

