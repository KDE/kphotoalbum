#ifndef CIRCLEDRAW_H
#define CIRCLEDRAW_H
#include "draw.h"

class CircleDraw :public Draw
{
public:
    CircleDraw( QWidget* widget );
    void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints();
};

#endif /* CIRCLEDRAW_H */

