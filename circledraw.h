#ifndef CIRCLEDRAW_H
#define CIRCLEDRAW_H
#include "draw.h"

class CircleDraw :public Draw
{
public:
    CircleDraw();
    void draw( QMouseEvent*, QPainter& );
};

#endif /* CIRCLEDRAW_H */

