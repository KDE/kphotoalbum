#ifndef LINEDRAW_H
#define LINEDRAW_H
#include "draw.h"

class LineDraw :public Draw
{
public:
    LineDraw();
    void draw( QMouseEvent*, QPainter& );
};

#endif /* LINEDRAW_H */

