#ifndef LINEDRAW_H
#define LINEDRAW_H
#include "draw.h"

class LineDraw :public Draw
{
public:
    LineDraw();
    void draw( QPainter&, QMouseEvent* );
};

#endif /* LINEDRAW_H */

