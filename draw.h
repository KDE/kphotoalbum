#ifndef DRAW_H
#define DRAW_H
class QMouseEvent;
class QPainter;
class QWidget;
#include <qpoint.h>
#include <qvaluelist.h>

typedef QValueList<QPoint> PointList;
typedef QValueList<QPoint>::Iterator PointListIterator;

class Draw
{
public:
    Draw( QWidget* widget ) : _widget( widget ) {};
    void startDraw( QMouseEvent* );
    virtual void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints() = 0;
    QPoint w2g( const QPoint& ); // widget2generic
    QPoint g2w( const QPoint& );


protected:
    QPoint _startPos;
    QPoint _lastPos;
    QWidget* _widget;
};

#endif /* DRAW_H */

