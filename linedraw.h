#ifndef LINEDRAW_H
#define LINEDRAW_H
#include "draw.h"

class LineDraw :public Draw
{
public:
    LineDraw( QWidget* widget = 0 );
    LineDraw( QDomElement elm );
    void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints();
    virtual Draw* clone();
    virtual QDomElement save( QDomDocument doc );
};

#endif /* LINEDRAW_H */

