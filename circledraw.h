#ifndef CIRCLEDRAW_H
#define CIRCLEDRAW_H
#include "draw.h"

class CircleDraw :public Draw
{
public:
    CircleDraw( QWidget* widget = 0 );
    CircleDraw( QDomElement elm );
    void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints();
    virtual Draw* clone();
    virtual QDomElement save( QDomDocument doc );
};

#endif /* CIRCLEDRAW_H */

