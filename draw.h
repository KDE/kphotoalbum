#ifndef DRAW_H
#define DRAW_H
class QMouseEvent;
class QPainter;
class QWidget;
#include <qpoint.h>
#include <qvaluelist.h>
#include <qdom.h>

typedef QValueList<QPoint> PointList;
typedef QValueList<QPoint>::Iterator PointListIterator;

class Draw
{
public:
    Draw( QWidget* widget = 0 ) :_widget( widget ) {}
    void startDraw( QMouseEvent* );
    virtual void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints() = 0;
    QPoint w2g( const QPoint& ); // widget2generic
    QPoint g2w( const QPoint& );
    virtual Draw* clone() = 0;
    virtual QDomElement save( QDomDocument doc ) = 0;
    void setWidget( QWidget* widget );

protected:
    QPoint _startPos;
    QPoint _lastPos;
    QWidget* _widget;
    void saveDrawAttr( QDomElement* elm );
    void readDrawAttr( QDomElement elm );
};

#endif /* DRAW_H */

