#ifndef RECTDRAW_H
#define RECTDRAW_H
#include "draw.h"

class RectDraw :public Draw
{
public:
    RectDraw( QWidget* widet );
    void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints();
};

#endif /* RECTDRAW_H */

