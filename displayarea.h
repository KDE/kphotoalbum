#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H
#include <qlabel.h>
#include <qpixmap.h>
class Draw;

class DisplayArea :public QLabel {
Q_OBJECT
public:
    DisplayArea( QWidget* parent, const char* name = 0 );
    QPixmap pix();

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

private:
    enum Tool {Line, Rectangle, Circle, None};
    Tool _tool;
    Draw* _activeTool;
    QPixmap _pixmap;
};


#endif /* DISPLAYAREA_H */

