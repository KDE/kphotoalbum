#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H
#include <qlabel.h>
#include <qpixmap.h>
#include <qptrlist.h>
class Draw;

class DisplayArea :public QLabel {
Q_OBJECT
public:
    DisplayArea( QWidget* parent, const char* name = 0 );

public slots:
    void slotLine();
    void slotRectangle();
    void slotCircle();
    void setPixmap( const QPixmap& pixmap );

protected:
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    Draw* createTool();
    void drawAll();

private:
    enum Tool {Line, Rectangle, Circle, None};
    Tool _tool;
    Draw* _activeTool;
    QValueList<Draw*> _drawings;
    QPixmap _origPixmap;
    QPixmap _curPixmap;
};


#endif /* DISPLAYAREA_H */

