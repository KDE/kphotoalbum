#ifndef RECTDRAW_H
#define RECTDRAW_H
#include "draw.h"

class RectDraw :public Draw
{
public:
    RectDraw( QWidget* widget = 0 );
    RectDraw( QDomElement elm );
    void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints();
    virtual Draw* clone();
    virtual QDomElement save( QDomDocument doc );
};

#endif /* RECTDRAW_H */

