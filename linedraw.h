#ifndef LINEDRAW_H
#define LINEDRAW_H
#include "draw.h"

class LineDraw :public Draw
{
public:
    LineDraw( QWidget* widget );
    void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints();
};

#endif /* LINEDRAW_H */

