#ifndef DRAW_H
#define DRAW_H
class QMouseEvent;
class QPainter;
#include <qpoint.h>
#include <qvaluelist.h>

typedef QValueList<QPoint> PointList;
typedef QValueList<QPoint>::Iterator PointListIterator;

class Draw
{
public:
    Draw() {};
    void startDraw( QMouseEvent* );
    virtual void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints() = 0;

protected:
    QPoint _startPos;
    QPoint _lastPos;
};

#endif /* DRAW_H */

