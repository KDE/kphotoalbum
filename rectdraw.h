#ifndef RECTDRAW_H
#define RECTDRAW_H
#include "draw.h"

class RectDraw :public Draw
{
public:
    RectDraw();
    void draw( QMouseEvent*, QPainter& );
};

#endif /* RECTDRAW_H */

